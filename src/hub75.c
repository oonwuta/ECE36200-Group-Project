// hub75.c - Fixed DMA interrupt handling
#include "hub75.h"
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hub75.pio.h"

#define PIO_INSTANCES pio0

#define ROWS (HUB75_ROWS)        // 32
#define COLUMNS (HUB75_WIDTH)    // 32

typedef uint8_t pixel6_t;

// Framebuffers
static pixel6_t *framebuf[2] = { NULL, NULL };
static int current_front = 0;
static int current_back = 1;

// PIO + DMA resources
static PIO pio = pio0;
static uint sm = 0;
static int dma_chan = -1;

// DMA completion flag
static volatile bool dma_done = false;

// DMA IRQ handler - CRITICAL FIX
static void dma_handler() {
    // Clear interrupt for our specific channel
    dma_hw->ints0 = 1u << dma_chan;
    dma_done = true;
}

// Allocate buffers
static bool alloc_buffers(void) {
    size_t size = ROWS * COLUMNS * sizeof(pixel6_t);
    framebuf[0] = (pixel6_t *)malloc(size);
    framebuf[1] = (pixel6_t *)malloc(size);
    if (!framebuf[0] || !framebuf[1]) {
        if (framebuf[0]) free(framebuf[0]);
        if (framebuf[1]) free(framebuf[1]);
        framebuf[0] = framebuf[1] = NULL;
        return false;
    }
    memset(framebuf[0], 0, size);
    memset(framebuf[1], 0, size);
    return true;
}

// Setup GPIO pins
static void setup_pins(void) {
    // Data pins R1-B2
    for (int p = HUB75_R1; p <= HUB75_B2; ++p) {
        gpio_init(p);
        gpio_set_dir(p, GPIO_OUT);
        gpio_put(p, 0);
    }
    
    // Control pins
    gpio_init(HUB75_CLK); gpio_set_dir(HUB75_CLK, GPIO_OUT); gpio_put(HUB75_CLK, 0);
    gpio_init(HUB75_LAT); gpio_set_dir(HUB75_LAT, GPIO_OUT); gpio_put(HUB75_LAT, 0);
    gpio_init(HUB75_OE);  gpio_set_dir(HUB75_OE,  GPIO_OUT); gpio_put(HUB75_OE, 1);

    // Address pins
    gpio_init(HUB75_A); gpio_set_dir(HUB75_A, GPIO_OUT); gpio_put(HUB75_A, 0);
    gpio_init(HUB75_B); gpio_set_dir(HUB75_B, GPIO_OUT); gpio_put(HUB75_B, 0);
    gpio_init(HUB75_C); gpio_set_dir(HUB75_C, GPIO_OUT); gpio_put(HUB75_C, 0);
    gpio_init(HUB75_D); gpio_set_dir(HUB75_D, GPIO_OUT); gpio_put(HUB75_D, 0);
}

// Build row shift buffer
static void build_row_shiftbuffer(uint32_t *out32, pixel6_t *fb, int row_addr) {
    int top_y = row_addr;
    int bottom_y = row_addr + ROWS;

    for (int x = 0; x < COLUMNS; ++x) {
        pixel6_t top = fb[top_y * COLUMNS + x];
        pixel6_t bot = fb[bottom_y * COLUMNS + x];

        uint32_t v = 0;
        // Top half: bits 0-2
        if (top & 0x1) v |= (1u << 0);  // R1
        if (top & 0x2) v |= (1u << 1);  // G1
        if (top & 0x4) v |= (1u << 2);  // B1
        // Bottom half: bits 3-5
        if (bot & 0x1) v |= (1u << 3);  // R2
        if (bot & 0x2) v |= (1u << 4);  // G2
        if (bot & 0x4) v |= (1u << 5);  // B2

        out32[x] = v;
    }
}

// Pulse latch
static inline void pulse_lat(void) {
    gpio_put(HUB75_LAT, 1);
    asm volatile("nop\n nop\n nop\n");
    gpio_put(HUB75_LAT, 0);
}

// Set row address
static inline void set_row_address(int ra) {
    gpio_put(HUB75_A, (ra >> 0) & 1);
    gpio_put(HUB75_B, (ra >> 1) & 1);
    gpio_put(HUB75_C, (ra >> 2) & 1);
    gpio_put(HUB75_D, (ra >> 3) & 1);
}

// FIXED: Start DMA with proper configuration
static void start_row_dma(uint32_t *src32, int count) {
    dma_done = false;

    // Stop any ongoing transfer
    dma_channel_abort(dma_chan);
    
    // Wait for abort to complete
    while (dma_channel_is_busy(dma_chan)) tight_loop_contents();

    // Configure DMA channel
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));

    // Configure and start transfer
    dma_channel_configure(
        dma_chan,
        &c,
        &pio->txf[sm],  // Write to PIO TX FIFO
        src32,          // Read from buffer
        count,          // Number of transfers
        true            // Start immediately
    );
}

// Send one row
static void send_row_from_front(int row_addr) {
    // Build shift buffer
    uint32_t shiftbuf[COLUMNS];
    build_row_shiftbuffer(shiftbuf, framebuf[current_front], row_addr);

    // Start DMA transfer
    start_row_dma(shiftbuf, COLUMNS);

    // Wait for DMA completion with timeout
    uint32_t timeout = 100000; // Timeout counter
    while (!dma_done && timeout > 0) {
        tight_loop_contents();
        timeout--;
    }
    
    if (timeout == 0) {
        // DMA timed out - manually complete
        dma_channel_abort(dma_chan);
    }

    // Disable output during latch
    gpio_put(HUB75_OE, 1);
    
    // Pulse latch
    pulse_lat();
    
    // Set row address
    set_row_address(row_addr);
    
    // Small delay
    busy_wait_us(1);
    
    // Enable output
    gpio_put(HUB75_OE, 0);
    
    // Display time
    busy_wait_us(300);
    
    // Disable before next row
    gpio_put(HUB75_OE, 1);
}

void hub75_init(void) {
    // Allocate buffers
    if (!alloc_buffers()) {
        panic("Failed to allocate framebuffers");
    }

    // Setup pins
    setup_pins();

    // Claim DMA channel FIRST
    dma_chan = dma_claim_unused_channel(true);

    // Setup DMA interrupt handler
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_enabled(DMA_IRQ_0, true);

    // Initialize PIO
    int offset = pio_add_program(pio, &hub75_program);
    sm = pio_claim_unused_sm(pio, true);
    
    pio_sm_config c = hub75_program_get_default_config(offset);
    
    // Configure OUT pins (R1-B2: 6 pins starting at GPIO 8)
    sm_config_set_out_pins(&c, HUB75_R1, 6);
    
    // Configure side-set (CLK: 1 pin)
    sm_config_set_sideset_pins(&c, HUB75_CLK);
    
    // Set output shift direction (shift right, autopull)
    sm_config_set_out_shift(&c, true, true, 32);
    
    // Set clock divider for reasonable speed
    sm_config_set_clkdiv(&c, 8.0f);
    
    // Initialize PIO pins
    for (int i = 0; i < 6; ++i) {
        pio_gpio_init(pio, HUB75_R1 + i);
    }
    pio_gpio_init(pio, HUB75_CLK);
    
    // Set pin directions
    pio_sm_set_consecutive_pindirs(pio, sm, HUB75_R1, 6, true);
    pio_sm_set_consecutive_pindirs(pio, sm, HUB75_CLK, 1, true);
    
    // Initialize and start state machine
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

void hub75_deinit(void) {
    if (dma_chan >= 0) {
        dma_channel_abort(dma_chan);
        irq_set_enabled(DMA_IRQ_0, false);
        dma_channel_set_irq0_enabled(dma_chan, false);
        dma_channel_unclaim(dma_chan);
        dma_chan = -1;
    }
    if (framebuf[0]) free(framebuf[0]);
    if (framebuf[1]) free(framebuf[1]);
    framebuf[0] = framebuf[1] = NULL;
}

void hub75_clear_backbuffer(void) {
    size_t size = ROWS * COLUMNS * sizeof(pixel6_t);
    memset(framebuf[current_back], 0, size);
}

uint8_t *hub75_backbuffer(void) {
    return (uint8_t *)framebuf[current_back];
}

void hub75_swap_buffers(bool copy) {
    // Swap buffer indices
    int old_front = current_front;
    current_front = current_back;
    current_back = old_front;

    if (copy) {
        size_t size = ROWS * COLUMNS * sizeof(pixel6_t);
        memcpy(framebuf[current_back], framebuf[current_front], size);
    }

    // Refresh all rows once
    for (int r = 0; r < ROWS; ++r) {
        send_row_from_front(r);
    }
}

void hub75_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= COLUMNS || y < 0 || y >= HUB75_HEIGHT) return;
    
    pixel6_t val = 0;
    if (r) val |= 1;
    if (g) val |= 1 << 1;
    if (b) val |= 1 << 2;
    
    framebuf[current_back][y * COLUMNS + x] = val;
}

void hub75_refresh_display(void) {
    // Refresh all rows from front buffer
    for (int r = 0; r < ROWS; ++r) {
        send_row_from_front(r);
    }
}