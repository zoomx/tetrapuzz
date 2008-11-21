#include <avr/pgmspace.h>

#define BOX_board_top		0
#define BOX_board_bottom	7
#define BOX_board_left		0
#define BOX_board_right		11

//Prototypes
void BOX_store_loc(void);
void BOX_clear_loc(void);
void BOX_load_reference(unsigned char piece, unsigned char rotation);
void BOX_rotate(unsigned char direction);
void BOX_draw(unsigned char X, unsigned char Y);
void BOX_erase(unsigned char X, unsigned char Y);
void BOX_write_piece(void);
void BOX_clear_piece(void);
void BOX_spawn(void);
void BOX_up(void);
void BOX_dn(void);
void BOX_lt(void);
void BOX_rt(void);