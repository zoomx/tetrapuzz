#include "box_game.h"
#include "KS0108.h"
#include <util/delay.h>

/*
 * TODO: Fix 8-bit color in 3595_LCD.c (seems to be backwards??)
 * 			http://en.wikipedia.org/wiki/8-bit_color
 *
 * TODO: Update line checking to work for new 4-byte pieces
 *
 * TODO: Clean this place up!
 */


/*
Program flow:
  -Initialize
  -Spawn piece
    -load from reference
    -calculate y_loc
    -draw piece
  -Wait for game input
*/

/**BOX_location[] Array*********************
* Used to track where boxes are in the     *
* playing area.  One byte per column with  *
* byte0 as the left column.  One bit per   *
* row with the LSB at the top.  This makes *
* for a 12x8 playing area                  *
*******************************************/

//TODO: make this scaleable
/*
    This can be scaled and still keep it a 1 dimensional array.
    We can use BOX_board_bottom and BOX_board_right to calculate
    where in the array data will be stored.

    This should be useful somehow:
      BOX_location[((x_loc%BOX_board_right)*BOX_board_rigth)+BOX_board_right)]

*/


/*********************************************
* BOX_piece[] is a 4x4 representation of      *
* the currently selected playing piece.      *
* When a piece spawns its shape is           *
* written to this array in four nibbles      *
* Byte0,LSN; Byte0,MSN; Byte1,LSN; Byte1,MSN *
* The LSB of each nibble is the top of       *
* the display area.                          *
*********************************************/

unsigned char BOX_piece[4];

static const char PROGMEM BOX_reference[7][4][4] = {
		//T
		{
				{
						0b00000010,
						0b00000011,
						0b00000010,
						0b00000000
				},

				{
						0b00000000,
						0b00000111,
						0b00000010,
						0b00000000
				},

				{
						0b00000001,
						0b00000011,
						0b00000001,
						0b00000000
				},

				{
						0b00000010,
						0b00000111,
						0b00000000,
						0b00000000
				}
		},

		// S
		{
				{
						0b00000010,
						0b00000011,
						0b00000001,
						0b00000000
				},

				{
						0b00000011,
						0b00000110,
						0b00000000,
						0b00000000
				},

				{
						0b00000010,
						0b00000011,
						0b00000001,
						0b00000000
				},

				{
						0b00000011,
						0b00000110,
						0b00000000,
						0b00000000
				}
		},

		// Z
		{
				{
						0b00000001,
						0b00000011,
						0b00000010,
						0b00000000
				},

				{
						0b00000110,
						0b00000011,
						0b00000000,
						0b00000000
				},

				{
						0b00000001,
						0b00000011,
						0b00000010,
						0b00000000
				},

				{
						0b00000110,
						0b00000011,
						0b00000000,
						0b00000000
				}
		},

		// L
		{
				{
						0b00000011,
						0b00000001,
						0b00000001,
						0b00000000
				},

				{
						0b00000000,
						0b00000001,
						0b00000111,
						0b00000000
				},

				{
						0b00000010,
						0b00000010,
						0b00000011,
						0b00000000
				},

				{
						0b00000000,
						0b00000111,
						0b00000100,
						0b00000000
				}
		},

		// J
		{
				{
						0b00000001,
						0b00000001,
						0b00000011,
						0b00000000
				},

				{
						0b00000000,
						0b00000100,
						0b00000111,
						0b00000000
				},

				{
						0b00000011,
						0b00000010,
						0b00000010,
						0b00000000
				},

				{
						0b00000000,
						0b00000111,
						0b00000001,
						0b00000000
				}
		},

		// Box
		{
				{
						0b00000011,
						0b00000011,
						0b00000000,
						0b00000000,
				},

				{
						0b00000011,
						0b00000011,
						0b00000000,
						0b00000000,
				},

				{
						0b00000011,
						0b00000011,
						0b00000000,
						0b00000000,
				},

				{
						0b00000011,
						0b00000011,
						0b00000000,
						0b00000000,
				}
		},

		// Line
		{
				{
						0b00000010,
						0b00000010,
						0b00000010,
						0b00000010
				},

				{
						0b00000000,
						0b00001111,
						0b00000000,
						0b00000000
				},

				{
						0b00000010,
						0b00000010,
						0b00000010,
						0b00000010
				},

				{
						0b00000000,
						0b00001111,
						0b00000000,
						0b00000000
				}
		}
};

//Variables
unsigned char BOX_location[array_size];
unsigned char x_loc, y_loc;     //Bottom left index of each piece
unsigned char cur_piece = 0;	//Index for BOX_reference
unsigned char rotate = 0;	//Index for piece rotation

//Messages
static const char PROGMEM message1[] = { "Tetrapuzz!" };
static const char PROGMEM message2[] = { "click to start" };
static const char PROGMEM message3[] = { "Game Over" };

//Functions

/*******************************
 * Display specific functions: *
 *   Change these to suit      *
 *   whatever display will     *
 *   be used.                  *
 *******************************/

void GLCD_box_draw(unsigned char x, unsigned char y,unsigned char color)
{
  y = 19-y;
  GLCD_GoTo(y, x/2);
  for (unsigned char i=0; i<4; i++)
  {
	  //GLCD_SetPixel((y*6)+j,(x*6)+i,color);
	  if (x%2)
	  {
		  if (BOX_loc_return_bit(x-1,y)) GLCD_WriteData(0xFF);
		  else GLCD_WriteData(0x0F);
	  }
	  else
	  {
		  if (BOX_loc_return_bit(x+1,y)) GLCD_WriteData(0xFF);
		  else GLCD_WriteData(0xF0);
	  }
  }
}

void BOX_draw(unsigned char X, unsigned char Y, unsigned char color)
{
	  unsigned char temp_data;
	  if (X%2)
	  {
		  if (BOX_loc_return_bit(X-1,Y)) temp_data = 0xFF;
		  else temp_data = 0x0F;
	  }
	  else
	  {
		  if (BOX_loc_return_bit(X+1,Y)) temp_data = 0xFF;
		  else temp_data = 0xF0;
	  }
	  Y = (19*4)-(Y*4);
	  GLCD_GoTo(Y, X/2);
	  for (unsigned char i=0; i<4; i++)
	  {
		  //GLCD_SetPixel((y*6)+j,(x*6)+i,color);
		  GLCD_WriteData(temp_data);
	  }
}

void BOX_erase(unsigned char X, unsigned char Y)
{
	  unsigned char temp_data;
	  if (X%2)
	  {
		  if (BOX_loc_return_bit(X-1,Y)) temp_data = 0xF0;
		  else temp_data = 0x00;
	  }
	  else
	  {
		  if (BOX_loc_return_bit(X+1,Y)) temp_data = 0x0F;
		  else temp_data = 0x00;
	  }
	  Y = (19*4)-(Y*4);
	  GLCD_GoTo(Y, X/2);
	  for (unsigned char i=0; i<4; i++)
	  {
		  //GLCD_SetPixel((y*6)+j,(x*6)+i,color);
		  GLCD_WriteData(temp_data);
	  }
}

void BOX_pregame(void)
{

  GLCD_Initalize();

  GLCD_ClearScreen();

  for (unsigned char i=0; i<16; i++)
  {
    BOX_draw(5,i,1);
    _delay_ms(500);
  }


  //LCD_Fill_Screen(yellow);

  //cursor_x = 18;
  //cursor_y = 9;
  //LCD_Write_String_P(message1,green,yellow);

  //cursor_x = 6;
  //cursor_y = 20;
  //LCD_Write_String_P(message2,black,yellow);
}

void BOX_start_game(void)
{
  //Populate BOX_location[] with 0
  for (unsigned char i=0; i<array_size; i++) { BOX_location[i] = 0x00; }

  BOX_rewrite_display(black, white);
  BOX_spawn();
}

void BOX_end_game(void)
{
  TCCR1B &= ~(1<<CS12 | 1<<CS11 | 1<<CS10);	//stop timer
  BOX_rewrite_display(white,black);
  //cursor_x = 24;
  //cursor_y = 29;
  //LCD_Write_String_P(message3,white,black);
  while(1) { }
}

/**********************************
 * End Display specific functions *
 **********************************/

/**********************************************
 * Functions that handle bits in BOX_location[]
 * BOX_loc_return_bit
 * BOX_loc_set_bit
 * BOX_loc_clear_bit
 ************************************************/

unsigned char BOX_loc_return_bit(unsigned char X, unsigned char Y)
{
  //Calculate array index and shift amount
  unsigned char array_index_offset = ((Y)/8)*(BOX_board_right+1);
  unsigned char shift_index = (Y)%8;		//How much to shift for our bit mask

  if (BOX_location[X+array_index_offset] & 1<<shift_index) return 1;
  else return 0;
}

void BOX_loc_set_bit(unsigned char X, unsigned char Y)
{
  //Calculate array index and shift amount
  unsigned char array_index_offset = ((Y)/8)*(BOX_board_right+1);
  unsigned char shift_index = (Y)%8;		//How much to shift for our bit mask

  BOX_location[X+array_index_offset] |= 1<<shift_index;
}

void BOX_loc_clear_bit(unsigned char X, unsigned char Y)
{
  //Calculate array index and shift amount
  unsigned char array_index_offset = ((Y)/8)*(BOX_board_right+1);
  unsigned char shift_index = (Y)%8;		//How much to shift for our bit mask

  BOX_location[X+array_index_offset] &= ~(1<<shift_index);
}

/********************************
 * Functions that handle bits in BOX_piece[]
 * BOX_piece_return_bit()
 * BOX_piece_set_bit()
 * BOX_piece_clear_bit()
 */

void BOX_store_loc(void)
{
  //Step through 4 columns
  for (unsigned char temp_col=0; temp_col<4; temp_col++)
  {
    //Only if x_loc is not out of bounds
    if ((unsigned char)(x_loc+temp_col) <= BOX_board_right)
    {
      //Step through 4 rows
      for (unsigned char temp_row=0; temp_row<4; temp_row++)
      {
		//Only if y_loc is not out of bounds
		if (y_loc-temp_row <= BOX_board_bottom)
		{
		  if (BOX_piece[temp_col] & 1<<(temp_row))	//Checks nibbles in Box_piece array
		  {
			BOX_loc_set_bit((unsigned char)(x_loc+temp_col),y_loc-temp_row);
		  }
		}
      }
    }
  }
}

void BOX_clear_loc(void)
{
  //Step through 4 columns
  for (unsigned char temp_col=0; temp_col<4; temp_col++)
  {
    //Only if x_loc is not out of bounds
    if ((unsigned char)(x_loc+temp_col) <= BOX_board_right)
    {
      //Step through 4 rows
      for (unsigned char temp_row=0; temp_row<4; temp_row++)
      {
		//Only if y_loc is not out of bounds
		if (y_loc-temp_row <= BOX_board_bottom)
		{
		  if (BOX_piece[temp_col] & 1<<(temp_row))	//Checks nibbles in Box_piece array
		  {
			BOX_loc_clear_bit((unsigned char)(x_loc+temp_col),y_loc-temp_row);
		  }
		}
      }
    }
  }
}

void BOX_load_reference(unsigned char piece, unsigned char rotation)
{
	  BOX_piece[0] = pgm_read_byte(&BOX_reference[piece][rotation][0]);
	  BOX_piece[1] = pgm_read_byte(&BOX_reference[piece][rotation][1]);
	  BOX_piece[2] = pgm_read_byte(&BOX_reference[piece][rotation][2]);
	  BOX_piece[3] = pgm_read_byte(&BOX_reference[piece][rotation][3]);
}

void BOX_rotate(unsigned char direction)
{
  //TODO: Allow for adjustments if rotation is prevented due to proximity

  BOX_clear_loc(); //Clear current location so we don't have false compares

  //Load in the candidate rotation
  unsigned char new_rotate = rotate;
  if (++new_rotate > 3) new_rotate = 0;
  BOX_load_reference(cur_piece,new_rotate);

  //Check for overlaps
  if (BOX_check(0, 0))
  {
	  //Overlap will be caused, restore piece settings and return
	  BOX_load_reference(cur_piece, rotate);
	  BOX_store_loc();
	  return;
  }
  //No overlap found, allow new rotation
  else
  {
	  //Load the current rotation back in and clear the piece from display
	  BOX_load_reference(cur_piece, rotate);
	  BOX_clear_piece();

	  //Load new rotation, display, and save its location
	  rotate = new_rotate;
	  BOX_load_reference(cur_piece, rotate);

	  BOX_store_loc();
	  BOX_write_piece();
  }
}



void BOX_write_piece(void)  //Writes piece to display
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
    //prevent invalid indices from being written
      if ((y_loc-j) >= BOX_board_top)
      {
		if (BOX_piece[i] & 1<<j)
		{
		  //TODO: change this for different colored playing pieces
		  BOX_draw(x_loc+i, y_loc-j, default_fg_color);
		}
      }
    }
  }
}

void BOX_clear_piece(void)  //Clears piece from display
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
    //prevent invalid indices from being written
      if ((y_loc-j) >= BOX_board_top)
      {
  		if (BOX_piece[i] & 1<<j)
  		{
  		  //TODO: change this for different colored playing pieces
  		  BOX_erase(x_loc+i, y_loc-j);
  		}
      }
    }
  }
}

void BOX_rewrite_display(unsigned char fgcolor, unsigned char bgcolor)	//Rewrites entire playing area
{
  for (unsigned char cols=0; cols<=BOX_board_right; cols++)
  {
	  for (unsigned char rows=0; rows<=BOX_board_bottom; rows++)
	  {
		  if(BOX_loc_return_bit(cols,rows)) BOX_draw(cols,rows,fgcolor);
		  else BOX_erase(cols,rows);
	  }
  }
}

void BOX_spawn(void)
{
  x_loc = 4;
  y_loc = 1;
  cur_piece = random_piece;
  rotate = 0;

  BOX_load_reference(cur_piece, rotate);  //load from reference



  //Check to see if we've filled the screen
  if (BOX_check(0,0))
  {
	  BOX_end_game();
  }


  BOX_store_loc(); //Store new location
  BOX_write_piece(); //draw piece

}

unsigned char BOX_check(signed char X_offset, signed char Y_offset)
{
	unsigned char temp_area[4] = { 0x00, 0x00, 0x00, 0x00 };
	unsigned char i;
	//Build compare mask in temp_area[]

		//Clear the current piece from BOX_location[] so we don't have a false overlap
			//Do this only if X_offset and Y_offset aren't both 0 (used for BOX_spawn)
		if (X_offset || Y_offset) BOX_clear_loc();
		//mask will be 4 sets of nibbles (2 bytes)
		for (i=0; i<4; i++)
		{
			//if out of bounds on the x axis
			if ((unsigned char)(x_loc+X_offset+i) > BOX_board_right) temp_area[i] = 0x0F;
			else
			{
				for (unsigned char j=0; j<4; j++)
				{
					//if we're out of bounds on the y axis
					if (((unsigned char)(y_loc+Y_offset-j) > BOX_board_bottom) ||
					   (BOX_loc_return_bit((unsigned char)(x_loc+X_offset+i),(unsigned char)(y_loc+Y_offset-j))))
					{
						temp_area[i] |= 1<<j;
					}
				}
			}
		}
		if (X_offset || Y_offset) BOX_store_loc(); //Restore the location we cleared earlier

	if ((temp_area[0] & BOX_piece[0]) | (temp_area[1] & BOX_piece[1]) | (temp_area[2] & BOX_piece[2]) | (temp_area[3] & BOX_piece[3]))
	{
		//Conflict has been found
		return 1;
	}
	else return 0;
}

void BOX_line_check(void)
{
  //TODO: Tweak this to enable scoring

  //Check every line on the playing area for complete rows and record them in an array
  unsigned char complete_lines[4];	//There will never be more than 4 complete rows
  unsigned char temp_index = 0;		//Index for complete_lines[]

  for (unsigned char board_rows=0; board_rows<=BOX_board_bottom; board_rows++)
  {
    unsigned char board_cols=0;
    while ((board_cols<=BOX_board_right) && (BOX_loc_return_bit(board_cols,board_rows)))
    {
    	//Complete row found, record in complete_lines[]
		if (board_cols == BOX_board_right) complete_lines[temp_index++] = board_rows;
		++board_cols;
    }
  }
  if (temp_index == 0) return;  //No complete lines found, return


  //If there are complete rows
    //TODO: Disable interrupts to pause game flow
    //TODO: Add an arbitrary delay, perhaps make complete lines flash?

  //Rewrite BOX_location[] without completed rows.
  --temp_index;	//This was incremented one too many times earlier, get it back to the proper index.

  //Rewrite BOX_location[] data without completed lines
  unsigned char read_from_row = BOX_board_bottom;
  unsigned char write_to_row = BOX_board_bottom;

  //When we have read from all rows, this will be set to 0 and
  //remaining bits cleared from BOX_location[]
  unsigned char rows_left_to_read = 1;

  //Use variable i to iterate through every row of the board
  unsigned char i=0;
  while (i <= BOX_board_bottom)
  {
	  //If the current row is a complete line
	  if (read_from_row == complete_lines[temp_index])
	  {
		  //Decrement indexes
		  if (read_from_row == 0)
		  {
			  rows_left_to_read = 0;

			  //Change complete_lines[0] so we don't do this again
			  complete_lines[0] = read_from_row + 1;
		  }
		  else
		  {
			  --read_from_row;
			  if (temp_index) --temp_index;
		  }
	  }

	  else
	  {
		  //Write data to all columns of current row
		  for (unsigned char col=0; col<=BOX_board_right; col++)
		  {
			  //If there are rows left to read from, do so.
			  if (rows_left_to_read)
			  {
				  if (BOX_loc_return_bit(col,read_from_row)) BOX_loc_set_bit(col, write_to_row);
				  else BOX_loc_clear_bit(col, write_to_row);
			  }
			  //There are no rows left to read from, fill with 0
			  else
			  {
				  BOX_loc_clear_bit(col, write_to_row);
			  }
		  }

		  //A row has now been read from, decrement the counter
		  --read_from_row;

		  //A row has now been written to, increment the counter, decrement the tracker
		  ++i;
		  --write_to_row;
	  }
  }

  BOX_rewrite_display(black, white);
}


void BOX_up(void)
{
  BOX_clear_loc();
  BOX_clear_piece();

  if (++cur_piece > 6) cur_piece = 0;
  x_loc = 4;
  y_loc = 0;

  BOX_spawn();
}

void BOX_dn(void)
{
  if (BOX_check(0, 1))
  {
    //Set piece here and spawn a new one
    BOX_rewrite_display(black, default_bg_color);
    BOX_line_check();
    BOX_spawn();
    return;
  }

  BOX_clear_loc();
  BOX_clear_piece();
  ++y_loc;

  BOX_store_loc();
  BOX_write_piece();
}

void BOX_lt(void)
{
  if (BOX_check(-1, 0)) return; //Do nothing if moving causes an overlap
  //if (((x_loc == BOX_board_left) && (BOX_piece[0] & 0x0F)) || (x_loc == 255)) return; //Do nothing if we're at the left edge already
  //if (BOX_location[x_loc-1] & 1<<y_loc) return; //Do nothing if there is a box beside us
  BOX_clear_loc();
  BOX_clear_piece();
  x_loc--;

  BOX_store_loc();
  BOX_write_piece();
}

void BOX_rt(void)
{
  if (BOX_check(1, 0)) return; //Do nothing if moving causes an overlap
  BOX_clear_loc();
  BOX_clear_piece();
  ++x_loc;

  BOX_store_loc();
  BOX_write_piece();
}
