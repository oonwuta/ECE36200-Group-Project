#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "music.h"
#include "joystick.h"

//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hub75.h"

int main() {
    stdio_init_all();
    sleep_ms(2000); // Wait for USB serial to stabilize
    
    printf("Starting HUB75 test...\n");

    // Initialize the HUB75 driver
    hub75_init();
    printf("HUB75 initialized\n");

    // Test 1: Simple single pixel test
    printf("Test 1: Single green pixel at (0,0)\n");
    hub75_clear_backbuffer();
    hub75_draw_pixel(0, 0, 0, 1, 0);  // Top-left corner green
    hub75_swap_buffers(false);
    sleep_ms(2000);

    // Test 2: Four corner pixels
    printf("Test 2: Four corner pixels (red)\n");
    hub75_clear_backbuffer();
    hub75_draw_pixel(0, 0, 1, 0, 0);              // Top-left red
    hub75_draw_pixel(31, 0, 1, 0, 0);             // Top-right red
    hub75_draw_pixel(0, 31, 1, 0, 0);             // Bottom-left red
    hub75_draw_pixel(31, 31, 1, 0, 0);            // Bottom-right red
    hub75_swap_buffers(false);
    sleep_ms(2000);

    // Test 3: Full screen red
    printf("Test 3: Full screen red\n");
    hub75_clear_backbuffer();
    for (int y = 0; y < HUB75_HEIGHT; y++) {
        for (int x = 0; x < HUB75_WIDTH; x++) {
            hub75_draw_pixel(x, y, 1, 0, 0);
        }
    }
    hub75_swap_buffers(false);
    sleep_ms(2000);

    // Test 4: Full screen green
    printf("Test 4: Full screen green\n");
    hub75_clear_backbuffer();
    for (int y = 0; y < HUB75_HEIGHT; y++) {
        for (int x = 0; x < HUB75_WIDTH; x++) {
            hub75_draw_pixel(x, y, 0, 1, 0);
        }
    }
    hub75_swap_buffers(false);
    sleep_ms(2000);

    // Test 5: Full screen blue
    printf("Test 5: Full screen blue\n");
    hub75_clear_backbuffer();
    for (int y = 0; y < HUB75_HEIGHT; y++) {
        for (int x = 0; x < HUB75_WIDTH; x++) {
            hub75_draw_pixel(x, y, 0, 0, 1);
        }
    }
    hub75_swap_buffers(false);
    sleep_ms(2000);

    // Test 6: White (all colors)
    printf("Test 6: Full screen white\n");
    hub75_clear_backbuffer();
    for (int y = 0; y < HUB75_HEIGHT; y++) {
        for (int x = 0; x < HUB75_WIDTH; x++) {
            hub75_draw_pixel(x, y, 1, 1, 1);
        }
    }
    hub75_swap_buffers(false);
    
    printf("Test complete. Display should be white now.\n");
    printf("Entering continuous refresh loop...\n");
    
    hub75_swap_buffers(false);
    // Continuous refresh loop - keep the display alive
    while (true) {
        // Continuously refresh all rows
        hub75_swap_buffers(false);
        sleep_us(200); 
    }

    return 0;
}



/*
int main()
{
    joystick_init();
    uint16_t x = 0;
    uint16_t y = 0;
    bool button_pressed = false;
    int screen_state = 0; //0 is start screen, 1 is game, 2 is high score
    matrix_init();
    init_pwm_audio();
    highscores_init_defaults(); //not yet included
    while(1){
        joystick_read(&x, &y);
        button_read(button_pressed);
        if(screen_state == 0)
        {
            //some code that accepts x, y and button press to select and returns the next screen state
            screen_state = 1; //to be changed to the output of that function

        } else if (screen_state == 1)
        {
            screen_state = 2; //again to be changed to some other function
        } else if (screen_state == 2)
        {
            //display high scores and wait for button press to go back to start screen
            if(button_pressed)
            {
                screen_state = 0;
            }
        }
    }
}


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