#include "led_matrix.h"

//temporarily here until I know which pins to use
const int PIN_R1 = 9;       //These next few are self explanatory
const int PIN_G1 = 21;
const int PIN_B1 = 10;
const int PIN_R2 = 11; 
const int PIN_G2 = 20;
const int PIN_B2 = 12;      
const int PIN_LRT = 17;     //latch pin
const int PIN_CLK = 15;     //clock pin
const int PIN_OE = 16;      //output enable of panel (active low)
const int PIN_A  = 13;      //row address bit A
const int PIN_B  = 19;      //row address bit B
const int PIN_C  = 14;      //row address bit C
const int PIN_D  = 18;      //row address bit D  

#define SET_PIN(pin)    (sio_hw->gpio_set = (1u << (pin)))
#define CLR_PIN(pin)    (sio_hw->gpio_clr = (1u << (pin)))

static inline void set_row_addr(uint8_t row) {
    (row & 1) ? SET_PIN(PIN_A) : CLR_PIN(PIN_A);
    (row & 2) ? SET_PIN(PIN_B) : CLR_PIN(PIN_B);
    (row & 4) ? SET_PIN(PIN_C) : CLR_PIN(PIN_C);
    (row & 8) ? SET_PIN(PIN_D) : CLR_PIN(PIN_D);
}

// Shift one pixel worth of data into panel (for both halves)
static inline void shift_pixel() {
    // Set data lines (Green ON)
    CLR_PIN(PIN_R1);
    SET_PIN(PIN_G1);
    CLR_PIN(PIN_B1);

    CLR_PIN(PIN_R2);
    SET_PIN(PIN_G2);
    CLR_PIN(PIN_B2);

    // Clock it in
    SET_PIN(PIN_CLK);
    CLR_PIN(PIN_CLK);
}

void matrix_init(void){
    for(int i = 9; i < 22; i++){
        gpio_init(i); 
        gpio_set_dir(i,GPIO_OUT);
    }
    CLR_PIN(PIN_OE);
}

void matrix_test(void){
    matrix_init(); 

    while (1) {
        for (uint8_t row = 0; row < 16; row++) {  // 1:16 multiplex
            set_row_addr(row);

            // Shift 32 pixels (columns)
            for (int i = 0; i < 32; i++) {
                shift_pixel();
            }

            // Latch new row data
            SET_PIN(PIN_LRT);
            CLR_PIN(PIN_LRT);

            // Enable output briefly
            SET_PIN(PIN_OE);
            sleep_us(300);   // Display time per row
            CLR_PIN(PIN_OE);
        }
    }
}


