#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "music.h"
#include "joystick.h"
#include "display.h"
#include "highscore.h"
#include <time.h>
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hub75.h"

#define COUNTER_TOP 32

static void kill_snake(snake *head)
{
    snake *next = head->next;
    snake *prev;
    while (next != NULL)
    {
        prev = head;
        head = next;
        next = head->next;
        free(prev);
    }
    free(head);
}

double x_thresh = 0.5; // do not currently know what these values will be thresholds needs to be greatenough where diagonal inputs do not generate input on both x and y
double y_thresh = 0.5;

void test_start()
{
    sleep_ms(1000);

    display_init();

    int x = 0, y = 0;
    int dx = 1, dy = 1;

    for (int i = 0; i < 50; i++)
    {
        display_clear();
        display_pixel_white(x, y);

        x += dx;
        y += dy;

        if (x <= 0 || x >= PANEL_WIDTH - 1)
            dx = -dx;
        if (y <= 0 || y >= PANEL_HEIGHT - 1)
            dy = -dy;

        for (int j = 0; j < 10; j++)
        {
            display_refresh();
        }
    }

    return;
}

static void loop_until_button_switch(bool start_state)
{
    bool button_pressed;
    while (button_pressed == start_state)
    {
        sleep_ms(1);
        button_pressed = button_read();
    }
    return;
}

int main()
{
    stdio_init_all();
    test_start();
    play_song(120, canon_in_d, 0);
    sleep_ms(1000);
    joystick_init(); // whatever decided interval
    float x = 0;
    float y = 0;
    float v = 0;
    bool button_pressed = false;
    int screen_state = 0; // 0 is start screen, 1 is game, 2 is high score
    int game_timer = 0;
    // int prev_screen_state = 0;
    display_init();
    init_pwm_audio();
    //highscores_init_defaults(); //not yet included | does nto work
    button_init();
    snake *head = NULL;
    bool dead = false;
    uint8_t startgame = 0; // 0 not started, 1 just started, 2 in progress, 3 dead | I should actually make a new state but im lazy
    hs_entry_t *hs_entry;
    highscore_eeprom_t hsstruct = {.i2c_addr = I2C_ADDR_DEFAULT, .i2c_port = i2c0};
    hs_entry_t *scores;
    scores = malloc(4 * sizeof(hs_entry_t));
    eeprom_init(i2c0, I2C_ADDR_DEFAULT, &hsstruct);
    bool hs_entered = false;
    bool hs_loaded = false;
    srand(time(NULL));
    int cursor = 0;
    // sleep_ms(1000);

    //WIPE EEPROM TEST////////////////////////////////////
    // hs_entry_t wipe_out[4] = 
    // {
    //     {
    //         .name = 0,
    //         .score = 0
    //     },
    //     {
    //         .name = 0,
    //         .score = 0
    //     },{
    //         .name = 0,
    //         .score = 0
    //     },{
    //         .name = 0,
    //         .score = 0
    //     }
    // };

    // highscores_save(&hsstruct, &wipe_out);
    //////////////////////////////////////////////////////
    
    while (1)
    {
        joystick_read(&x, &y, &v);
        set_master_volume(v);
        // printf("X: %f, Y: %f\n", x, y);
        int xdir = (x > x_thresh && x > y) ? 1 : (x < -x_thresh && x < y ? -1 : 0);
        int ydir = (y > y_thresh && y >= x) ? 1 : (y < -y_thresh && y <= x ? -1 : 0);
        // printf("dir: %d, %d\n", xdir, ydir);
        button_pressed = false;
        button_pressed = button_read(); // imagine that this function exists
        if (screen_state == 0)
        {
            cursor = start_display(ydir);
            if (button_pressed)
            {
                // loop_until_button_switch(button_pressed);
                //  some code that accepts x, y and button press to select and returns the next screen state
                screen_state = cursor == 0 ? 1 : 2; // to be changed to the output of that function
                startgame = cursor == 0 ? 1 : 0;    // start game if "start" was selected
                display_clear();
                display_refresh();
                button_pressed = false;
            }
        }
        else if (screen_state == 1)
        {
            if (startgame == 1)
            {
                printf("Start Game\n");
                head = init_snake_game();
                startgame += 1; // move to in progress
                dead = false;
                hs_entered = false;
                hs_loaded = false;
            }
            game_timer += 1;
            if (game_timer >= COUNTER_TOP)
            {
                game_timer = 0;
                if (!dead)
                {
                    // game code here
                    dead = game_loop(xdir, ydir, head); // function that runs the game and returns true if player died
                    game_loop_update_screen_contents();

                    if (dead)
                    {
                        play_song(80, death, 0);
                        display_clear();
                    }
                }

                else if (dead)
                {
                    if (startgame == 2)
                    {

                        kill_snake(head); // free snake memory
                        startgame += 1;   // move to dead state
                    }

                    if (!hs_entered)
                    {
                        hs_entry = death_screen_display(xdir, ydir); // function that displays score and if high score
                    }

                    if (button_pressed && !hs_entered)
                    {
                        // compare highscore in here and then store here if highscore is to be determined here

                        int success = highscores_load(&hsstruct, scores);

                        if (success == -1) // invalid read from eeprom
                        {
                            for (int i = 0; i < 4; i++)
                            {
                                scores[i].name[0] = 0;
                                scores[i].name[1] = 0;
                                scores[i].name[2] = 0;
                                scores[i].score = 0;
                            }
                        }

                        int position = 4;
                        while (position > 0 && hs_entry->score > scores[position-1].score)
                        {
                            position -= 1;
                        }

                        //SWAP DOWNWARD
                        int temp_pos = 3;
                        while (temp_pos > position)
                        {
                            scores[temp_pos].score = scores[temp_pos-1].score;
                            scores[temp_pos].name[0] = scores[temp_pos-1].name[0];
                            scores[temp_pos].name[1] = scores[temp_pos-1].name[1];
                            scores[temp_pos].name[2] = scores[temp_pos-1].name[2];
                            temp_pos-=1;
                        }

                        scores[position].name[0] = hs_entry->name[0];
                        scores[position].name[1] = hs_entry->name[1];
                        scores[position].name[2] = hs_entry->name[2];
                        scores[position].score = hs_entry->score;

                        highscores_save(&hsstruct, scores);
                        hs_entered = true;

                        screen_state = 2;
                        button_pressed = false;
                    }
                }
            }

        }
        else if (screen_state == 2)
        {

            // display high scores and wait for button press to go back to start screen
            if (button_pressed)
            {
                // loop_until_button_switch(button_pressed);
                display_clear();
                screen_state = 0;
                startgame = 0;
                
                dead = false;
                hs_entered = false;
                hs_loaded = false;
                button_pressed = false;
            }
            else
            {
                display_clear();
                highscore_display(&hsstruct, scores, &hs_loaded);
            }
        }
        display_refresh();
    }

    free(scores);
}
/*

psuedocode draft for main is
main: //kind of a state machine
    game start, init joystick,
    while(1):
        read joystick
        if in start screen:
            update cursor position based on joystick
            if select pressed on start:
                start
            else if highscore pressed:
                highscore
        else if in game:
            dead = update_snake(joystickx, joysticky) //function will have to know board state and return if player lost or not
        else if dead:
            print score, print if high score //can also just print leaderboard
            wait for button to go back to start screen
*/

//test for eeprom/highscore, running into issues where im stuck loading eeprom (highscores_load)
static void i2c_scan(i2c_inst_t *i2c) {
    printf("I2C scan: ");
    for (int addr = 1; addr < 0x78; addr++) 
    {
        int r = i2c_write_blocking(i2c, (uint8_t)addr, NULL, 0, false);
        if (r >= 0) 
        {
            printf("0x%02X ", addr);
        }
    }
    printf("\n");
}

// int main(void) {
//     stdio_init_all();
//     sleep_ms(1200);

//     printf("\nHighscore test\n");
//     printf("Type a number + Enter to insert; 'p' to print; 'q' to quit.\n\n");

//     //init I2C pins & bus
//     gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
//     gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
//     i2c_init(I2C_PORT_USED, 100000);

//     i2c_scan(I2C_PORT_USED);

//     //setup EEPROM handle (use default address unless you changed it)
//     highscore_eeprom_t e;
//     eeprom_init(I2C_PORT_USED, EEPROM_I2C_ADDR_DEFAULT, &e);

//     //load & print existing highscores (or zeros if read failed)
//     uint32_t scores[HS_COUNT];
//     if (highscores_load(&e, scores) < 0) {
//         printf("Couldn't read highscores; starting with zeros.\n");
//         for (int i = 0; i < HS_COUNT; ++i) scores[i] = 0;
//     }

//     printf("\nCurrent highscores:\n");
//     for (int i = 0; i < HS_COUNT; ++i) {
//         printf(" %2d: %10u\n", i+1, (unsigned)scores[i]);
//     }

//     char line[64];
//     for(;;) 
//     {
//         printf("\n> ");
//         fflush(stdout);

//         if (!fgets(line, sizeof(line), stdin)) 
//         {
//             continue;
//         }
//         // strip newline
//         char *nl = strchr(line, '\n');
//         if (nl) 
//         {
//             *nl = '\0';
//         }

//         if (line[0] == '\0') 
//         {
//             continue;
//         }

//         if (line[0] == 'q' || line[0] == 'Q') 
//         {
//             printf("Quitting.\n");
//             break;
//         }

//         if (line[0] == 'p' || line[0] == 'P') 
//         {
//             if (highscores_load(&e, scores) == 0) 
//             {
//                 printf("\nHighscores:\n");
//                 for (int i = 0; i < HS_COUNT; ++i) printf(" %2d: %10u\n", i+1, (unsigned)scores[i]);
//             } 
//             else 
//             {
//                 printf("Error reading highscores.\n");
//             }
//             continue;
//         }

//         long v = atoi(line);
//         if (v < 0) 
//         {
//             printf("Negative values not supported.\n");
//             continue;
//         }
//         uint32_t val = (uint32_t)v;

//         int res = highscores_insert_and_save(&e, val);
//         if (res < 0) 
//         {
//             printf("I2C/write error while inserting.\n");
//         } 
//         else if (res == 0) 
//         {
//             printf("Not high enough to enter top %d.\n", HS_COUNT);
//         } 
//         else 
//         {
//             highscores_load(&e, scores);
//             printf("Inserted %u — updated highscores:\n", val);
//             for (int i = 0; i < HS_COUNT; i++) 
//             {
//                 printf(" %2d: %10u\n", i+1, (unsigned)scores[i]);
//             }
//             printf("\nPower-cycle to verify persistence.\n");
//         }
//     }

//     return 0;
// }
