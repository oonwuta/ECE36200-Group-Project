// #include <stdio.h>
// #include <string.h>
// #include "pico/stdlib.h"
// #include "hardware/timer.h"
// #include "hardware/irq.h"
// #include "music.h"
// #include "joystick.h"

// //////////////////////////////////////////////////////////////////////////////



// //////////////////////////////////////////////////////////////////////////////


// /*
// int main()
// {
//     joystick_init();
//     uint16_t x = 0;
//     uint16_t y = 0;
//     bool button_pressed = false;
//     int screen_state = 0; //0 is start screen, 1 is game, 2 is high score
//     matrix_init();
//     init_pwm_audio();
//     highscores_init_defaults(); //not yet included
//     while(1){
//         joystick_read(&x, &y);
//         button_read(button_pressed);
//         if(screen_state == 0)
//         {
//             //some code that accepts x, y and button press to select and returns the next screen state
//             screen_state = 1; //to be changed to the output of that function

//         } else if (screen_state == 1)
//         {
//             screen_state = 2; //again to be changed to some other function
//         } else if (screen_state == 2)
//         {
//             //display high scores and wait for button press to go back to start screen
//             if(button_pressed)
//             {
//                 screen_state = 0;
//             }
//         }
//     }
// }


// psuedocode draft for main is 
// main: //kind of a state machine
//     game start, init joystick,
//     while(1):
//         read joystick
//         if in start screen:
//             update cursor position based on joystick
//             if select pressed on start:
//                 start
//             else if highscore pressed:
//                 highscore
//         else if in game:
//             dead = update_snake(joystickx, joysticky) //function will have to know board state and return if player lost or not
//         else if dead:
//             print score, print if high score //can also just print leaderboard 
//             wait for button to go back to start screen
// */