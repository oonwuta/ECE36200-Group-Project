#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

uint16_t buffer[2] = {0,0}; //buffer for dma to write to
bool button_state = false;

#define joystickX 41 //dummy number for now
#define joystickY 40 //dummy number for y
#define joystickbutton 25
#define joysticXadc 1 //adc channel for joystick pins
#define joysticYadc 0 //adc channel for other pin
#define OVERSAMPLE 64 


uint16_t x_buffer[OVERSAMPLE];
uint16_t y_buffer[OVERSAMPLE];

// Calibration data
float x_min = 3.3f, x_max = 0.0f;
float y_min = 3.3f, y_max = 0.0f;
bool calibrated = false;
float x_center = 0.0f;
float y_center = 0.0f;
static float x_filtered = 0.0f;
static float y_filtered = 0.0f;
static float x_center = 0.0f;
static float y_center = 0.0f;

static const float alpha = 0.15f;   // Low-pass filter strength

// DMA channels
int dma_x_chan, dma_y_chan;

/*
The way this works is first the joystick is initialized with the X and Y pins set to ADC. The function
starts selecting X as the adc input and is put into round robin mode. dma is initialized to read from the adc
fifo and write to the xpos variable. The point is to run joystick init and then continuously run
joystick read so that xpos and ypos are continously changing and updated with the value from the DMA,
*/

int dma_init_channel(uint16_t *buffer, uint dreq, uint adc_input) {
    int chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, dreq);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    dma_channel_configure(chan, &c, buffer, &adc_hw->fifo, OVERSAMPLE, false);
    return chan;
}

// Initialize ADC and DMA
void joystick_init() {
    adc_init();
    adc_gpio_init(joystickX);
    adc_gpio_init(joystickY);

    adc_fifo_setup(true, true, 1, false, false);
    adc_run(true);

    // Setup DMA for X channel
    adc_select_input(joysticXadc);
    dma_x_chan = dma_init_channel(x_buffer, DREQ_ADC, joysticXadc);
    dma_channel_start(dma_x_chan);

    // Setup DMA for Y channel
    adc_select_input(joysticYadc);
    dma_y_chan = dma_init_channel(y_buffer, DREQ_ADC, joysticYadc);
    dma_channel_start(dma_y_chan);
}

// Read joystick axes, apply oversampling, calibration, and normalization
void joystick_read(float *x_out, float *y_out) {
    // Wait for DMA transfer complete (or polling could be used)
    dma_channel_wait_for_finish_blocking(dma_x_chan);
    dma_channel_wait_for_finish_blocking(dma_y_chan);

    // Compute averages
    uint32_t x_sum = 0, y_sum = 0;
    for (int i = 0; i < OVERSAMPLE; i++) {
        x_sum += x_buffer[i];
        y_sum += y_buffer[i];
    }
    float x_raw = (x_sum / (float)OVERSAMPLE) * 3.3f / 4095.0f;
    float y_raw = (y_sum / (float)OVERSAMPLE) * 3.3f / 4095.0f;

    // Auto-calibrate min/max
    if (x_raw < x_min) x_min = x_raw;
    if (x_raw > x_max) x_max = x_raw;
    if (y_raw < y_min) y_min = y_raw;
    if (y_raw > y_max) y_max = y_raw;

    // Normalize to -1..1
    float x_norm = 0, y_norm = 0;
    if ((x_max - x_min) > 0.001f) x_norm = 2.0f * (x_raw - x_min) / (x_max - x_min) - 1.0f;
    if ((y_max - y_min) > 0.001f) y_norm = 2.0f * (y_raw - y_min) / (y_max - y_min) - 1.0f;

    *x_out = x_norm;
    *y_out = y_norm;

    // Restart DMA for next batch
    dma_channel_start(dma_x_chan);
    dma_channel_start(dma_y_chan);
}



void button_init(){
    gpio_init(joystickbutton);
    gpio_set_dir(joystickbutton, GPIO_IN);
    gpio_pull_up(joystickbutton);
}

static float x_min = 0.2f, x_max = 3.1f;
static float y_min = 0.2f, y_max = 3.1f;

void joystick_read(float* x_out, float* y_out) {

    float x_raw = (buffer[0] * 3.3f) / 4095.0f;
    float y_raw = (buffer[1] * 3.3f) / 4095.0f;

    if (!calibrated) {
        x_center = x_raw;
        y_center = y_raw;
        calibrated = true;
    }

    // Normalize with fixed ranges
    float x_norm = (x_raw - x_center) / ((x_max - x_min) / 2.0f);
    float y_norm = (y_raw - y_center) / ((y_max - y_min) / 2.0f);

    // Clamp normalized output
    if (x_norm >  1.0f) x_norm = 1.0f;
    if (x_norm < -1.0f) x_norm = -1.0f;
    if (y_norm >  1.0f) y_norm = 1.0f;
    if (y_norm < -1.0f) y_norm = -1.0f;

    // Low-pass filter
    //x_filtered = x_filtered * (1.0f - alpha) + x_norm * alpha;
    //y_filtered = y_filtered * (1.0f - alpha) + y_norm * alpha;

    // Deadzone
    //if (fabsf(x_filtered) < deadzone) x_filtered = 0.0f;
    //if (fabsf(y_filtered) < deadzone) y_filtered = 0.0f;

    *x_out = x_norm;
    *y_out = y_norm;
}

bool button_read(){
    return (gpio_get(joystickbutton) == 0);
}