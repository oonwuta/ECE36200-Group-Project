
#include <display.h>
#include <stdlib.h>
#include "hub75.h"
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "highscore.h"

uint8_t cursor = 0;
uint8_t ycursor = 0;
uint32_t f_xpos = 16;
uint32_t f_ypos = 16;
uint8_t sdir = 0;
uint16_t score = 0;
uint32_t d_xpos = 0;
uint32_t d_ypos = 0;
uint8_t char1 = 0;
uint8_t char2 = 0;
uint8_t char3 = 0;

Grid ***game_board;

bool letters[36][15] = {{0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},  // A
                        {1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0},  // B
                        {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1},  // C
                        {1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0},  // D
                        {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1},  // E
                        {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0},  // F
                        {0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1},  // G
                        {1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},  // H
                        {1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},  // I
                        {0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0},  // J
                        {1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1},  // K
                        {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1},  // L
                        {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},  // M
                        {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},  // N
                        {0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0},  // O
                        {1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0},  // P
                        {0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1},  // Q
                        {1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1},  // R
                        {0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0},  // S
                        {1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0},  // T
                        {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0},  // U
                        {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0},  // V
                        {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1},  // W
                        {1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1},  // X
                        {1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0},  // Y
                        {1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1},  // Z
                        {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1},  // 0
                        {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},  // 1
                        {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1},  // 2
                        {1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0},  // 3
                        {1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1},  // 4
                        {1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0},  // 5
                        {0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0},  // 6
                        {1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0},  // 7
                        {0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0},  // 8
                        {0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1}}; // 9

int start_display(int y)                                                                                                   // used huge aray to make display readable, is also possible to switch this to writing each pixel but since multiple people will be reading this, this is easier to visualize
{                                                                                                                          // 1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
    int buff[32][32] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 1
                        {0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 2
                        {0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 3
                        {0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 4
                        {0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 5
                        {0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 6
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 7
                        {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},  // 8
                        {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 9
                        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},  // 10
                        {0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},  // 11
                        {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 12
                        {0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},  // 13
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 14
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 15
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 16
                        {0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 17
                        {0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 18
                        {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 19
                        {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 20
                        {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 21
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 22
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 23
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 24
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 25
                        {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 26
                        {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 27
                        {0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 28
                        {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 29
                        {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 30
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 31
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}; // 32

    score = 0;
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (buff[i][j] == 1)
            {
                display_set_pixel(i, j, 1, 1, 1); // white pixel
            }
        }
    }

    cursor = (cursor == 0 && y == -1) ? 1 : (cursor == 1 && y == 1) ? 0
                                                                    : cursor; // move cursor up and down
    if (cursor == 0)
    {
        // clear red box around score
        for (int i = 2; i <= 24; i++)
        {
            display_set_pixel(23, i, 0, 0, 0);
            display_set_pixel(31, i, 0, 0, 0);
        }
        for (int i = 23; i <= 31; i++)
        {
            display_set_pixel(i, 2, 0, 0, 0);
            display_set_pixel(i, 24, 0, 0, 0);
        }

        for (int i = 2; i <= 24; i++)
        {
            display_set_pixel(14, i, 1, 0, 0); // draw red line next to Start Game
            display_set_pixel(22, i, 1, 0, 0);
        }
        for (int i = 14; i <= 22; i++)
        {
            display_set_pixel(i, 2, 1, 0, 0);
            display_set_pixel(i, 24, 1, 0, 0);
        }
        display_refresh();

        // wait_ms(50); //flashing red border I ned to figure out how long this needs to propagate
        //  for(int i = 2; i <= 24; i++)
        //  {
        //      display_set_pixel(15, i, 0, 0, 0); //draw red line next to Start Game
        //      display_set_pixel(23,  i, 0, 0, 0);
        //  }
        //  for(int i = 15; i <= 23; i++)
        //  {
        //      display_set_pixel(i, 2, 0, 0, 0);
        //      display_set_pixel(i, 24, 0, 0, 0);
        //  }

        // display_refresh();
    }
    else
    {

        // clear red box around start game
        for (int i = 2; i <= 24; i++)
        {
            display_set_pixel(14, i, 0, 0, 0); // draw red line next to Start Game
            display_set_pixel(22, i, 0, 0, 0);
        }
        for (int i = 14; i <= 22; i++)
        {
            display_set_pixel(i, 2, 0, 0, 0);
            display_set_pixel(i, 24, 0, 0, 0);
        }

        for (int i = 2; i <= 24; i++)
        {
            display_set_pixel(23, i, 1, 0, 0); // draw red line next to High Scores
            display_set_pixel(31, i, 1, 0, 0);
        }
        for (int i = 23; i <= 31; i++)
        {
            display_set_pixel(i, 2, 1, 0, 0);
            display_set_pixel(i, 24, 1, 0, 0);
        }

        display_refresh();
        // //wait_ms(50); //flashing red border I ned to figure out how long this needs to propagate
        // for(int i = 2; i <= 24; i++)
        // {
        //     display_set_pixel(23, i, 0, 0, 0); //draw red line next to High Scores
        //     display_set_pixel(31,  i, 0, 0, 0);
        // }
        // for(int i = 23; i <= 31; i++)
        // {
        //     display_set_pixel(i, 2, 0, 0, 0);
        //     display_set_pixel(i, 24, 0, 0, 0);
        // }
    }
    return cursor;
}

static void push(snake *head, uint32_t xpos, uint32_t ypos) // exchanging the new head with whats in front of it
{
    snake *top = calloc(sizeof(*top), 1);
    top->xpos = xpos;
    top->ypos = ypos;
    top->next = head->next;
    ;
    head->next = top;
    top->prev = head;
}

static void game_loop_update_screen_contents()
{
    for (int i = 0; i < 30; i++)
    {
        for (int j = 0; j < 30; j++)
        {
            Grid *grid = game_board[i][j];
            if (grid->isFood)
            {
                display_set_pixel(i, j, 1, 0, 0);
            }
            else if (grid->snake_segment != NULL || grid->isEdge)
            {
                display_set_pixel(i, j, 1, 1, 1);
            }
            else
            {
                display_set_pixel(i, j, 0, 0, 0);
            }
        }
    }
}

snake *init_snake_game(void)
{
    printf("init snake game\n");

    game_board = malloc(sizeof(Grid **) * 32);
    for (int i = 0; i < 32; i++)
    {
        game_board[i] = malloc(sizeof(Grid*) * 32);
        for (int j = 0; j < 32; j++)
        {
            game_board[i][j] = malloc(sizeof(Grid));
            Grid new_grid = {
                .x = i,
                .y = j,
                .isFood = false,
                .isEdge = false,
                .isSnake = false,
                .snake_segment = NULL
            };

            if (i == 1 || i == 31 || j == 1 || j == 31)
            {
                new_grid.isEdge = true;
            }

            game_board[i][j] = &new_grid;
        }
    }

    snake *head = malloc(sizeof(snake)); // no "if calloc == NULL" logic but also atp if that happenes the entire game breaks
    head->xpos = 16;
    head->ypos = 16;
    head->next = NULL;
    game_board[16][16]->snake_segment = head;

    snake *second = malloc(sizeof(snake));
    second->xpos = 15;
    second->ypos = 16;
    second->next = head;
    head->prev = second;
    game_board[15][16]->snake_segment = second;

    snake *third = malloc(sizeof(snake));
    third->xpos = 14;
    third->ypos = 16;
    third->next = second;
    second->prev = third;
    game_board[14][16]->snake_segment = third;

    snake *tail = malloc(sizeof(snake));
    tail->xpos = 13;
    tail->ypos = 16;
    tail->next = third;
    third->prev = tail;
    tail->prev = NULL;
    game_board[13][16]->snake_segment = tail;

    f_xpos = (rand() % 30 - 1 + 1) + 1; // this number gen not right lol
    f_ypos = (rand() % 30 - 1 + 1) + 1;
    printf("putting food at: %d, %d\n", f_xpos, f_ypos);
    if (game_board[f_xpos][f_ypos]->snake_segment != NULL) // just in case
    {
        f_xpos = 23;
        f_ypos = 23;
    }
    game_board[f_xpos][f_ypos]->isFood = true;
    return head;
}

bool game_loop(int xdir, int ydir, snake *head)
{
    uint32_t prevxpos = head->xpos;
    uint32_t prevypos = head->ypos;





    //Progress note:
    //Added game board array, now need to update movement logic and checking for game over






    if (sdir == 0)
    {
        if (xdir == 1)
        {
            head->xpos += 1;
            sdir = 1;
        }
        else if (xdir == -1)
        {
            head->xpos -= 1;
            sdir = 3;
        }
        else if (ydir == 1)
        {
            head->ypos += 1;
            sdir = 2;
        }
        else if (ydir == -1)
        {
            head->ypos -= 1;
            sdir = 4;
        }
    }
    else if (sdir == 1) // moving right
    {
        if (xdir == -1)
        {
            head->xpos -= 1;
            sdir = 3;
        }
        else if (ydir == 1)
        {
            head->ypos += 1;
            sdir = 2;
        }
        else if (ydir == -1)
        {
            head->ypos -= 1;
            sdir = 4;
        }
        else
        {
            head->xpos += 1;
        }
    }
    else if (sdir == 2) // moving down
    {
        if (xdir == 1)
        {
            head->xpos += 1;
            sdir = 1;
        }
        else if (xdir == -1)
        {
            head->xpos -= 1;
            sdir = 3;
        }
        else if (ydir == -1)
        {
            head->ypos -= 1;
            sdir = 4;
        }
        else
        {
            head->ypos += 1;
        }
    }
    else if (sdir == 3) // moving left
    {
        if (xdir == 1)
        {
            head->xpos += 1;
            sdir = 1;
        }
        else if (ydir == 1)
        {
            head->ypos += 1;
            sdir = 2;
        }
        else if (ydir == -1)
        {
            head->ypos -= 1;
            sdir = 4;
        }
        else
        {
            head->xpos -= 1;
        }
    }
    else if (sdir == 4) // moving up
    {
        if (xdir == 1)
        {
            head->xpos += 1;
            sdir = 1;
        }
        else if (xdir == -1)
        {
            head->xpos -= 1;
            sdir = 3;
        }
        else if (ydir == 1)
        {
            head->ypos += 1;
            sdir = 2;
        }
        else
        {
            head->ypos -= 1;
        }
    }
    if (head->xpos == 0 || head->xpos == 31 || head->ypos == 0 || head->ypos == 31) // if updating to invalid position
    {
        return true; // player died
    }
    display_set_pixel(head->xpos, head->ypos, 1, 1, 1); // since not updating to invalid position draw new head
    // note that we do not need to check if it is colliding with food because the food position is now a snake piece giving the illusion of moving forward
    if (head->xpos == f_xpos && head->ypos == f_ypos) // ate food
    {
        push(head, prevxpos, prevypos); // add new segment, insert old head position as new segment right behind updated position

        f_xpos = (rand() % 31 - 1 + 1) + 1;
        f_ypos = (rand() % 31 - 1 + 1) + 1;
        display_set_pixel(f_xpos, f_ypos, 0, 1, 0); // draw new food
    }
    else
    {
        // move snake
        if (head->next == NULL)
        {
            display_set_pixel(prevxpos, prevypos, 0, 0, 0); // clear trail and exit | could be source of visual issues
            return false;
        }
        snake *current = head->next; // take the segment after head snake will look like X _ Y... at this point
        uint32_t temp_x, temp_y, next_x, next_y;
        temp_x = current->xpos;   // temp x is copying the next segments x  X _ [Y] ... <- this segment copied into temp
        temp_y = current->ypos;   // next segments y
        current->xpos = prevxpos; // update next segment to be old head's xpos X [Y] _  ... moving y to be here (old pos is now _ since curr moved, remember that value is saved by temp)
        current->ypos = prevypos; // next segment is old y
        prevxpos = temp_x;        // updating values to point at Y's tail X Y [_] ?... (? is to represent next segment since we dont know what that is)
        prevypos = temp_y;
        // keep in mind visually the led matrix still looks like X1 X2 Y ?... where X1 is our new head pixel X2 is the old head pixel
        // on the other hand our struct says X1 Y [?]... with prevxpos and prevypos being Y's pos
        if (current->next == NULL)
        {
            display_set_pixel(prevxpos, prevypos, 0, 0, 0);
            return false;
        }
        current = current->next; // current now points at ? so it looks like X Y _ [?] ...
        // we know this is not null so now it looks like this X Y _ [Z]... where Z is the current
        while (current != NULL)
        {
            next_x = current->xpos;
            next_y = current->ypos;
            current->xpos = prevxpos;
            current->ypos = prevypos;
            prevxpos = next_x;
            prevypos = next_y;
            if (current->xpos == head->xpos && current->ypos == head->ypos) // if after moving something is colliding with head you lost
            {
                return true; // player died
            }
            if (current->next == NULL)
            {
                display_set_pixel(prevxpos, prevypos, 0, 0, 0);
            }
            current = current->next;
        }
    }
    // game code here
    // return true if player died

    return false;
}

// We are going to require external instructions including one to tlell the user to press any button to get out of highcore state due to not being able to display everything
void highscore_display() // int prev_screen_state
{
    // this isnt used but
    int buff[32][32] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 1
                        {0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0},  // 2
                        {0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0},  // 3
                        {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0},  // 4
                        {0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0},  // 5
                        {0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0},  // 6
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 7
                        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  // 8
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 9
                        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 10
                        {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 11
                        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 12
                        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 13
                        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 14
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 15
                        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 16
                        {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 17
                        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 18
                        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 19
                        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 20
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 21
                        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 22
                        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 23
                        {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 24
                        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 25
                        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 26
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 27
                        {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 28
                        {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 29
                        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 30
                        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 31
                        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}; // 32

    // if (prev_screen_state == 2)
    // {
    //     return;
    // }

    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (buff[i][j] == 1)
            {
                display_set_pixel(i, j, 255, 255, 255); // white pixel
            }
        }
    }

    uint32_t *scores;
    scores = malloc(5 * sizeof(uint32_t));
    int check = 0; // load_highscores(scores); //should have the upper 15 bits store the name and the lower 17-10 bits store the actual score _____
    while (check == -1)
    {
        check = 0; // load_highscores(scores); //should have some timeout nature
    }
    scores[0] = 0;
    scores[1] = 0;
    scores[2] = 0;
    scores[3] = 0;
    scores[4] = 0;
    int xpos = 9;
    int ypos = 7;
    for (int i = 0; i < 4; i++) // for each high
    {
        int l1 = scores[i] >> 27;                 // first letter
        int l2 = (scores[i] >> 22) & 0x1F;        // second letter
        int l3 = (scores[i] >> 17) & 0x1F;        // third letter
        int d3 = (scores[i]) & 0x0001FFFF;        // buffer for score
        int d1 = d3 / 100 + 26;                   // first digit
        int d2 = (d3 / 10) % 10 + 26;             // second digit
        d3 = d3 % 10 + 26;                        // reusing d3 for third digit
        int l_deco[6] = {l1, l2, l3, d1, d2, d3}; // put into its own array for iteration
        ypos = 7;
        for (int j = 0; j < 6; j++) // for each letter {XXX ###}
        {
            int count = 0;
            for (int l = 0; l < 5; l++) // for each row in specified letter/number
            {
                for (int k = 0; k < 3; k++) // for each column in specified letter/number
                {
                    if (letters[l_deco[j]][count] == 1)                 // l_deco corresponds to the spesific letter/number and count is the pixel to turn on/off
                        display_set_pixel(xpos + l, ypos + k, 1, 1, 1); // white pixel
                    else
                        display_set_pixel(xpos + l, ypos + k, 0, 0, 0); // black pixel
                    count++;
                }
            }
            ypos += 4; // move y position for next high score
        }
        xpos += 6; // move x position for next letter/number
    }
}

uint32_t death_screen_display(int x, int y)
// i can either pass in the score or keep it as a global variable in this function
// I dont know if every score is being updated to the EEprom but if not then I can pull the
// top 5 scores, sort, and compare it with all of them which would not take much time
{
    int buff[32][32] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 1
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 2
                        {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},  // 3
                        {0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 4
                        {0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 5
                        {0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 6
                        {0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 7
                        {1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 8
                        {0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},  // 9
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 10
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0},  // 11
                        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},  // 12
                        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},  // 13
                        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},  // 14
                        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},  // 15
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0},  // 16
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 17
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 18
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 19
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 20
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 21
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 22
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 23
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 24
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 25
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 26
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 27
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 28
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 29
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 30
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 31
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}; // 32
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (buff[i][j] == 1)
            {
                display_set_pixel(j, i, 1, 1, 1); // white pixel
            }
        }
    }

    cursor = cursor == 0 && x == 1 ? 1 : (cursor == 1 && x == 1 ? 2 : (cursor == 1 && x == -1 ? 0 : (cursor == 2 && x == -1 ? 1 : cursor))); // move cursor up and down
    ycursor = y ? (ycursor + 1) % 36 : ycursor;
    if (cursor == 0)
    {
        char1 = ycursor;
        int count = 0;
        for (int k = 0; k < 5; k++) // for each row in specified letter/number
        {
            for (int l = 0; l < 3; l++) // for each column in specified letter/number
            {
                if (letters[ycursor][count] == 1)              // l_deco corresponds to the spesific letter/number and count is the pixel to turn on/off
                    display_set_pixel(6 + l, 20 + k, 1, 1, 1); // white pixel
                else
                    display_set_pixel(6 + l, 20 + k, 0, 0, 0); // black pixel
                count++;
            }
        }
        for (int i = 5; i <= 9; i++)
        {
            display_set_pixel(i, 19, 1, 0, 0); // draw red line next to Start Game
            display_set_pixel(i, 25, 1, 0, 0);
        }
        for (int i = 19; i <= 25; i++)
        {
            display_set_pixel(5, i, 1, 0, 0);
            display_set_pixel(9, i, 1, 0, 0);
        }

        display_refresh();
        // wait_ms(50); //flashing red border I ned to figure out how long this needs to propagate
        for (int i = 5; i <= 9; i++)
        {
            display_set_pixel(i, 19, 0, 0, 0); // draw red line next to Start Game
            display_set_pixel(i, 25, 0, 0, 0);
        }
        for (int i = 19; i <= 25; i++)
        {
            display_set_pixel(5, i, 0, 0, 0);
            display_set_pixel(9, i, 0, 0, 0);
        }
    }
    else if (cursor == 1)
    {
        char2 = ycursor;
        int count = 0;
        for (int k = 0; k < 5; k++) // for each row in specified letter/number
        {
            for (int l = 0; l < 3; l++) // for each column in specified letter/number
            {
                if (letters[ycursor][count] == 1)               // l_deco corresponds to the spesific letter/number and count is the pixel to turn on/off
                    display_set_pixel(12 + l, 20 + k, 1, 1, 1); // white pixel
                else
                    display_set_pixel(12 + l, 20 + k, 0, 0, 0); // black pixel
                count++;
            }
        }
        for (int i = 11; i <= 15; i++)
        {
            display_set_pixel(i, 19, 1, 0, 0); // draw red line next to Start Game
            display_set_pixel(i, 25, 1, 0, 0);
        }
        for (int i = 19; i <= 25; i++)
        {
            display_set_pixel(15, i, 1, 0, 0);
            display_set_pixel(11, i, 1, 0, 0);
        }

        display_refresh();
        // wait_ms(50); //flashing red border I ned to figure out how long this needs to propagate
        for (int i = 11; i <= 15; i++)
        {
            display_set_pixel(i, 19, 0, 0, 0); // draw red line next to Start Game
            display_set_pixel(i, 25, 0, 0, 0);
        }
        for (int i = 19; i <= 25; i++)
        {
            display_set_pixel(15, i, 0, 0, 0);
            display_set_pixel(11, i, 0, 0, 0);
        }
    }
    else
    {
        char3 = ycursor;
        int count = 0;
        for (int k = 0; k < 5; k++) // for each row in specified letter/number
        {
            for (int l = 0; l < 3; l++) // for each column in specified letter/number
            {
                if (letters[ycursor][count] == 1)               // l_deco corresponds to the spesific letter/number and count is the pixel to turn on/off
                    display_set_pixel(18 + l, 20 + k, 1, 1, 1); // white pixel
                else
                    display_set_pixel(18 + l, 20 + k, 0, 0, 0); // black pixel
                count++;
            }
        }
        for (int i = 17; i <= 21; i++)
        {
            display_set_pixel(i, 19, 1, 0, 0); // draw red line next to Start Game
            display_set_pixel(i, 25, 1, 0, 0);
        }
        for (int i = 19; i <= 25; i++)
        {
            display_set_pixel(17, i, 1, 0, 0);
            display_set_pixel(21, i, 1, 0, 0);
        }

        display_refresh();
        // wait_ms(50); //flashing red border I ned to figure out how long this needs to propagate
        for (int i = 17; i <= 21; i++)
        {
            display_set_pixel(i, 19, 0, 0, 0); // draw red line next to Start Game
            display_set_pixel(i, 25, 0, 0, 0);
        }
        for (int i = 19; i <= 25; i++)
        {
            display_set_pixel(17, i, 0, 0, 0);
            display_set_pixel(21, i, 0, 0, 0);
        }
    }

    return (char1 << 31) | (char2 << 26) || (char3 << 21) || ((uint32_t)score);
}

// pin constants
const int SPI_LED_SCK = -1; // replace with the SCK pin number for the LED display
const int SPI_LED_CSn = -1; // replace with the CSn pin number for the LED display
const int SPI_LED_TX = -1;  // replace with the TX pin number for the LED display

// temporarily here until I know which pins to use
/*const int PIN_R1 = 9;       //These next few are self explanatory
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
}*/