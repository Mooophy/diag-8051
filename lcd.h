//-- Filename : LCD.h
//-- Header File for LCD display routines implemented in LCD.c


#include <c8051f020_defs.h>

//------------------------------------------------------------------------------------
// Global Defines
//------------------------------------------------------------------------------------

#define LCD_DAT_PORT  P6		  // LCD is in 8 bit mode
#define LCD_CTRL_PORT P7		  // 3 control pins on P7
#define RS_MASK       0x01		  // for assessing LCD_CTRL_PORT
#define RW_MASK       0x02
#define E_MASK        0x04

//------------------------------------------------------------------------------------
// Global MACROS
//------------------------------------------------------------------------------------
#define pulse_E();\
	small_delay(1);\
	LCD_CTRL_PORT = LCD_CTRL_PORT | E_MASK;\
	small_delay(1);\
	LCD_CTRL_PORT = LCD_CTRL_PORT & ~E_MASK;\

//------------------------------------------------------------------------------------
// LCD function prototypes
//------------------------------------------------------------------------------------
void lcd_init       (void);          // initialize the lcd to 8 bit mode
void lcd_busy_wait  (void);          // wait until the lcd is no longer busy
char putchar        (char c);        // replaces standard function and uses LCD
void lcd_cmd        (char cmd);      // write a command to the lcd controller
void lcd_home       (void);          // home curser
void lcd_clear      (void);          // clear display
void lcd_goto       (char addr);    // move to address addr
void lcd_move_curser(char dist);     // moves curser forward or back by dist
void lcd_curser     (bit on);        // 1 displays curser, 0 hides it
void lcd_puts       (char string[]); // send string to lcd at current curser location
void small_delay    (char d);   // 8 bit,  about 0.34us per count @22.1MHz
void large_delay    (char d);   // 16 bit, about 82us   per count @22.1MHz
//void huge_delay     (char d);   // 24 bit, about 22ms   per count @22.1MHz
//---------------------------------------------------------------------------

