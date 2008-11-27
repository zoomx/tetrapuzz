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
* lfuse=0xE2          *
* hfuse=0xDF          *
* efuse=0x01          *
* Power=3.3v          *
**********************/

//Debounce
unsigned char debounce_cnt = 0;
volatile unsigned char key_press;
unsigned char key_state;

//Game timer
volatile unsigned char drop_timer_flag = 0;

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
  cli();
  //Timer0 for buttons
  TCCR0B |= 1<<CS02 | 1<<CS00;		//Divide by 1024
  TIMSK0 |= 1<<TOIE0;			//enable timer overflow interrupt

  //Timer1 for game timer
  TCCR1B |= 1<<CS12 | 1<<CS10;		//Divide by 1024
  TIMSK1 |= 1<<TOIE1;			//enable timer overflow interupt
  TCNT1 = (unsigned int)(signed short)-(F_CPU / 1024);  //Preload the timer for 1 second
  sei();
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
  random_piece = 0;
  init_io();
  init_timers();
  LCD_init();

  LCD_Fill_Screen(white);

  cursor_x = 8;
  cursor_y = 9;
  LCD_Write_String("Enter to Start",green,yellow);
  
  while(get_key_press(1<<BTN_ENTER) == 0) { }
  BOX_start_game();

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
    if (drop_timer_flag)
    {
      drop_timer_flag = 0;
      BOX_dn();
    }
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

  if (++random_piece > 6) random_piece = 0;
}

//Game timer
ISR(TIMER1_OVF_vect)
{
  //TODO: Make the overflow time shorter as the game advances in level
  TCNT1 = (unsigned int)(signed short)-(F_CPU / 1024);  //Preload the timer for 1 second
  drop_timer_flag = 1;					//Set flag so that piece will drop
}