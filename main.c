#include <avr/io.h>
#include <avr/interrupt.h>

#include "3595_LCD.h"
#include "box_game.c"

//Debounce definitions
#define KEY_DDR 	DDRD
#define KEY_PIN 	PIND
#define KEY_PORT 	PORTD
#define BTN_UP		6
#define BTN_DN		5
#define BTN_LT		7
#define BTN_RT		4
#define BTN_ENTER	3
#define KEY_MSK 	(1<<BTN_UP | 1<<BTN_DN | 1<<BTN_LT | 1<<BTN_RT | 1<<BTN_ENTER)

/**Device Information**
* ATmega168           *
* lfuse=          *
* hfuse=          *
* efuse=          *
* Power=3.3v          *
**********************/

//Debounce
unsigned char debounce_cnt = 0;
volatile unsigned char key_press;
unsigned char key_state;

/***************************
* Initialization Functions *
***************************/

void init_io(void)
{
  KEY_DDR |= ~KEY_MSK;		//Set button pins as inputs
  KEY_PORT |= KEY_MSK;	//Enable internal pullups
  
}

void init_timers(void)
{
 //Timer0 for buttons
  TCCR0B |= 1<<CS02 | 1<<CS00;                //divide by 1024
  TIMSK0 |= 1<<TOIE0;                       //enable timer interrupt
}

/*********************
* Debounce Functions *
*********************/

unsigned char get_key_press(unsigned char key_mask)
{
  cli();               // read and clear atomic !
  key_mask &= key_press;                        // read key(s)
  key_press ^= key_mask;                        // clear key(s)
  sei();
  return key_mask;
}

/****************
* Main Function *
****************/

int main(void)
{
  init_io();
  init_timers();
  LCD_init();

  LCD_Fill_Screen(white);
  BOX_spawn();

  while(1)
  {
    if (get_key_press(1<<BTN_UP))
    {
      BOX_up();
    }    

    if (get_key_press(1<<BTN_DN)) 
    {
      BOX_dn();
    }

    if (get_key_press(1<<BTN_LT)) { 
      BOX_lt();
    }

    if (get_key_press(1<<BTN_RT)) 
    { 
      BOX_rt();
    }
    
    if (get_key_press(1<<BTN_ENTER))
    {
      BOX_rotate(1);
    }

  /*  
    if (get_key_press(1<<BTN_UP)) {
      LCD_XorScreen();	//Fill screen with an XOR pattern, pause for 2 seconds 
    }

    if (get_key_press(1<<BTN_DN)) {
      LCD_StripedScreen();	//Fill Screen with colored stripes, pause for 2 seconds
    }

    if (get_key_press(1<<BTN_LT)) { 
      LCD_Hello_World();	//Write "Hello World", pause for 10 seconds
    }
    
    if (get_key_press(1<<BTN_RT)) { 
      cursor_x = 8;
      cursor_y = 12;
      LCD_Write_String("Hello World", 0x00, 0xFF);
    }
    
    if (get_key_press(1<<BTN_ENTER)) { 
      LCD_Fill_Screen(white);  
      cursor_x = 0;
      cursor_y = 12;
      for (char ascii_letter=32; ascii_letter<128; ascii_letter++)
      {
	LCD_Write_Char(ascii_letter,blue,white);
	LCD_Advance_Cursor();
      }
    }
  */  



  }
}

/*************
* Interrupts *
*************/

//Debounce Overflow timer
ISR(TIMER0_OVF_vect)           // every 10ms
{
  static unsigned char ct0, ct1;
  unsigned char i;

  TCNT0 = (unsigned char)(signed short)-(F_CPU / 1024 * 10e-3 + 0.5);   // preload for 10ms
  //TCNT0 = 177;

  i = key_state ^ ~KEY_PIN;    // key changed ?
  ct0 = ~( ct0 & i );          // reset or count ct0
  ct1 = ct0 ^ (ct1 & i);       // reset or count ct1
  i &= ct0 & ct1;              // count until roll over ?
  key_state ^= i;              // then toggle debounced state
  key_press |= key_state & i;  // 0->1: key press detect
}