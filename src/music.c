/********************************************************************************************
This is used as an 8 bit audio handler, can generate notes A-G +- one octave and # or *(flat)
********************************************************************************************/
#include "music.h"

extern const int canon_in_d[32] = {C,C*2,E*2,G*2,G/2,G,B,D*2,
                      A/2,A,C*2,E*2,E/2,E,G,B,
                      F/2,F,A,C*2,C/2,C,E,G,
                      F/2,F,A,C*2,G/2,G,B,D*2};

/////////////////////////////////////////////////////////////////////////////////////////////

void play_song(int bpm,const int* song,int channel){
    //plays defined song on desired channel 
    init_pwm_audio();

    for(;;){
        for(int i = 0; i < 32; i++){
            set_freq((channel % 2), song[i]);
            sleep_ms(60000/bpm);
        }
    }
}