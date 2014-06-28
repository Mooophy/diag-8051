//------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------
#include <compiler_defs.h>		//-- Compiler definitions
#include <c8051f020_defs.h>		//-- SFR declarations
#include <stdio.h>						//-- Standard I/O operations
#include <lcd.h>							//-- LCD operations


//------------------------------------------------------------------------------------
// Global CONSTANTS
//------------------------------------------------------------------------------------
//!	@brief	:	the green led on board
sbit LED_16 = P1 ^ 6;	

//!	@brief	:	PWM pin.
//!	@atention	:	P0 ^ 4 refering to port0 pin 5
sbit pwm_pin = P0 ^ 4;	 

//!	@brief	iterator used for DAC part
char count=0;

//! @brief	:	used for iteration
//! @attention : probbably need to delete in the final version
unsigned char data i = 0;					

//!	@brief	:	store the command from PC 	 
unsigned char data command = 0;	

//! @brief	:	store [header][DIP][BTNs][ADC0H][\0] 	 
unsigned char data input_image[INPUT_IMG_SIZE] = {0,0,0,0,0};

//!	@brief	:	used in local function
unsigned char data char_buffer = 0;
bit 					 		 bit_buffer  = 0;
unsigned char data ADC0H_Buffer= 0;

//!	@brief	:	data array used for DAC
unsigned char xdata cos[] =
{
	255,255,254,253,251,248,243,239,233,226,220,218,210,201,192,182,172,161,
	150,139,128,116,106,95,84,74,64,54,45,37,30,23,17,12,7,2,0,2,7,12,17,23,
	30,37,45,54,64,74,84,95,106,116,128,139,150,161,172,182,192,201,210,218,
	220,226,233,239,243,248,251,253,254,255,255
};
//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
//!	@attention : below from notes.
void initPorts(void);	
void initDAC(void);
void initTimer4(void);
void initCrossBar(void);
void initTimer1(void);
void init_UART0 (void);
void Init_ADC0 (void);


//!	@author	:	below by alan

//!	@brief	:	for PWM generation.
void initTimer0(void);
//!	@brief	:	acctually to on/off DAC0 output.
//!	@attention:	make sure timer4 and DAC0 has been init.
void operate_timer4(bit on);
void build_command (unsigned char* data cmd);
void handle_command(unsigned char* data cmd);
//!	@attention only for sending string
void transmit(const unsigned char* p);
//------------------------------------------------------------------------------------
// main routine
//------------------------------------------------------------------------------------
//
void main(void)
{
	EA = 0;					//-- disable global interrupts
	initPorts();		//-- initialise development board
	P5 = 0x0F;			//-- turn off the four green LEDs on Port 5[7:4]
	lcd_init();			//-- call this function to initialise LCD
	initCrossBar();
	initTimer1();
	init_UART0();
	LED_16 = 0;
	lcd_curser(0);	
	//! @attention for DAC0
	initTimer4();
	initDAC();
	//!	@attention for PWM
	initTimer0();
	//! @attention for ADC0
	Init_ADC0();
	EA = 1;			
	
	while (1)
	{	
		//!	@brief	prompt	
		lcd_clear();		
		lcd_goto(0x00);
		printf("     WELCOME");
		transmit("\r\n\t\t ==WELCOME==\r\n\t\   ==PLEASE CHOOSE A MODE==\r\n");
		
		//!	@brief	retrieve the command from pc and process it 
		build_command(&command);		
		handle_command(&command);
	}	
}

//!	@brief	:	timer0 isr used for pwm.
void Timer0_ISR (void) interrupt 1
{
	static unsigned int	data duty_cycle=0;
	static unsigned int data count=0;

	TF0 = 0;
	count %= 765;
	duty_cycle = (P4 ^ 0xff) * 3;
	pwm_pin = (count < duty_cycle)? 1 : 0;	
	++count;
}


void Uart0_ISR(void) interrupt 4{	}
void ADC0_ISR (void) interrupt 15
{
	AD0INT = 0 ; // clear ADC0 conversion complete interrupt flag
	ADC0H_Buffer = ADC0H ; // read ADC0 value, to make 12bit equivalent to 8-bit, effectively it 
	// is right shift 4-bit ie. divide by 16, this is something to think about!
}

void Timer4_ISR(void) interrupt 16
{
	T4CON &= 0x7f;		//to clear the flag	
	count %= 73;		
	DAC0H = cos[count++];
} 


//!	@brief	:	acctually to on/off DAC0 output.
//!	@attention:	make sure timer4 and DAC0 has been init.
void operate_timer4( bit on)
{
	if(on)		T4CON |= 0x04;
	else			T4CON &= 0x04 ^ 0xff ;	
}


//! @brief	process the command from PC
void handle_command(unsigned char* data cmd)
{
	switch(*cmd)
	{
		//!	@LCD
		case '1'	:
			transmit("case 1 LCD type a few keys ,'c' to clear, 'q' to return\r\n");
			lcd_clear();
			i = 0;
			while(command != 'q')
			{
				if(command == 'c')	{
					lcd_clear();
					i=0;
				}
				build_command(&command);
				i %= 56;
				lcd_goto(  (i >15)? (i+24) : i   );
				printf(&command);		
				++i;
			}
			lcd_clear();
			lcd_goto(0x00);
			printf("     WELCOME");
															break;		
		//!	@LED
		case '2'	:
			lcd_goto(0x00);
			printf("   Tesing LCD");
			transmit("case 2 click key '2' to toggle 5 LEDs on board.\r\n");	
			P5 ^= 0xf0; 
			LED_16 ^= 1;
			break; 
		
		//!	@ADC		
		case '3'	:	
			lcd_goto(0x00);
			printf("   Testing ADC");	
			transmit("case 3	ADC :  adjust the potentiometer check the LCD display. any key to quit. \r\n");
			SCON0 = 0x50;
			while(RI0 == 0) 
			{
				AD0BUSY = 1;
				lcd_goto(0x44);
				printf("ADC0H=%d     ", (unsigned int)ADC0H);
			}		
			//!	@attention : clear the flag 
			RI0 = 0;			
			break;
		
		
		//! @DIP
		case '4'	:
			lcd_goto(0x00);
			printf("   Tesing DIP");	
			transmit("case 4	DIP : Adjust the dip , check LCD . P3.7 to quit.\r\n");
			
			while( P3 >> 7 )
			{
				for(i=0; i != 8; ++i)
				{
					lcd_goto(0x43 + 8 - i);
					printf("%d",(unsigned int)(  ((P4^0xff) & (1 << i)) >> i  ));
				}				
			}
			lcd_clear();
			lcd_goto(0x00);
			printf("     WELCOME");	
			break; 
		
		//!	@buttons			
		case '5'	:
			lcd_goto(0x00);
			printf("   Tesing BTN");	
			transmit("case 5	BTN : press a button and check LCD. any key on terminal to quit \r\n");
			
			//! @attention	any thing on uart would end this loop.
			SCON0 = 0x50;
			while(RI0 == 0)
			{
				LED_16 = (P3 >> 7) ^ 1;
				if((P3 >> 7) ^ 1)	
				{
					lcd_goto(0x43);
					printf("P3.7 works");	
				}
				for(i = 0; i != 4; ++i)
				{
					LED_16 = (P5 >> i) & 1 ^ 1;
					if((P5 >> i) & 1 ^ 1)		
					{
						lcd_goto(0x43);
						printf("P5.%d works", (unsigned int)i);	
					}	
				}	
			}
			//!	@attention : clear the flag 
			RI0 = 0;
			break;
		
		//!	@DAC			
		case '6'	:
			lcd_goto(0x00);
			printf("   Testing DAC");	
			transmit("case 6	DAC :  check cosine wave on the scope. any key to quit \r\n");
			SCON0 = 0x50;
			while(RI0 == 0) operate_timer4(1);
		
			//!	@attention : clear the flag 
			RI0 = 0;
			operate_timer4(0);
			break;

		
		//!	@PWM			
		case '7'	:
			lcd_goto(0x00);
			printf("   Testing PWM");	
			transmit("case 7	PWM :  check PWM on the scope, adjust dutycylce using DIP. any key to quit \r\n");
			
			//!	@brief	:	 start timer0	to generate pwm
			TR0 = 1;		
		
			SCON0 = 0x50;
			while(RI0 == 0)
			{
				//lcd_clear();
				lcd_goto(0x43);
				printf("duty  =  %d%%  ",  (unsigned int)(((float)((P4 ^ 0xff) + 1)) * 0.390625f));
			}
			//!	@attention : clear the flag 
			RI0 = 0;
			
			//!	@brief	: 	stop 	timer0	to terminate pwm
			TR0 = 0;
			break;		
		
		//!	@exceptions		
		default		:
			transmit("\t\t");
			transmit(&command);
			transmit(" ?\r\n\t\tNo... What am I supposed to do with it?\r\n");
			break;
	}
}
//! @brief	transmit a string to PC via uart.
//!	@param	pointer to the globle where ths command is stored.
void transmit(const unsigned char* p)
{
	while(*p != '\0')
	{
		SBUF0 = *p;
		while(TI0 == 0);
		TI0 = 0;
		++p;
	}
}

//!	@biref	receive commands from PC via uart and store it.
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



void Init_ADC0 (void)
{
	REF0CN = 0x03 ; 	// Vref setup: 
	// Enable internal bias generator and internal reference buffer, and 
	//select ADC0 reference from VREF0 pin.
	// assume system clock = 22 MHz external oscillator 
	// this has to be programmed earlier through using OSCXCN register
	// Configure ADC0 to use AIN0 as single-ended input only. Other inputs are not used. 
	ADC0CF = 0xB8 ; 	// ADC0CF= 10111000b , SAR0 Conversion clock=941 kHz approx, 
	// Gain=1
	AMX0CF = 0x00 ;		// AMX0CF=00000000b, 8 single-ended inputs
	AMX0SL = 0x00 ; 	// AMX0SL= 00000000b, select AIN0 input
	ADC0CN= 0x81 ; 		// ADC0CN=10000001b , Enable ADC0, Continuous Tracking Mode, 
	//conversion initiated on AD0BUSYand left justify data
	EIE2 |= 0x02 ; 		// enable ADC end-of-conversion interrupt
}

//!	@brief :  for uart0
void initTimer1(void)
{
	CKCON |= 0x10;  	// #00010000b,Timer 1 uses system clock of
										// 22.1184MHz
	TMOD 	|= 0x20; 		// #20h , configureTimer1 mode 2, 8-bit auto reload
	TH1 	 = 0xF4;		//#F4h,Baud rate = 115200
	TL1 	 = TH1;
	TR1= 1;						// Start Timer 1
}

//!	@brief	for PWM
void initTimer0(void)
{
	CKCON  |= 0x08;		//--T0M = 1; Timer 0 uses SysClock which is 22.1MH in this case.
	TMOD 	 |= 0x02;		//	mode 2 i.e. 8-bit counter with auto-reload.
	
	//!	@brief	:	255 -34 = 221		=>		22.1M/221	=	100K
	TL0 		= 0xff;		//	register TL0 stores the starting value.
	TH0			= 34;			//	register TH0 stores the reload value.
	TR0			= 0;			//  stop the timer0	  	

	ET0 		= 1;			//  enable interrupt when timer 0 flag is set.
}

void initTimer4(void)
{
	//Timer 4 Setup
	//NOTE: System clock = 22.1184 MHz external oscillator
	//----------------------------------------------------
	CKCON |= 0x40 ; 			// 00100000b,Timer 4 uses system clock
	RCAP4L = 0xD0 ; 			// Set reload time to 0xFF0F
	RCAP4H = 0xFE ; 			// FFFF-FF0F = SYSCLK/DAC sampling rate DAC
												// sampling time=92160 Hz
	TL4 = RCAP4L; 				//Initialize TL4 & TH4 before
	TH4 = RCAP4H; 				//counting

	EIE2 |= 04;	

//	T4CON |= 0x04 ; // 00000100 ,start Timer 4 in 16 bit auto-reload mode
}

//!	@brief	general initialization
void initPorts(void)
{
	WDTCN = 0x07;					// Watchdog Timer Control Register
	WDTCN = 0xDE;					// Disable watch dog timer
	WDTCN = 0xAD;

	OSCXCN = 0x67;	  							// EXTERNAL Oscillator Control Register   
	while ((OSCXCN & 0x80) == 0);		// wait for xtal to stabilize

	OSCICN = 0x0C;								// Internal Oscillator Control Register

	//---- Configure the Crossbar Registers
	XBR0 = 0x00;
	XBR1 = 0x00;
	XBR2 = 0x40;		  	// Enable the crossbar, weak pullups enabled
	// XBR2 = 0xC0;			// To disable weak pull-ups 

	//---- Port configuration (1 = Push Pull Output, 0 = Open Drain)
	P0MDOUT = 0x10;			// Output configuration for P0 
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


