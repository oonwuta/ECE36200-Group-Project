/********************************************************************************************
This is used as an 8 bit audio handler, can generate notes A–G +- one octave and # or *(flat)
********************************************************************************************/
#include "music.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

/* ===== SONG DATA ===== */
const int canon_in_d[32] = {
    C, C*2, E*2, G*2, G/2, G, B, D*2,
    A/2, A, C*2, E*2, E/2, E, G, B,
    F/2, F, A, C*2, C/2, C, E, G,
    F/2, F, A, C*2, G/2, G, B, D*2
};

/* ===== INTERNAL STATE (IRQ SHARED) ===== */
static const int *g_song;
static int g_song_len;
static int g_song_index;
static int g_bpm;
static int g_channel;

/* ===== TIMER IRQ ===== */
void play_note_irq(void) {
    // clear alarm 0 interrupt
    timer0_hw->intr = (1u << 0);

    set_freq(g_channel % 2, g_song[g_song_index]);

    g_song_index++;
    if (g_song_index >= g_song_len) {
        g_song_index = 0; // loop
    }

    // schedule next beat
    timer0_hw->alarm[0] =
        timer0_hw->timerawl + (60000000 / g_bpm);
}

/* ===== TIMER INIT ===== */
static void init_audio_timer(int bpm) {
    g_bpm = bpm;

    timer0_hw->inte = (1u << 0);
    irq_set_exclusive_handler(TIMER0_IRQ_0, play_note_irq);
    irq_set_enabled(TIMER0_IRQ_0, true);

    timer0_hw->alarm[0] =
        timer0_hw->timerawl + (60000000 / bpm);
}

/* ===== PUBLIC API ===== */
void play_song(int bpm, const int* song, int channel) {
    init_pwm_audio();

    g_song = song;
    g_song_len = 32;
    g_song_index = 0;
    g_channel = channel;

    init_audio_timer(bpm);
}
