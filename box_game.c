#include "box_game.h"
#include "3595_LCD.h"

/**BOX_location[] Array*********************
* Used to track where boxes are in the     *
* playing area.  One byte per column with  *
* byte0 as the left column.  One bit per   *
* row with the LSB at the top.  This makes *
* for a 12x8 playing area                  *
*******************************************/

unsigned char BOX_location[] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};

/*****************************************
* BOX_piece[] is a 4x4 represntation of  *
* the currently selected playing piece.  *
* When a piece spawns its shape is       *
* written to this array in four nibbles, *
* BIT0,LSN; Bit0,MSN; BIT1,LSN; BIT1,MSN *
* The LSB of each nibble is the top of   *
* the display area.                      *
*****************************************/

unsigned char BOX_piece[] = { 0b00000000, 0b00000000 }

//Variables
unsigned char x_loc, y_loc;

//Functions
void BOX_store_loc(void)		//Stores current x_loc & y_loc to array
{
  BOX_location[x_loc] |= 1<<y_loc;
}

void BOX_clear_loc(void)		//Stores current x_loc & y_loc to array
{
  BOX_location[x_loc] &= ~(1<<y_loc);
}

void BOX_draw(unsigned char X, unsigned char Y)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(X*8, 0);
  LCD_Out((X*8)+7, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(Y*8, 0);
  LCD_Out((Y*8)+7, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (unsigned char i=0; i<64; i++) LCD_Out(red,0);
}
void BOX_erase(unsigned char X, unsigned char Y)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(X*8, 0);
  LCD_Out((X*8)+7, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(Y*8, 0);
  LCD_Out((Y*8)+7, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (unsigned char i=0; i<64; i++) LCD_Out(white,0);
}
void BOX_spawn(void)
{
  x_loc = 0;
  y_loc = 0;
  BOX_store_loc();
  BOX_draw(x_loc, y_loc);
}

void BOX_up(void)
{
  if (y_loc == 0) return; //Do nothing if we're at the top already
  if (BOX_location[x_loc] & 1<<(y_loc-1)) return; //Do nothing if there is a box above us
  BOX_clear_loc();
  BOX_erase(x_loc, y_loc--);
  BOX_draw(x_loc, y_loc);
  BOX_store_loc();
}

void BOX_dn(void)
{
  if (y_loc == 7) return; //Do nothing if we're at the bottom already
  if (BOX_location[x_loc] & 1<<(y_loc+1)) return; //Do nothing if there is a box below us
  BOX_clear_loc();
  BOX_erase(x_loc, y_loc++);
  BOX_draw(x_loc, y_loc);
  BOX_store_loc();
}

void BOX_lt(void)
{
  if (x_loc == 0) return; //Do nothing if we're at the left edge already
  if (BOX_location[x_loc-1] & 1<<y_loc) return; //Do nothing if there is a box beside us
  BOX_clear_loc();
  BOX_erase(x_loc--, y_loc);
  BOX_draw(x_loc, y_loc);
  BOX_store_loc();
}

void BOX_rt(void)
{
  if (x_loc == 11) return; //Do nothing if we're at the left edge already
  if (BOX_location[x_loc+1] & 1<<y_loc) return; //Do nothing if there is a box beside us
  BOX_clear_loc();
  BOX_erase(x_loc++, y_loc);
  BOX_draw(x_loc, y_loc);
  BOX_store_loc();
}