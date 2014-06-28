//-- Filename : LCD.c
//-- LCD Functions Implementation


#include <compiler_defs.h>
#include <c8051f020_defs.h>			  // SFR declarations
#include <LCD.h>

//----------------------------- LCD related Functions -----------------------------
#pragma OPTIMIZE (7)
void lcd_init(void)
{
	LCD_CTRL_PORT = LCD_CTRL_PORT & ~RS_MASK;	// RS = 0
	LCD_CTRL_PORT = LCD_CTRL_PORT & ~RW_MASK;	// RW = 0
	LCD_CTRL_PORT = LCD_CTRL_PORT & ~E_MASK;	//  E = 0
	large_delay(200);				  // 16ms delay

	LCD_DAT_PORT = 0x38;			  // set 8-bit mode
	pulse_E();
	large_delay(50);				  // 4.1ms delay

	LCD_DAT_PORT = 0x38;			  // set 8-bit mode
	pulse_E();
	large_delay(2);				  	  // 1.5ms delay

	LCD_DAT_PORT = 0x38;			  // set 8-bit mode
	pulse_E();
	large_delay(2);				  	  // 1.5ms delay

	lcd_cmd(0x06);					  // curser moves right
	lcd_clear();
	lcd_cmd(0x0E);					  // display and curser on
}
#pragma OPTIMIZE (9)



//------------------------------------------------------------------------------------
// lcd_busy_wait
//------------------------------------------------------------------------------------
//
// wait for the busy bit to drop
//
void lcd_busy_wait(void)
{
	LCD_DAT_PORT = 0xFF;
	LCD_CTRL_PORT = LCD_CTRL_PORT & ~RS_MASK;	// RS = 0
	LCD_CTRL_PORT = LCD_CTRL_PORT | RW_MASK;	// RW = 1
	small_delay(1);
	LCD_CTRL_PORT = LCD_CTRL_PORT | E_MASK;	//  E = 1

	do
	{								  // wait for busy flag to drop
		small_delay(1);
	} while ((LCD_DAT_PORT & 0x80) != 0);

}


//------------------------------------------------------------------------------------
// lcd_dat (putchar)
//------------------------------------------------------------------------------------
//
// write a character to the lcd screen
//
char putchar(char dat)
{
	lcd_busy_wait();
	LCD_CTRL_PORT = LCD_CTRL_PORT | RS_MASK;	// RS = 1
	LCD_CTRL_PORT = LCD_CTRL_PORT & ~RW_MASK;	// RW = 0
	LCD_DAT_PORT = dat;
	pulse_E();
	return 1;
}


//------------------------------------------------------------------------------------
// lcd_cmd
//------------------------------------------------------------------------------------
//
// write a command to the lcd controller
//
void lcd_cmd(char cmd)
{
	lcd_busy_wait();
	LCD_CTRL_PORT = LCD_CTRL_PORT & ~RS_MASK;	// RS = 0
	LCD_CTRL_PORT = LCD_CTRL_PORT & ~RW_MASK;	// RW = 0
	LCD_DAT_PORT = cmd;
	pulse_E();
}


//------------------------------------------------------------------------------------
// lcd_goto
// Call: lcd_goto(0x00); 	//-- takes the cursor to the beginning of the top line
// Call: lcd_goto(0x40); 	//-- takes the cursor to the beginning of the bottom line
//------------------------------------------------------------------------------------
void lcd_goto(char addr)
{
	lcd_cmd(addr | 0x80);
}

//------------------------------------------------------------------------------------
// lcd_clear
//------------------------------------------------------------------------------------
void lcd_clear(void)
{
	lcd_cmd(0x01);	//-- clear LCD display
	lcd_cmd(0x80);	//-- curser go to 0x00
}

//------------------------------------------------------------------------------------
// lcd_curser
//------------------------------------------------------------------------------------
void lcd_curser(bit on)        // 1 displays curser, 0 hides it
{
	if (on)
		lcd_cmd(0x0E);	
	else
		lcd_cmd(0x0C);
}

//------------------------------------------------------------------------------------
// delay routines
//------------------------------------------------------------------------------------
void small_delay(char d)
{
	while (d--);
}


void large_delay(char d)
{
	while (d--)
		small_delay(255);
}

//!	@alan
//!	@note	:	commented it to prevent warnings.	
/*
void huge_delay(char d)
{
	while (d--)
		large_delay(255);
}
*/