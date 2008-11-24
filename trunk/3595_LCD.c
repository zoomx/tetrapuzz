#include "3595_LCD.h"
#include "font5x8.h"

/***************************
* Initialization Functions *
***************************/

void LCD_init(void)
{
  LCD_DDR |= (LCD_CLK | LCD_SIO | LCD_CS | LCD_RST);

  //Hardware Reset
  LCD_PORT &= ~LCD_RST;
  LCD_PORT |= LCD_RST;
  _delay_ms(5);

  LCD_PORT |= (LCD_CLK | LCD_SIO | LCD_CS);

  //Software Reset
  LCD_Out(0x01, 1);
  _delay_ms(10);

/*
  //Refresh set
  LCD_Out(0xB9, 1);
  LCD_Out(0x00, 0);
*/


  //Display Control
  LCD_Out(0xB6, 0);
  LCD_Out(128, 0);
  LCD_Out(128, 0);
  LCD_Out(129, 0);
  LCD_Out(84, 0);
  LCD_Out(69, 0);
  LCD_Out(82, 0);
  LCD_Out(67, 0);


/*
  //Temperature gradient set
  LCD_Out(0xB7, 1);
  for(char i=0; i<14; i++)  LCD_Out(0, 0);
*/

  //Booster Voltage On
  LCD_Out(0x03, 1);
  _delay_ms(50);  //NOTE: At least 40ms must pass between voltage on and display on.
		  //Other operations may be carried out as long as the display is off
		  //for this length of time.

/*
  //Test Mode
  LCD_Out(0x04, 1);
*/

/*
  // Power Control
  LCD_Out(0xBE, 1);
  LCD_Out(4, 0);
*/

  //Sleep Out
  LCD_Out(0x11, 1);

  //Display mode Normal
  LCD_Out(0x13, 1);

  //Display On
  LCD_Out(0x29, 1);

  //Set Color Lookup Table
  LCD_Out(0x2D, 1);		//Red and Green (3 bits each)
  char x, y;
  for(y = 0; y < 2; y++) {
	  for(x = 0; x <= 14; x+=2) {
		  LCD_Out(x, 0);
	  }
  }
  //Set Color Lookup Table	//Blue (2 bits)
  LCD_Out(0, 0);
  LCD_Out(4, 0);
  LCD_Out(9, 0);
  LCD_Out(14, 0);

  //Set Pixel format to 8-bit color codes
  LCD_Out(0x3A, 1);
  LCD_Out(0b00000010, 0);

//***************************************
//Initialization sequence from datasheet:

//Power to chip
//RES pin=low
//RES pin=high -- 5ms pause
//Software Reset
//5ms Pause
//INIESC
  //<Display Setup 1>
    //REFSET
    //Display Control
    //Gray Scale position set
    //Gamma Curve Set
    //Common Driver Output Select
  //<Power Supply Setup>
    //Power Control
    //Sleep Out
    //Voltage Control
    //Write Contrast
    //Temperature Gradient
    //Boost Voltage On
  //<Display Setup 2>
    //Inversion On
    //Partial Area
    //Vertical Scroll Definition
    //Vertical Scroll Start Address
  //<Display Setup 3>
    //Interface Pixel Format
    //Colour Set
    //Memory access control
    //Page Address Set
    //Column Address Set
    //Memory Write
  //Display On

//****************************************
}

/************
* Functions *
************/

//LCD_Out function comes from source code found here:
//http://hobbyelektronik.org/Elo/AVR/3510i/index.htm
//Unfortunately this is the only way I know to attribute
//this code to the writer.
void LCD_Out(unsigned char Data, unsigned char isCmd) 
{
	if(isCmd) LCD_PORT |= LCD_CS;  
	LCD_PORT &= ~(LCD_CLK|LCD_CS);  //Clock and CS low

	LCD_PORT |= LCD_SIO;		//SData High
	if(isCmd) LCD_PORT &= ~LCD_SIO; //If it is a command, SData Low

	LCD_PORT |= LCD_CLK;		//Clock High

	for(char x=0; x<8; x++)	{
		LCD_PORT &= ~(LCD_SIO|LCD_CLK);		//Clock and SData low
		if(Data & 128) LCD_PORT |= LCD_SIO;  	// Mask MSB - SData high if it is a 1
		LCD_PORT |= LCD_CLK;			//Clock High
		Data=Data<<1;				//Shift bits 1 left (new MSB to be read)
	}
}

void LCD_Flash_BW(unsigned int flash_delay_ms)
{
  LCD_Out(0x13, 1);
  //All pixel ON
  LCD_Out(0x23, 1);
  _delay_ms(flash_delay_ms);
  
  LCD_Out(0x13, 1);
  //All pixel OFF
  LCD_Out(0x22, 1);
  _delay_ms(flash_delay_ms);
  
  LCD_Out(0x13, 1);
}

void LCD_XorScreen(void)
{
  //Screen is 96x65

  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(0, 0);
  LCD_Out(97, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(0, 0);
  LCD_Out(66, 0);
  LCD_Out(0x2C, 1); //Write Data
  //Row 0-64
  for (char i=0; i<=66; i++)
  {
    //Column 0-95
    for(char j=0; j<=97; j++)
    {
      LCD_Out(j^i, 0);
    }
  }
}

void LCD_StripedScreen(void)
{
  unsigned char color_palate[] = {
	//BBGGGRRR
	0b00000111,	//Red
	0b00111111,	//Yellow
	0b00111100,	//Green
	0b11111000,	//Cyan
	0b11000000,	//Blue
	0b11000111,	//Magenta
	0b11111111,	//White
	0b00000111	//This should be 0x00(black) but for screen wrapping it was changed to Red
  };

  LCD_Out(0x13, 1);
  for (unsigned char i=0; i<8; i++)
  {
    LCD_Out(0x2A, 1);
    LCD_Out(0, 0);
    LCD_Out(97, 0);
    LCD_Out(0x2B, 1);
    LCD_Out(i*9, 0);
    LCD_Out((i*9)+8, 0);
    LCD_Out(0x2C, 1);
    for (int j=0; j<882; j++)
    {
      LCD_Out(color_palate[i], 0);
    }
  }
}

void LCD_Hello_World(void)
{
  //Binary representation of "Hello World"
  unsigned char Hello_World[5][5] = {
    { 0b10101110, 0b10001000, 0b01001010, 0b10010011, 0b00100110 },
    { 0b10101000, 0b10001000, 0b10101010, 0b10101010, 0b10100101 },
    { 0b11101100, 0b10001000, 0b10101010, 0b10101011, 0b00100101 },
    { 0b10101000, 0b10001000, 0b10101010, 0b10101010, 0b10100101 },
    { 0b10101110, 0b11101110, 0b01000101, 0b00010010, 0b10110110 }
  };

    LCD_Out(0x2A, 1);
    LCD_Out(8, 0);
    LCD_Out(87, 0);
    LCD_Out(0x2B, 1);
    LCD_Out(23, 0);
    LCD_Out(32, 0);
    LCD_Out(0x2C, 1);
    for (unsigned char i=0; i<5; i++) //Scan Rows
    {
      char h=2;
      while(h)
      {
	for (unsigned char k=0; k<5; k++) //Scan Columns
	{
	  for (char j=0; j<8; j++)
	  {
	    if (Hello_World[i][k] & 1<<(7-j))	//Should there be a letter pixel here?
	    {
	      LCD_Out(0x00, 0);			//yes - draw it in black
	      LCD_Out(0x00, 0);			
	    }
	    else 
	    {
	      LCD_Out(0xFF, 0);			//no - draw background in white
	      LCD_Out(0xFF, 0);
	    }
	  }
	}
	--h;
      }
    }
}

/********************************
* LCD_Advance_Cursor() function *
*******************************************************
* This function is used to increment a virtual cursor *
* to facilitate writing strings to the screen.  After *
* one char is written Advance_Cursor() will move to   *
* the next available cursor position, wrapping lines  *
* if necessary                                        *
******************************************************/

void LCD_Advance_Cursor(void)
{
  //TODO: Need space between letters and lines???
  cursor_x += 6;
  if (cursor_x+5 > page_size)
  {
    cursor_x = 0;
    cursor_y += 8;
    if (cursor_y+7 > row_size) cursor_y = 0;
  }
}

void LCD_Write_String(char * myString, unsigned char fgcolor, unsigned char bgcolor)
{
  while (*myString)			//Repeat until a null character is encountered (signals end of a string)
  {
    LCD_Write_Char(*myString,fgcolor,bgcolor);		//Scroll in the next character
    LCD_Advance_Cursor();
    ++myString;				//Increment the point which tracks where we are in the string

  }
}


void LCD_Write_Char(unsigned char letter, unsigned char fgcolor, unsigned char bgcolor)		//Function that writes one character to display
{
  //TODO: Prevent non-valid characters from crashing program
  
  //Setup display to write one char:
  LCD_Out(0x2A, 1);
  LCD_Out(cursor_x, 0);
  LCD_Out(cursor_x+5, 0);
  LCD_Out(0x2B, 1);
  LCD_Out(cursor_y, 0);
  LCD_Out(cursor_y+7, 0);
  LCD_Out(0x2C, 1);
  
  //letters come from font5x8[] in progmem (font5x8.h)
  letter -= 32;						//Adjust char value to match our font array indicies
  unsigned char temp[5];
  for (unsigned char i=0; i<5; i++)				//Read one column of char at a time
  {
    temp[i] = pgm_read_byte((char *)((int)font5x8 + (5 * letter) + i));	//Get column from progmem
  }
  for (unsigned char j=0; j<8; j++)						//Cycle through each bit in column
  {
      LCD_Out(bgcolor, 0);
      for (unsigned char k=0; k<5; k++)
      {
	if (temp[k] & 1<<j) LCD_Out(fgcolor, 0);
	else LCD_Out(bgcolor, 0);
      }
  }
}

void LCD_Fill_Screen(unsigned char color)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(0, 0);
  LCD_Out(97, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(0, 0);
  LCD_Out(66, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (int i=0; i<6566; i++) LCD_Out(color, 0);
}