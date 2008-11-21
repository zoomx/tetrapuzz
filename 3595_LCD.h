/**LCD Pinout*****************
* 1=Reset------>3.3v+        *
* 2=CS--------->Logic        *
* 3=GND-------->GND          *
* 4=SIO-------->Logic        *
* 5=SCL-------->Logic        *
* 6=VDigital--->3.3v+        *
* 7=VBoost----->3.3v+        *
* 8=VLCD--->0.22uf Cap-->GND *
*  (This cap may not be      *
*   optimal, other schematics*
*   have suggested 1uf)      *
*****************************/

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

#define LCD_PORT PORTC
#define LCD_DDR  DDRC
#define LCD_CLK (1<<PC0)
#define LCD_SIO (1<<PC1)
#define LCD_CS  (1<<PC2)
#define LCD_RST (1<<PC3)

#define page_size	97
#define row_size	66

#define red	0b00000111
#define yellow	0b00111111
#define green	0b00111100
#define cyan	0b11111000
#define blue	0b11000000
#define magenta	0b11000111
#define white	0b11111111
#define black	0b11111111

//Variables
unsigned char cursor_x, cursor_y; //Tracks cursor position (top-left corner of cursor area)

//Prototypes
void LCD_init(void);
void LCD_Out(unsigned char Data, unsigned char isCmd);
void LCD_Flash_BW(unsigned int flash_delay_ms);
void LCD_XorScreen(void);
void LCD_StripedScreen(void);
void LCD_Hello_World(void);
void LCD_Advance_Cursor(void);
void LCD_Write_String(char * myString, unsigned char fgcolor, unsigned char bgcolor);
void LCD_Write_Char(unsigned char letter, unsigned char fgcolor, unsigned char bgcolor);
void LCD_Fill_Screen(unsigned char color);