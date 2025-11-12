#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/multicore.h"
#include <string.h>
#include <stdbool.h>

extern const int PIN_R1;       //These next few are self explanatory
extern const int PIN_G1;
extern const int PIN_B1;
extern const int PIN_R2; 
extern const int PIN_G2;
extern const int PIN_B2;      
extern const int PIN_LRT;     //latch pin
extern const int PIN_CLK;     //clock pin
extern const int PIN_OE;      //output enable of panel (active low)
extern const int PIN_A;      //row address bit A
extern const int PIN_B;      //row address bit B
extern const int PIN_C;      //row address bit C
extern const int PIN_D;

static inline void set_row_addr(uint8_t row);
static inline void shift_pixel(void);
void matrix_init(void);
void matrix_test(void);