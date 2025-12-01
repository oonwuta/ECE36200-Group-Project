#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

void joystick_init();
void init_dma();
void joystick_read(uint16_t* x_out, uint16_t* y_out);
bool button_read();