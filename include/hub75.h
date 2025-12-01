#ifndef HUB75_SIMPLE_H
#define HUB75_SIMPLE_H

#include <stdint.h>
#include <stdbool.h>

// Panel geometry
#define PANEL_WIDTH  32
#define PANEL_HEIGHT 32

// Pin definitions
#define PIN_R1   8
#define PIN_G1   9
#define PIN_B1   10
#define PIN_R2   11
#define PIN_G2   12
#define PIN_B2   13
#define PIN_CLK  14
#define PIN_LAT  15
#define PIN_OE   16
#define PIN_A    17
#define PIN_B    18
#define PIN_C    19
#define PIN_D    20

// Simple RGB color structure
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

// Initialize the display
void display_init(void);

// Set a pixel color (x, y, r, g, b) - r/g/b are 0 or 1
void display_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

// Clear entire display
void display_clear(void);

// Refresh the display (call this regularly, e.g. in your game loop)
void display_refresh(void);

// Helper functions for common colors
static inline void display_pixel_red(int x, int y) { display_set_pixel(x, y, 1, 0, 0); }
static inline void display_pixel_green(int x, int y) { display_set_pixel(x, y, 0, 1, 0); }
static inline void display_pixel_blue(int x, int y) { display_set_pixel(x, y, 0, 0, 1); }
static inline void display_pixel_white(int x, int y) { display_set_pixel(x, y, 1, 1, 1); }
static inline void display_pixel_yellow(int x, int y) { display_set_pixel(x, y, 1, 1, 0); }
static inline void display_pixel_cyan(int x, int y) { display_set_pixel(x, y, 0, 1, 1); }
static inline void display_pixel_magenta(int x, int y) { display_set_pixel(x, y, 1, 0, 1); }
static inline void display_pixel_off(int x, int y) { display_set_pixel(x, y, 0, 0, 0); }

#endif // HUB75_SIMPLE_H