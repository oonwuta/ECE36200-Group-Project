#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define HS_COUNT 5

int load_highscores(uint32_t scores_out[HS_COUNT]);
int save_highscores(const uint32_t scores[HS_COUNT]);
int highscores_insert_and_save(uint32_t new_score);
int highscores_init_defaults();
int highscores_ensure_initialized();
void i2c_init_pins();