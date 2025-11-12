
#include <display.h>

//pin constants
const int SPI_LED_SCK = -1; //replace with the SCK pin number for the LED display
const int SPI_LED_CSn = -1; //replace with the CSn pin number for the LED display
const int SPI_LED_TX = -1; //replace with the TX pin number for the LED display



//temporarily here until I know which pins to use
const int PIN_R1 = 9;       //These next few are self explanatory
const int PIN_G1 = 21;
const int PIN_B1 = 10;
const int PIN_R2 = 11; 
const int PIN_G2 = 20;
const int PIN_B2 = 12;      
const int PIN_LRT = 17;     //latch pin
const int PIN_CLK = 15;     //clock pin
const int PIN_OE = 16;      //output enable of panel (active low)
const int PIN_A  = 13;      //row address bit A
const int PIN_B  = 19;      //row address bit B
const int PIN_C  = 14;      //row address bit C
const int PIN_D  = 18;      //row address bit D 

//framebuffers (double buffering for pixel display)
//32x32 pixels for LED Display, each pixel stores 3-bit color: bit2=R, bit1=G, bit0=B
static uint8_t fb0[32][32];
static uint8_t fb1[32][32];
uint8_t (*display_fb)[32] = fb0; //pointer read by refresh core
uint8_t (*draw_fb)[32] = fb1; //pointer used by menu drawing code
bool new_frame_ready = false; //to know when to refresh

//SPI/Shift register transfer
#define ROW_BYTES 24 //192 bits/8 = 24 bytes per row

void init_led_pins_and_spi(void) {
    //set MOSI/SCK to SPI function
    gpio_set_function(SPI_LED_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI_LED_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_LED_CSn, GPIO_FUNC_SPI);

    // init control pins
    gpio_init(PIN_OE);      
    gpio_init(PIN_A);       
    gpio_init(PIN_B);       
    gpio_init(PIN_C);       
    gpio_init(PIN_D);       
    
    gpio_set_dir(PIN_OE, GPIO_OUT);
    gpio_set_dir(PIN_A, GPIO_OUT); 
    gpio_set_dir(PIN_B, GPIO_OUT);  
    gpio_set_dir(PIN_C, GPIO_OUT); 
    gpio_set_dir(PIN_D, GPIO_OUT);

    gpio_put(PIN_OE, 1);      //disable output (active low)

    spi_init(spi0, 20000000); //init SPI peripheral at 20MHz (can change if needed)
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

//send the 24 bytes to chained shift registers and latch them
void send_row_spi(const uint8_t *rowbuf, size_t len_bytes) {
    spi_write_blocking(spi0, rowbuf, len_bytes);
}

//packing rows from framebuffer
//per-column bits order: [R1 G1 B1 R2 G2 B2]
void build_rowbuf_from_framebuffer(int row, uint8_t *out24) {
    memset(out24, 0, ROW_BYTES); //clear the buffer
    int bit_index = 0; //0..191

    //iterate columns from 31 -> 0 so left/right mapping is consistent
    for (int col = 31; col >= 0; col--) 
    {
        uint8_t p1 = display_fb[row][col] & 0x7;      // top half pixel
        uint8_t p2 = display_fb[row + 16][col] & 0x7; // bottom half

        int bits[6];
        bits[0] = (p1 >> 2) & 1; //R1
        bits[1] = (p1 >> 1) & 1; //G1
        bits[2] = (p1 >> 0) & 1; //B1
        bits[3] = (p2 >> 2) & 1; //R2
        bits[4] = (p2 >> 1) & 1; //G2
        bits[5] = (p2 >> 0) & 1; //B2

        for (int b = 0; b < 6; b++) //perbit loop
        {
            int byte_idx = bit_index / 8;
            int bit_in_byte = 7 - (bit_index % 8); //MSB-first
            if (bits[b]) 
            {
                out24[byte_idx] |= (1 << bit_in_byte);
            }
            bit_index++;
        }
    }
}

//refresh core (still looking into this)
void core1_refresh_loop(void) {
    const int rows = 16; //typical for 1/16 scan panels
    uint8_t rowbuf[ROW_BYTES];

    for(;;)
    {
        //swap the buffers when requested
        if (new_frame_ready) 
        {
            uint8_t (*tmp)[32] = display_fb;
            display_fb = draw_fb;
            draw_fb = tmp;
            new_frame_ready = false;
        }

        for (int row = 0; row < rows; row++) 
        {
            build_rowbuf_from_framebuffer(row, rowbuf);
            send_row_spi(rowbuf, ROW_BYTES); //shift + latch

            //set row address if pins provided
            gpio_put(PIN_A, (row >> 0) & 1);
            gpio_put(PIN_B, (row >> 1) & 1);
            gpio_put(PIN_C, (row >> 2) & 1);
            gpio_put(PIN_D, (row >> 3) & 1);

            gpio_put(PIN_OE, 0); //enable output
            busy_wait_us(80);
        }
    }
}

//creating menu fonts
//for now i just have start, but can add more in future
static const uint8_t font3x5_A[] = {0b010,0b101,0b111,0b101,0b101};
static const uint8_t font3x5_R[] = {0b110,0b101,0b110,0b101,0b101};
static const uint8_t font3x5_S[] = {0b111,0b100,0b111,0b001,0b111};
static const uint8_t font3x5_T[] = {0b111,0b010,0b010,0b010,0b010};

static const uint8_t *getFont3x5(char c) {
    switch (c) {
        case 'A': return font3x5_A;
        case 'R': return font3x5_R;
        case 'S': return font3x5_S;
        case 'T': return font3x5_T;
        default:  return NULL;
    }
}

static inline void setPixelDrawFb(int x, int y, uint8_t color) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) //incase it is outisde the bounds
    {
        return;
    }
    draw_fb[y][x] = color & 0x7; // 3-bit color
}

static void fillRect(int x0, int y0, int w, int h, uint8_t color) {
    int x1 = x0 + w;
    int y1 = y0 + h;
    for (int y = y0; y < y1; y++)
    {
        for (int x = x0; x < x1; x++)
        {
            draw_fb[y][x] = color & 0x7;
        }
    }
}

static inline void clearDrawFb(void) {
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 32; x++)
        {
            draw_fb[y][x] = 0;
        }
    }
}

static void drawChar3x5(int x, int y, char c, uint8_t color, bool inv) {
    if (c == ' ') 
    {
        return;
    }
    uint8_t *glyph = getFont3x5(c);
    if (!glyph) 
    {
        return;
    }
    for (int row = 0; row < 5; ++row) 
    {
        uint8_t rowbits = glyph[row];
        for (int col = 0; col < 3; ++col) 
        {
            bool on = (rowbits >> (2 - col)) & 1u;
            int px = x + col;
            int py = y + row;
            if (px < 0 || px >= 32 || py < 0 || py >= 32) 
            {
                continue;
            }
            if (inv) //if need to invert the text (selected)
            {
                if (!on) 
                {
                    draw_fb[py][px] = color;
                }
                else 
                {
                    draw_fb[py][px] = 0;
                }
            } 
            else 
            {
                if (on) 
                {
                    draw_fb[py][px] = color;
                }
            }
        }
    }
}

static void drawText3x5(int x, int y, const char *s, uint8_t color, bool inv) {
    int cursorX = x;
    for (const char *p = s; *p; ++p) 
    {
        if (*p == ' ') 
        { 
            cursorX += 4; continue; 
        }
        drawChar3x5(cursorX, y, *p, color, inv);
        cursorX += 4;
        if (cursorX >= 32) break; //cant go out of bounds
    }
}

//menu api
#define MENU_COLOR 0x2 //green
#define MENU_HIGHLIGHT 0x4 //red

static int menu_selected_index = 0;                         // 0..N-1
static const char *menu_options[] = {"START"};   //menu labels (currently just start, maybe add highscore)
static const int menu_option_count = 1;

void menu_set_selected(int idx) {                           // clamp selection
    if (idx < 0) 
    {
        idx = 0;
    }
    if (idx >= menu_option_count)
    {
        idx = menu_option_count - 1;
    } 
    menu_selected_index = idx;
}

//make a centered option and highlight it
static void drawMenuOption(const char *label, int yTop, bool selected) {
    int len = 0; 
    while (label[len])
    {
        len++;
    }
    int textWidth = len * 4 - 1;
    int xLeft = (32 - textWidth) / 2;
    if (selected) 
    {
        fillRect(0, yTop - 1, 32, 7, MENU_HIGHLIGHT); // background bar
        drawText3x5(xLeft, yTop, label, 0x0, false); // draw text as black (inverted)
    } 
    else 
    {
        drawText3x5(xLeft, yTop, label, MENU_COLOR, false);  // normal text
    }
}

//render menu into draw_fb and request a swap so refresh core displays it
void menu_render_and_swap(void) {
    clearDrawFb();
    drawText3x5(4, 1, "SNAKE", MENU_COLOR, false); //title
    drawMenuOptionCentered(menu_options[0], 12, menu_selected_index == 0);   //first option, add more later

    //maybe add small selector arrow on left

    new_frame_ready = true;  //signal refresh core to swap buffers
}