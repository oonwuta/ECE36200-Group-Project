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
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hub75.h"

static void kill_snake(snake *head)
{
    snake *next = head->next;
    snake *prev;
    while(next != NULL)
    {
        prev = head;
        head = next;
        next = head->next;
        free(prev);
    }
    free(head);   
}

double x_thresh = 3.0; //do not currently know what these values will be thresholds needs to be greatenough where diagonal inputs do not generate input on both x and y
double y_thresh = 3.0;


/*int main() {
    stdio_init_all();
    sleep_ms(1000);
    
    display_init();
    
    int x = 0, y = 0;
    int dx = 1, dy = 1;
    
    for (int i = 0; i < 500; i++) {
        display_clear();
        display_pixel_white(x, y);
        
        x += dx;
        y += dy;
        
        if (x <= 0 || x >= PANEL_WIDTH - 1) dx = -dx;
        if (y <= 0 || y >= PANEL_HEIGHT - 1) dy = -dy;
        
        for (int j = 0; j < 10; j++) {
            display_refresh();
        }
    }

    return 0;
}*/


int main()
{
    stdio_init_all();
    //sleep_ms(1000);
    joystick_init(500); //whatever decided interval
    float x = 0;
    float y = 0;
    bool button_pressed = false;
    int screen_state = 0; //0 is start screen, 1 is game, 2 is high score
    display_init();
    //init_pwm_audio();
    //highscores_init_defaults(); //not yet included | does nto work
    display_clear();
    button_init();
    snake *head = NULL;
    bool dead = false;
    //sleep_ms(1000);
    while(1){
        joystick_get(&x, &y);
        printf("X: %f, Y: %f\n", x, y);
        int xdir = x > x_thresh ? 1 : (x < -x_thresh ? -1 : 0); 
        int ydir = y > y_thresh ? 1 : (y < -y_thresh ? -1 : 0);
        button_pressed = false;
        button_pressed = button_read(); //imagine that this function exists
        uint8_t startgame = 0; //0 not started, 1 just started, 2 in progress, 3 dead | I should actually make a new state but im lazy
        if(screen_state == 0)
        {
            int cursor = start_display(ydir);
            if(button_pressed)
            {
                //some code that accepts x, y and button press to select and returns the next screen state
                screen_state = cursor == 0? 1 : 2; //to be changed to the output of that function
                startgame = 1;
            }
        } else if (screen_state == 1)
        {
            //game code here
            if(startgame == 1)
            {
                head = init_snake_game();
                startgame += 1; //move to in progress
            }
            dead = game_loop(xdir, ydir, head); //function that runs the game and returns true if player died
            if(dead)
            {
                if(startgame == 2)
                {
                    
                    kill_snake(head); //free snake memory
                    startgame += 1; //move to dead state
                }
                uint32_t runscore = death_screen_display(xdir, ydir); //function that displays score and if high score
                if(button_pressed)
                {
                    //compare highscore in here and then store here if highscore is to be determined here
                    display_clear();
                    screen_state = 2;
                }
            }

        } else if (screen_state == 2)
        {
            highscore_display();
            //display high scores and wait for button press to go back to start screen
            if(button_pressed)
            {
                display_clear();
                screen_state = 0;
            }
        }
        display_refresh();
    }
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