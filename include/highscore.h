#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <stdint.h>
#include "hardware/i2c.h"

//I2C
#define I2C_ADDR_DEFAULT 0x50 //grounding a0, a1, a2 should lead to this being addr
#define I2C_BAUDRATE 100000
#define EEPROM_PAGE_SIZE 32 
#define EEPROM_BLOCK_SIZE 4096 //4 KB
#define EEPROM_WRITE_MS 5

//highscore layout
#define HS_COUNT 4 //top 4 scores
#define HS_ENTRY_BYTES 7 //3 bytes name + 4 bytes score = 7
#define HS_TOTAL_BYTES 28 //highscore count * entry bytes = 28

//eeprom storage offset (should just be 0, not rlly needed)
#define EEPROM_ADDR 0x0000

typedef struct {
    i2c_inst_t *i2c_port;
    uint8_t i2c_addr; //7bit address
} highscore_eeprom_t;

//for our current score def (3 letter initial and then integer score)
typedef struct {
    char name[3]; //3 character initials
    uint32_t score;
} hs_entry_t;

void eeprom_init(i2c_inst_t *i2c_port, uint8_t i2c_addr, highscore_eeprom_t *e);
int highscores_load(highscore_eeprom_t *e, hs_entry_t out[HS_COUNT]);
int highscores_save(highscore_eeprom_t *e, const hs_entry_t in[HS_COUNT]);
int highscores_insert_inplace(hs_entry_t arr[HS_COUNT], const hs_entry_t *newe);
int highscores_insert_and_save(highscore_eeprom_t *e, const hs_entry_t *newe);
static int eeprom_wait_ack(i2c_inst_t *i2c, uint8_t addr, int timeout_ms);
static int eeprom_write_bytes(highscore_eeprom_t *e, uint16_t mem_addr, const uint8_t *buf, size_t len);
static int eeprom_read_bytes(highscore_eeprom_t *e, uint16_t mem_addr, uint8_t *buf, size_t len);

#endif
