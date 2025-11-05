#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

//support.h from lab 5
#define N 1000 // Size of the wavetable
short int wavetable[N];
#define RATE 20000

void init_wavetable(void);
void set_freq(int chan, float f);
void init_pwm_audio();
void pwm_audio_handler();