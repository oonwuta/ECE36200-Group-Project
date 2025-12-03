#include "audio.h"

//support.c from lab 5
int step0 = 0;
int offset0 = 0;
int step1 = 0;
int offset1 = 0;
int volume = 2400;

void init_wavetable(void) {
    for(int i=0; i < N; i++)
        wavetable[i] = (16383 * sin(2 * M_PI * i / N)) + 16384;
}

void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0) {
            step0 = 0;
            offset0 = 0;
        } else
            step0 = (f * N / RATE) * (1<<16);
    }
    if (chan == 1) {
        if (f == 0.0) {
            step1 = 0;
            offset1 = 0;
        } else
            step1 = (f * N / RATE) * (1<<16);
    }
}

//from lab 5
void pwm_audio_handler() {
    uint slice_num = pwm_gpio_to_slice_num(36);
    uint chan = pwm_gpio_to_channel(36);
    pwm_clear_irq(slice_num);
    uint period = (1000000 / RATE) - 1;

    offset0 += step0;
    offset1 += step1;

    if (offset0 >= (N << 16)) 
    {
        offset0 -= (N << 16);
    }
    if (offset1 >= (N << 16))
    {
        offset1 -= (N << 16);
    }

    uint samp = wavetable[offset0 >> 16] + wavetable[offset1 >> 16];
    samp = (samp * period)/(1 << 16);

    pwm_set_chan_level(slice_num, chan, samp);
}


void init_pwm_audio() {
    gpio_set_function(36, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(36);
    uint chan = pwm_gpio_to_channel(36);

    //set clock divider
    pwm_set_clkdiv(slice_num, 150);

    //period calcuation
    uint period = (1000000 / RATE) - 1;
    pwm_set_wrap(slice_num, period);

    //duty cycle to 0
    pwm_set_chan_level(slice_num, chan, 0);

    init_wavetable();

    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);

    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_set_enabled(slice_num, true);
}