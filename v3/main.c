//------------------------------------------------------------------------------------
// Skeleton.c
//------------------------------------------------------------------------------------
//
// This is a skeleton program which doesnot do anything meaningful other than initailise
// the Massey C8051F020 Development Board to use the external oscillator (22.1184 MHz)
// and set up the various port pins for I/O operations. The Watchdog timer is disabled.
//
//
//------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------
#include <compiler_defs.h>	//-- Compiler definitions
#include <c8051f020_defs.h>	//-- SFR declarations
#include <stdio.h>			//-- Standard I/O operations
//#include <lcd.h>			//-- LCD operations
#include <xData.h>

//------------------------------------------------------------------------------------
// Global CONSTANTS
//------------------------------------------------------------------------------------
sbit LED_16 = P1 ^ 6;	//-- green LED on P1.6

char count=0;

unsigned char data i = 0;
unsigned char data command = 0;

//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
void initPorts(void);	// general system initialization

void initDAC(void);
void initTimer4(void);

void initCrossBar(void);
void initTimer1(void);
void init_UART0 (void);

//!	@alan
void build_command (unsigned char* data cmd);
void handle_command(unsigned char* data cmd);
void transmit(char* p);
//------------------------------------------------------------------------------------
// main routine
//------------------------------------------------------------------------------------
//
void main(void)
{
	EA = 0;			//-- disable global interrupts
	initPorts();	//-- initialise development board
	P5 = 0x0F;		//-- turn off the four green LEDs on Port 5[7:4]
	//lcd_init();	//-- call this function to initialise LCD
	initCrossBar();
	initTimer1();
	init_UART0();
	LED_16 = 0;
	EA = 1;			//-- enable global interrupts
	transmit("\r\n\t\t== WELCOME! PLEASE CHOOSE A MODE ==\r\n");
	
	
	while (1)
	{
		build_command(&command);
		handle_command(&command);
	}
}//////===end of main===//////


void Uart0_ISR(void) interrupt 4
{

}

void Timer4_ISR(void) interrupt 16
{
	T4CON &= 0x7f;		//to clear the flag	
	count %= 73;		
	DAC0H = cos[count++];
} 

//! process the command from PC
void handle_command(unsigned char* data cmd)
{
	switch(*cmd)
	{
		case '1'	:	
			transmit("case 1\r\n");	break;
		case '2'	:
			transmit("case 2\r\n");	break; 
		case '3'	:	
			transmit("case 3\r\n");	break;
		case '4'	:
			transmit("case 4\r\n");	break; 
		default		:
			transmit("\t\t");
			transmit(&command);
			transmit(" ?\r\n\t\tNo... What am I supposed to do with it?\r\n");
			break;
	}
}
//! transmit the char to PC.	
void transmit(char* p)
{
	while(*p != '\0')
	{
		SBUF0 = *p;
		while(TI0 == 0);
		TI0 = 0;
		++p;
	}
}


void build_command(unsigned char* data cmd)
{	
	SCON0 = 0x50; 	//UART0 mode 1: 8-bit ,variable baud rate and enable RX
	while (RI0 ==0);
	*cmd 	= SBUF0;
	RI0 	= 0;				// clear flag
	if ( command == '1')		LED_16 ^= 1; 				
}

void init_UART0 (void)
{
	// Set up UART0
	PCON |= 0x80; // #80h,disable UART0 baud rate div-by-2
	SCON0 = 0x50; //UART0 mode 1: 8-bit ,variable baud rateand enable RX
	// enable UART0 interrupt
	IE |= 0X10;
}

void initTimer1(void)
{
	CKCON |= 0x10;  // #00010000b,Timer 1 uses system clock of
	        		// 22.1184MHz
	TMOD = 0x20; 	// #20h , configureTimer1 mode 2, 8-bit auto reload
	TH1 = 0xF4;		//#F4h,Baud rate = 115200
	TL1 = TH1;
	TR1= 1;			// Start Timer 1
}

void initCrossBar(void)	//put it after initPorts() function
{
	XBR0 |= 0x04; // #00000100b ,enable UART0 I/O   UART0 TX routed to P0.0, and RX routed to P0.1.
	XBR2 |= 0x40; // #01000000b, enable Crossbar
}


void initDAC(void)
{
	// Vref setup:
	// Enable internal bias generator and internal, reference buffer.
	REF0CN = 0x03 ; // 00000011b
	DAC0CN = 0x84; 
}


void initTimer4(void)
{
	//Timer 4 Setup
	//NOTE: System clock = 22.1184 MHz external oscillator
	//----------------------------------------------------
	CKCON |= 0x40 ; // 00100000b,Timer 4 uses system clock
	RCAP4L = 0xD0 ; // Set reload time to 0xFF0F
	RCAP4H = 0xFE ; // FFFF-FF0F = SYSCLK/DAC sampling rate DAC
					// sampling time=92160 Hz
	TL4 = RCAP4L; //Initialize TL4 & TH4 before
	TH4 = RCAP4H; //counting

	EIE2 |= 04;	

	T4CON |= 0x04 ; // 00000100 ,start Timer 4 in 16 bit auto-reload mode
}

void initPorts(void)
{
	WDTCN = 0x07;		// Watchdog Timer Control Register
	WDTCN = 0xDE;		// Disable watch dog timer
	WDTCN = 0xAD;

	OSCXCN = 0x67;	  				// EXTERNAL Oscillator Control Register   
	while ((OSCXCN & 0x80) == 0);	// wait for xtal to stabilize

	OSCICN = 0x0C;					// Internal Oscillator Control Register

	//---- Configure the Crossbar Registers
	XBR0 = 0x00;
	XBR1 = 0x00;
	XBR2 = 0x40;		  	// Enable the crossbar, weak pullups enabled
	// XBR2 = 0xC0;			// To disable weak pull-ups 

	//---- Port configuration (1 = Push Pull Output, 0 = Open Drain)
	P0MDOUT = 0x00;			// Output configuration for P0 
	P1MDOUT = 0x40;			// Output configuration for P1 (Push-Pull for P1.6) 
	P2MDOUT = 0x00;			// Output configuration for P2 
	P3MDOUT = 0x00;			// Output configuration for P3 

	//---- Set up Ports 7-4 I/O Lines
	P74OUT = 0x48;	// Output configuration for P4-7
					// (P7[0:3] Push Pull) - Control Lines for LCD
					// (P6 Open-Drain)- Data Lines for LCD
					// (P5[7:4] Push Pull) - 4 LEDs
					// (P5[3:0] Open Drain) - 4 Push-Button Switches (input)
					// (P4 Open Drain) - 8 DIP Toggle Switches (input)

	//---- Write a logic 1 to those pins which are to be used for input operations
	P5 |= 0x0F;
	P4 = 0xFF;
}


