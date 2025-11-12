#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "music.h"
#include "led_matrix.h"

//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////


int main()
{
    stdio_init_all(); 
    //play_song(120 ,canon_in_d,0);
    matrix_test();
    for(;;);
    return 0;
}
