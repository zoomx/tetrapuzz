#include "box_game.h"
#include "3595_LCD.h"

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
* BOX_piece[] is a 4x4 represntation of      *
* the currently selected playing piece.      *
* When a piece spawns its shape is           *
* written to this array in four nibbles      *
* Byte0,LSN; Byte0,MSN; Byte1,LSN; Byte1,MSN *
* The LSB of each nibble is the top of       *
* the display area.                          *
*********************************************/

unsigned char BOX_piece[] = { 0b00000000, 0b00000000 };

static const char PROGMEM BOX_reference[7][4][4] = {
		//T
		{
				{
						0b00001000,
						0b00001100,
						0b00001000,
						0b00000000
				},

				{
						0b00000000,
						0b00001110,
						0b00000100,
						0b00000000
				},

				{
						0b00000100,
						0b00001100,
						0b00000100,
						0b00000000
				},

				{
						0b00000100,
						0b00001110,
						0b00000000,
						0b00000000
				}
		},

		// S
		{
				{
						0b00001000,
						0b00001100,
						0b00000100,
						0b00000000
				},

				{
						0b00000110,
						0b00001100,
						0b00000000,
						0b00000000
				},

				{
						0b00001000,
						0b00001100,
						0b00000100,
						0b00000000
				},

				{
						0b00000110,
						0b00001100,
						0b00000000,
						0b00000000
				}
		},

		// Z
		{
				{
						0b00000100,
						0b00001100,
						0b00001000,
						0b00000000
				},

				{
						0b00001100,
						0b00000110,
						0b00000000,
						0b00000000
				},

				{
						0b00000100,
						0b00001100,
						0b00001000,
						0b00000000
				},

				{
						0b00001100,
						0b00000110,
						0b00000000,
						0b00000000
				}
		},

		// L
		{
				{
						0b00001100,
						0b00000100,
						0b00000100,
						0b00000000
				},

				{
						0b00000000,
						0b00000010,
						0b00001110,
						0b00000000
				},

				{
						0b00001000,
						0b00001000,
						0b00001100,
						0b00000000
				},

				{
						0b00000000,
						0b00001110,
						0b00001000,
						0b00000000
				}
		},

		// J
		{
				{
						0b00000100,
						0b00000100,
						0b00001100,
						0b00000000
				},

				{
						0b00000000,
						0b00001000,
						0b00001110,
						0b00000000
				},

				{
						0b00001100,
						0b00001000,
						0b00001000,
						0b00000000
				},

				{
						0b00000000,
						0b00001110,
						0b00000010,
						0b00000000
				}
		},

		// Box
		{
				{
						0b00001100,
						0b00001100,
						0b00000000,
						0b00000000,
				},

				{
						0b00001100,
						0b00001100,
						0b00000000,
						0b00000000,
				},

				{
						0b00001100,
						0b00001100,
						0b00000000,
						0b00000000,
				},

				{
						0b00001100,
						0b00001100,
						0b00000000,
						0b00000000,
				}
		},

		// Line
		{
				{
						0b00000100,
						0b00000100,
						0b00000100,
						0b00000100
				},

				{
						0b00000000,
						0b00001111,
						0b00000000,
						0b00000000
				},

				{
						0b00000100,
						0b00000100,
						0b00000100,
						0b00000100
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

/**********************************************
 * Functions that handle bits in BOX_location[]
 * BOX_loc_return_bit
 * BOX_loc_set_bit
 * BOX_loc_clear_bit
 ************************************************/

unsigned char BOX_loc_return_bit(unsigned char X, unsigned char Y)
{
  //Calculate array index and shift amount
  unsigned char array_index_offset = ((Y+1)/8)*(BOX_board_right+1);
  unsigned char shift_index = (Y+1)%8;		//How much to shift for our bit mask

  //Make adjustment so that index is 0-7 and not 1-8
  if (shift_index) shift_index -= 1;
  else shift_index = 7;

  if (BOX_location[X+array_index_offset] & 1<<shift_index) return 1;
  else return 0;
}

void BOX_loc_set_bit(unsigned char X, unsigned char Y)
{
  //Calculate array index and shift amount
  unsigned char array_index_offset = ((Y+1)/8)*(BOX_board_right+1);
  unsigned char shift_index = (Y+1)%8;		//How much to shift for our bit mask

  //Make adjustment so that index is 0-7 and not 1-8
  if (shift_index) shift_index -= 1;
  else shift_index = 7;

  BOX_location[X+array_index_offset] |= 1<<shift_index;
}

void BOX_loc_clear_bit(unsigned char X, unsigned char Y)
{
  //Calculate array index and shift amount
  unsigned char array_index_offset = ((Y+1)/8)*(BOX_board_right+1);
  unsigned char shift_index = (Y+1)%8;		//How much to shift for our bit mask

  //Make adjustment so that index is 0-7 and not 1-8
  if (shift_index) shift_index -= 1;
  else shift_index = 7;

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
	  if (BOX_piece[temp_col/2] & 1<<((4*(temp_col%2))+(3-temp_row)))	//Checks nibbles in Box_piece array
	  {
	    BOX_loc_set_bit((unsigned char)(x_loc+temp_col),y_loc-temp_row);
	  }
	}
      }
    }
  }
}

/* code rewrite */
/*
void BOX_store_loc(void)		//Stores current x_loc & y_loc to array
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    //if ((x_loc+i) > BOX_board_right) return;  //Prevent invalid x_loc
    if (((unsigned char)(x_loc+i) >= BOX_board_left) && ((unsigned char)(x_loc+i) <= BOX_board_right))
    {
      for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
      {
      //prevent invalid indicies from being written
	if (((y_loc-j) >= BOX_board_top) && ((y_loc-j) <= BOX_board_bottom))
	{
	  if (BOX_piece[i/2] & 1<<((4*(i%2))+(3-j)))
	  {
	    BOX_location[(unsigned char)(x_loc+i)] |= 1<<(y_loc-j);
	  }
	}
      }
    }
  }
}
*/
/* end rewrite */

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
	  if (BOX_piece[temp_col/2] & 1<<((4*(temp_col%2))+(3-temp_row)))	//Checks nibbles in Box_piece array
	  {
	    BOX_loc_clear_bit((unsigned char)(x_loc+temp_col),y_loc-temp_row);
	  }
	}
      }
    }
  }
}

/* code rewrite */
/*
void BOX_clear_loc(void)		//Stores current x_loc & y_loc to array
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    //if ((x_loc+i) > BOX_board_right) return;  //Prevent invalid x_loc
    if (((unsigned char)(x_loc+i) >= BOX_board_left) && ((unsigned char)(x_loc+i) <= BOX_board_right))
    {
      for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
      {
      //prevent invalid indicies from being written
	if (((y_loc-j) >= BOX_board_top) && ((y_loc-j) <= BOX_board_bottom))
	{
	  if (BOX_piece[i/2] & 1<<((4*(i%2))+(3-j)))
	  {
	    BOX_location[(unsigned char)(x_loc+i)] &= ~(1<<(y_loc-j));
	  }
	}
      }
    }
  }
}
*/
/* end rewrite */

void BOX_load_reference(unsigned char piece, unsigned char rotation)
{
  BOX_piece[0] = pgm_read_byte((char *)(BOX_reference + (piece*8) + (rotation*2)));
  BOX_piece[1] = pgm_read_byte((char *)(BOX_reference + (piece*8) + (rotation*2) + 1));
}

void BOX_rotate(unsigned char direction)
{
  //TODO: Check if we are going to hit something when we rotate
  //working area
  BOX_clear_loc(); //Clear current location so we don't have false compares

  //Load in the candidate rotation
  unsigned char new_position[2];
  unsigned char new_rotate = rotate;
  if (++new_rotate > 3) new_rotate = 0;
  new_position[0] = pgm_read_byte((char *)(BOX_reference + (cur_piece*8) + (new_rotate*2)));
  new_position[1] = pgm_read_byte((char *)(BOX_reference + (cur_piece*8) + (new_rotate*2) + 1));


  //check left
  if ((new_position[0] & 0x0F) && (x_loc > BOX_board_right)) { BOX_store_loc(); return; }
    //Find how much we need to go right to rotate
    //Check this distance and move right if possible    //Find how much we need to go down to rotate
  //check right
  if ((new_position[1] & 0xF0) && (x_loc+3 > BOX_board_right)) { BOX_store_loc(); return; }
  if ((new_position[1] & 0x0F) && (x_loc+2 > BOX_board_right)) { BOX_store_loc(); return; }
    //Find how much we need to go down to rotate
    //Check this distance and move left if possible
  //check top
    //Find how much we need to go down to rotate
    //Check this distance below us and move down if possible
  //check bottom
  if (((new_position[0] | new_position[1]) & 0x44) && (y_loc > BOX_board_bottom)) { BOX_store_loc(); return; }
    //if something is below us, too bad!

  //Check if Rotation will hit another piece
  unsigned char temp_area[2];
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
      if (BOX_location[x_loc+i] & 1<<(y_loc-j)) temp_area[i/2] |= 1<<((4*(i%2))+(3-j)); 	//Box already here: 1
      else temp_area[i/2] &= ~(1<<((4*(i%2))+(3-j)));	//No Box: 0
    }
  }
  if ((temp_area[0] & new_position[0]) || (temp_area[1] & new_position[1])) { BOX_store_loc(); return; }


  //Rotation will not cause an overlap or overflow and can proceed
  BOX_clear_piece();
  if (++rotate > 3) rotate = 0;
  BOX_load_reference(cur_piece, rotate);
  BOX_write_piece();
  BOX_store_loc();
}

void BOX_draw(unsigned char X, unsigned char Y, unsigned char color)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(X*4, 0);
  LCD_Out((X*4)+3, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(Y*4, 0);
  LCD_Out((Y*4)+3, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (unsigned char i=0; i<16; i++) LCD_Out(color,0);
}

void BOX_erase(unsigned char X, unsigned char Y, unsigned char color)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(X*4, 0);
  LCD_Out((X*4)+3, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(Y*4, 0);
  LCD_Out((Y*4)+3, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (unsigned char i=0; i<16; i++) LCD_Out(white,0);
}

void BOX_write_piece(void)  //Writes piece to display
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
    //prevent invalid indicies from being written
      if ((y_loc-j) >= BOX_board_top)
      {
	if (BOX_piece[i/2] & 1<<((4*(i%2))+(3-j)))
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
	if (BOX_piece[i/2] & 1<<((4*(i%2))+(3-j)))
	{
	  //TODO: Change this for different bg colors on each level
	  BOX_erase(x_loc+i, y_loc-j, default_bg_color);
	}
      }
    }
  }
}

void BOX_rewrite_display(unsigned char fgcolor, unsigned char bgcolor)	//Rewrites entire playing area
{
  unsigned char array_depth = (BOX_board_bottom+1)/8;
  if ((BOX_board_bottom+1)%8) ++array_depth;

  //cycle through 8 row chunks
  for (unsigned char row_chunk=0; row_chunk<array_depth; row_chunk++)
  {
    //cycle through columns
    for (unsigned char temp_col=0; temp_col<=BOX_board_right; temp_col++)
    {
      //cycle through all rows in block
      for (unsigned char temp_row=0; temp_row<8; temp_row++)	//We are working in 8 row chunks
      {
	//Make sure we're not out of bounds
	if ((row_chunk*8)+temp_row <= BOX_board_bottom)
	{
	  if(BOX_loc_return_bit(temp_col,(row_chunk*8)+temp_row)) BOX_draw(temp_col,(row_chunk*8)+temp_row, fgcolor);
	  else BOX_draw(temp_col,(row_chunk*8)+temp_row, bgcolor);
	}
      }
    }
  }
}

void BOX_spawn(void)
{
  x_loc = 4;
  y_loc = 0;
  cur_piece = random_piece;

  BOX_load_reference(cur_piece, rotate);  //load from reference

  //calculate y_loc
  for (unsigned char i=0; i<3; i++)
  {
    if((BOX_piece[0] | BOX_piece[1]) & 0x11<<i) ++y_loc; //There is a box in the row, make sure we see it
  }

  //Check to see if we've filled the screen
  for (unsigned char i=0; i<=y_loc; i++)
  {
    for (unsigned char j=0; j<4; j++)
    {
      if ((BOX_location[x_loc+j] & 1<<i)
      && (BOX_piece[j%2] & 1<<((4*(j%2))+(3-i))))
      {
	BOX_end_game();
	break;
      }
    }
  }

  BOX_store_loc(); //Store new location
  BOX_write_piece(); //draw piece
}

//TODO: finish this development
unsigned char BOX_check_rev2(signed char X_offset, signed char Y_offset)
{
	unsigned char temp_area[2];
	unsigned char i;
	//Build compare mask in temp_area[]
	//mask will be 4 sets of nibbles (2 bytes)
    for (i=0; i<4; i++) { }//Column 1
	//Column 2
	//Column 3
	//Column 4

	//Subtract current piece location from compare mask

	//Return a value
}
unsigned char BOX_check(signed char X_offset, signed char Y_offset)
{
  //Check to see if we overlap a piece or the side of the board
  //Return 1 = there IS an overlap
  //Return 0 = NO Overlap


  //Check left
  if (X_offset < 0)
  {
    if (((x_loc == BOX_board_left) && (BOX_piece[0] & 0x0F)) || (x_loc > BOX_board_right)) return 1;
  }
  //Check right
  if (X_offset > 0)
  {
    if (((unsigned char)(x_loc+X_offset+3) > BOX_board_right) && (BOX_piece[1] & 0xF0)) return 1;
    if (((unsigned char)(x_loc+X_offset+2) > BOX_board_right) && (BOX_piece[1] & 0x0F)) return 1;
    if ((unsigned char)(x_loc+X_offset+1) > BOX_board_right) return 1;
  }
  //Check bottom
  if (Y_offset > 0)
  {
    if (((y_loc == BOX_board_bottom) && ((BOX_piece[0] | BOX_piece[1]) & 0x88)) || ((y_loc-1 == BOX_board_bottom) && ((BOX_piece[0] |BOX_piece[1]) & 0x44))) return 1; //Do nothing if we're at the bottom already
  }

  //Get 4x4 grid area on playing board
  BOX_clear_loc();	//Clear the current location so we don't get a false reading from it
  unsigned char temp_area[2];
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
      //Weed out edges, previous checks ensure we have not left the playing area with the piece.
      if ((unsigned char)(x_loc+X_offset+i) < BOX_board_left) temp_area[i/2] &= ~(1<<((4*(i%2))+(3-j)));	//Off left: 0
      else if ((unsigned char)(x_loc+X_offset+i) > BOX_board_right) temp_area[i/2] &= ~(1<<((4*(i%2))+(3-j)));	//Off right: 0
      else if ((unsigned char)(y_loc+Y_offset-j) < BOX_board_top) temp_area[i/2] &= ~(1<<((4*(i%2))+(3-j)));	//Off top: 0
      else if ((unsigned char)(y_loc+Y_offset-j) > BOX_board_bottom) temp_area[i/2] &= ~(1<<((4*(i%2))+(3-j)));	//Off bottom: 0
      //else if (BOX_location[(unsigned char)(x_loc+X_offset+i)] & 1<<((unsigned char)(y_loc+Y_offset-j))) temp_area[i/2] |= 1<<((4*(i%2))+(3-j)); 	//Box already here: 1
      else if (BOX_loc_return_bit((unsigned char)(x_loc+X_offset+i),(unsigned char)(y_loc+Y_offset-j)));
      else temp_area[i/2] &= ~(1<<((4*(i%2))+(3-j)));	//No Box: 0
    }
  }
  BOX_store_loc();	//Store the current location that we cleared earlier

  //Compare two arrays for overlaps
  if ((BOX_piece[0] & temp_area[0]) || (BOX_piece[1] & temp_area[1])) return 1;


  return 0;
}

void BOX_line_check(void)
{
  //TODO: Tweak this to enable scoring

  //Check every line on the playing area for complete rows and record them in an array
  //TODO: make this work for more than 8 rows
  unsigned char complete_lines[4];
  unsigned char temp_index = 0;			//Index for complete_lines[]
  for (unsigned char i=0; i<=BOX_board_bottom; i++)
  {
    unsigned char j=0;
    while ((j<=BOX_board_right) && (BOX_location[j] & 1<<i))
    {
      if (j == BOX_board_right) complete_lines[temp_index++] = i; //Complete row found, record in complete_lines[]
      ++j;
    }
  }
  if (temp_index == 0) return;  //If no complete rows, return

  /*
  for (unsigned char i=0; i < temp_index; i++)
  {
    for (unsigned char j=0; j <= BOX_board_right; j++) BOX_draw(j, complete_lines[i], green);//Test to see if this works.
  }
  */

  //If there are complete rows
    //TODO: Disable interrupts to pause game flow
    //TODO: Add an arbitrary delay, perhaps make complete lines flash?

  //Rewrite BOX_location[] without completed rows.
  --temp_index;	//This was incremented one too many times earlier, get it back to the proper index.
  unsigned char row_write_tracker = 0; //Tracks how many rows above the current row we are getting information from.
  for (unsigned char i=0; i<=BOX_board_bottom; i++)
  {
    if (complete_lines[temp_index] == (BOX_board_bottom-i-row_write_tracker))
    {
      ++row_write_tracker;
      if (temp_index > 0) --temp_index;
    }
    if (i+row_write_tracker > BOX_board_bottom)	//Clear with zeros if all rows have been shifted down
    {
      for (unsigned char j=0; j<=BOX_board_right; j++) BOX_location[j] &= ~(1<<(BOX_board_bottom-i));
    }
    else for (unsigned char j=0; j<=BOX_board_right; j++)
    {
      //if the bit in the row above the full row is 1, set that in the full row
      if (BOX_location[j] & 1<<(BOX_board_bottom-i-row_write_tracker)) BOX_location[j] |= 1<<(BOX_board_bottom-i);
      //otherwise make sure that bit is unset
      else BOX_location[j] &= ~(1<<(BOX_board_bottom-i));
    }
  }
  BOX_rewrite_display(blue, white);
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
    BOX_rewrite_display(blue, default_bg_color);
//NOTE:testing    BOX_line_check();
    BOX_spawn();
    return;
  }

  BOX_clear_loc();
  BOX_clear_piece();
  ++y_loc;
  BOX_write_piece();
  BOX_store_loc();
}

void BOX_lt(void)
{
  if (BOX_check(-1, 0)) return; //Do nothing if moving causes an overlap
  //if (((x_loc == BOX_board_left) && (BOX_piece[0] & 0x0F)) || (x_loc == 255)) return; //Do nothing if we're at the left edge already
  //if (BOX_location[x_loc-1] & 1<<y_loc) return; //Do nothing if there is a box beside us
  BOX_clear_loc();
  BOX_clear_piece();
  x_loc--;
  BOX_write_piece();
  BOX_store_loc();
}

void BOX_rt(void)
{
  if (BOX_check(1, 0)) return; //Do nothing if moving causes an overlap
  BOX_clear_loc();
  BOX_clear_piece();
  ++x_loc;
  BOX_write_piece();
  BOX_store_loc();
}

void BOX_pregame(void)
{
  LCD_Fill_Screen(yellow);

  cursor_x = 18;
  cursor_y = 9;
  LCD_Write_String_P(message1,green,yellow);

  cursor_x = 6;
  cursor_y = 20;
  LCD_Write_String_P(message2,black,yellow);
}

void BOX_start_game(void)
{
  //Poplulate BOX_location[] with 0b00000000
  for (unsigned char i=0; i<array_size; i++) { BOX_location[i] = 0x00; }
  BOX_rewrite_display(blue, white);
  BOX_spawn();
}

void BOX_end_game(void)
{
  TCCR1B &= ~(1<<CS12 | 1<<CS11 | 1<<CS10);	//stop timer
  BOX_rewrite_display(black,red);
  cursor_x = 24;
  cursor_y = 29;
  LCD_Write_String_P(message3,white,black);
  while(1) { }
}
