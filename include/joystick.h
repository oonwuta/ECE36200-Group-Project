#pragma once
#include <stdint.h>
#include <stdbool.h>

// Initialize joystick sampling with a given interval in microseconds
void joystick_init();

// Get latest normalized values (-1.0 to 1.0)
void joystick_read(float *x, float *y, float *v);

void button_init();
bool button_read();
