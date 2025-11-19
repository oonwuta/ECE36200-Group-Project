#ifndef HUB75_H
#define HUB75_H

#include <stdint.h>
#include <stdbool.h>

// Panel geometry (32x32 1:16 scan)
#define HUB75_WIDTH  32
#define HUB75_HEIGHT 32
#define HUB75_ROWS   (HUB75_HEIGHT/2)  // 16 (addresses)

#define HUB75_R1   8
#define HUB75_G1   9
#define HUB75_B1   10
#define HUB75_R2   11
#define HUB75_G2   12
#define HUB75_B2   13

#define HUB75_CLK  14
#define HUB75_LAT  15
#define HUB75_OE   16

#define HUB75_A    17
#define HUB75_B    18
#define HUB75_C    19
#define HUB75_D    20

// API
void hub75_init(void);                    // initialize PIO/DMA, allocate buffers
void hub75_deinit(void);                  // stop everything, free resources
void hub75_clear_backbuffer(void);        // clear back buffer to black
uint8_t *hub75_backbuffer(void);          // pointer to back buffer (raw pixel format)
void hub75_swap_buffers(bool copy);       // present back buffer (copy=true copies front->back after swap)
void hub75_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b); // set pixel in back buffer (r/g/b are 0 or 1)
void hub75_refresh_display(void);

#endif // HUB75_H
