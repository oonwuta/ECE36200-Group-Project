#pragma once
#include <stdint.h>
#include <stdbool.h>

// Initialize joystick sampling with a given interval in microseconds
void joystick_init(uint32_t interval_us);

// Get latest normalized values (-1.0 to 1.0)
void joystick_read(float *x, float *y);

void button_init();
bool button_read();