#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/multicore.h"
#include <string.h>
#include <stdbool.h>

typedef struct _snake{
    uint32_t xpos;
    uint32_t ypos;
    struct _snake *next; 
    // Your display state variables here
} snake;

uint32_t death_screen_display(int x, int y); 
snake *init_snake_game(void);
int start_display(int y);
bool game_loop(int xdir, int ydir, snake *head);
void highscore_display(void);
