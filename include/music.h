#ifndef MUSIC_H
#define MUSIC_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "audio.h"

/* ===== NOTES ===== */
enum notes {
    C  = 261,
    CS = 277, DF = CS,
    D  = 293,
    DS = 311, EF = DS,
    E  = 330,
    F  = 350,
    FS = 379, GF = FS,
    G  = 392,
    GS = 415, AF = GS,
    A  = 440,
    AS = 466, BF = AS,
    B  = 493,
    R  = 0
};

/* ===== SONGS ===== */
extern const int canon_in_d[32];

/* ===== API ===== */
void play_song(int bpm, const int *song, int channel);
void stop_song(void); 
void play_note(int note, int duration, int channel);
#endif
