#include "hub75.h"
#include "pico/stdlib.h"
#include <string.h>

// Framebuffer: stores RGB values for each pixel
static rgb_t framebuffer[PANEL_HEIGHT][PANEL_WIDTH];

// Initialize all pins
void display_init(void) {
    // Initialize data pins
    gpio_init(PIN_R1); gpio_set_dir(PIN_R1, GPIO_OUT);
    gpio_init(PIN_G1); gpio_set_dir(PIN_G1, GPIO_OUT);
    gpio_init(PIN_B1); gpio_set_dir(PIN_B1, GPIO_OUT);
    gpio_init(PIN_R2); gpio_set_dir(PIN_R2, GPIO_OUT);
    gpio_init(PIN_G2); gpio_set_dir(PIN_G2, GPIO_OUT);
    gpio_init(PIN_B2); gpio_set_dir(PIN_B2, GPIO_OUT);
    
    // Initialize control pins
    gpio_init(PIN_CLK); gpio_set_dir(PIN_CLK, GPIO_OUT);
    gpio_init(PIN_LAT); gpio_set_dir(PIN_LAT, GPIO_OUT);
    gpio_init(PIN_OE);  gpio_set_dir(PIN_OE, GPIO_OUT);
    
    // Initialize address pins
    gpio_init(PIN_A); gpio_set_dir(PIN_A, GPIO_OUT);
    gpio_init(PIN_B); gpio_set_dir(PIN_B, GPIO_OUT);
    gpio_init(PIN_C); gpio_set_dir(PIN_C, GPIO_OUT);
    gpio_init(PIN_D); gpio_set_dir(PIN_D, GPIO_OUT);
    
    // Set initial states
    gpio_put(PIN_CLK, 0);
    gpio_put(PIN_LAT, 0);
    gpio_put(PIN_OE, 1);  // Output disabled initially
    gpio_put(PIN_A, 0);
    gpio_put(PIN_B, 0);
    gpio_put(PIN_C, 0);
    gpio_put(PIN_D, 0);
    
    // display_set_pixel(0,0,1,1,1);
    // sleep_ms(100); 

    // Clear framebuffer
    display_clear();
}

// Set a pixel in the framebuffer
void display_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= PANEL_WIDTH || y < 0 || y >= PANEL_HEIGHT) return;
    
    framebuffer[y][x].r = r ? 1 : 0;
    framebuffer[y][x].g = g ? 1 : 0;
    framebuffer[y][x].b = b ? 1 : 0;
}

// Clear the framebuffer
void display_clear(void) {
    memset(framebuffer, 0, sizeof(framebuffer));
}

// Shift out one row of data
static void shift_row_data(int row) {
    // Disable output while shifting
    gpio_put(PIN_OE, 1);
    
    // Calculate which rows to display (1:16 scan = top and bottom half)
    int top_row = row;
    int bottom_row = row + 16;
    
    // Shift in 32 columns of data
    for (int col = 0; col < PANEL_WIDTH; col++) {
        // Get pixel data for top and bottom halves
        rgb_t top = framebuffer[top_row][col];
        rgb_t bottom = framebuffer[bottom_row][col];
        
        // Set data pins
        gpio_put(PIN_R1, top.r);
        gpio_put(PIN_G1, top.g);
        gpio_put(PIN_B1, top.b);
        gpio_put(PIN_R2, bottom.r);
        gpio_put(PIN_G2, bottom.g);
        gpio_put(PIN_B2, bottom.b);
        
        // Clock pulse
        sleep_us(1);
        gpio_put(PIN_CLK, 1);
        sleep_us(1);
        gpio_put(PIN_CLK, 0);
    }
    
    // Latch the data
    gpio_put(PIN_LAT, 1);
    sleep_us(1);
    gpio_put(PIN_LAT, 0);
    
    // Set row address
    gpio_put(PIN_A, (row >> 0) & 1);
    gpio_put(PIN_B, (row >> 1) & 1);
    gpio_put(PIN_C, (row >> 2) & 1);
    gpio_put(PIN_D, (row >> 3) & 1);
    
    // Enable output
    gpio_put(PIN_OE, 0);
}

// Refresh the display - scan through all 16 row addresses
void display_refresh(void) {
    for (int row = 0; row < 16; row++) {
        shift_row_data(row);
        sleep_us(200);  // Display time per row (adjust for brightness)
    }
}