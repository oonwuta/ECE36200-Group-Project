#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

uint16_t buffer[2] = {0,0}; //buffer for dma to write to

#define joystickX 4 //dummy number for now
#define joystickY 3 //dummy number for y
#define joystickbutton 2
#define joysticXadc 8 //adc channel for joystick pins
#define joysticYadc 9 //adc channel for other pin

/*
The way this works is first the joystick is initialized with the X and Y pins set to ADC. The function
starts selecting X as the adc input and is put into round robin mode. dma is initialized to read from the adc
fifo and write to the xpos variable. The point is to run joystick init and then continuously run
joystick read so that xpos and ypos are continously changing and updated with the value from the DMA,
*/

void init_dma() {
    dma_hw->ch[0].read_addr = (uint32_t)&(adc_hw->fifo);
    dma_hw->ch[0].write_addr = (uint32_t)(buffer);
    dma_hw->ch[0].transfer_count = 0x2 | (0x1 <<28); // effectively infinite
    dma_hw->ch[0].ctrl_trig = 0;
    uint32_t temp = (0x1 << 2) | (0x1 << 6) |(DREQ_ADC << 17)| (2 << 12) |(0x2 << 8)| DMA_CH0_CTRL_TRIG_EN_BITS; //need to check these
    dma_hw->ch[0].ctrl_trig = temp;
    
}

void joystick_init() {
    // Initialize ADC hardware
    adc_init();
    // Enable the ADC input on the specified GPIO pins
    adc_gpio_init(joystickX);
    adc_gpio_init(joystickY);
    //gpio_init(joystickbutton);
    //gpio_set_dir(joystickbutton, GPIO_IN); //a lot of options for how to handle this either an interrupt can be enabled for the start screen and highscore screen but if button needs to be debounced dma may be better
    //adc_select_input(joysticYadc); // Select Y axis for initial read
    adc_set_round_robin(1 << joysticXadc | 1 << joysticYadc); //enable round robin on both channels
    adc_run(true);
    init_dma();
}

void joystick_read(uint16_t* x_out, uint16_t* y_out) {
    // Read X axis
    *x_out = buffer[0];
    *y_out = buffer[1];
}

bool button_read(){
    return false;
}