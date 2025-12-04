#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <stdint.h>
#include "hardware/i2c.h"

//eeprom device parameters
#define EEPROM_I2C_ADDR_DEFAULT 0x50
#define EEPROM_PAGE_SIZE 32
#define EEPROM_BLOCK_SIZE 128 //not too sure what this should be
#define EEPROM_WRITE_TIME_MS 5

//addresses where scores are stored inside EEPROM 
#define SCORES_BASE_ADDR 0x00
#define HS_COUNT 10 //just doing top 10 scores for now (could change)
#define SCORES_BYTES (HS_COUNT * 4) //4 bytes per uint32_t

//saw eeprom structures being used in other libraries
typedef struct {
    i2c_inst_t *i2c_port;
    uint8_t i2c_addr;
} highscore_eeprom_t;

void eeprom_init(i2c_inst_t *i2c_port, uint8_t i2c_addr, highscore_eeprom_t *eeprom);
void eeprom_erase(highscore_eeprom_t *eeprom);
void eeprom_write_byte(uint8_t addr, uint8_t val, highscore_eeprom_t *eeprom);
void eeprom_write_page(uint8_t addr, uint8_t buf[], highscore_eeprom_t *eeprom);
void eeprom_read_byte(uint8_t *addr, uint8_t *dst, highscore_eeprom_t *eeprom);
void eeprom_read_page(uint8_t *addr, uint8_t dst[], highscore_eeprom_t *eeprom);

int highscores_load(highscore_eeprom_t *eeprom, uint32_t out[HS_COUNT]);
int highscores_save(highscore_eeprom_t *eeprom, const uint32_t in[HS_COUNT]);
int highscores_insert(uint32_t scores[HS_COUNT], uint32_t new_score);
int highscores_insert_and_save(highscore_eeprom_t *eeprom, uint32_t new_score);

#endif
