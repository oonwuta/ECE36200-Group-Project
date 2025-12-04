#include "highscore.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdlib.h>

//eeprom code
void eeprom_init(i2c_inst_t *i2c_port, uint8_t i2c_addr, highscore_eeprom_t *eeprom){
    eeprom->i2c_port = i2c_port;
    eeprom->i2c_addr = i2c_addr;
}

void eeprom_erase(highscore_eeprom_t *eeprom){
    for (uint8_t addr=0; addr<EEPROM_BLOCK_SIZE; addr++)
    {
        eeprom_write_byte(addr, 255, eeprom);
    }
}

void eeprom_write_byte(uint8_t addr, uint8_t val, highscore_eeprom_t *eeprom) {
    uint8_t buf[2] = {addr, val};
    i2c_write_blocking(eeprom->i2c_port, eeprom->i2c_addr, buf, 2, false);
    sleep_ms(EEPROM_WRITE_TIME_MS);
}

void eeprom_write_page(uint8_t addr, uint8_t buf[], highscore_eeprom_t *eeprom) {   
    uint8_t *temp_buf = malloc(EEPROM_PAGE_SIZE + 1);
    if (!temp_buf) 
    {
        return;
    }

    temp_buf[0] = addr;
    memcpy(temp_buf+1, buf, EEPROM_PAGE_SIZE);
    i2c_write_blocking(eeprom->i2c_port, eeprom->i2c_addr, temp_buf, EEPROM_PAGE_SIZE + 1, false);
    free(temp_buf);
    sleep_ms(EEPROM_WRITE_TIME_MS);
}

void eeprom_read_byte(uint8_t *addr, uint8_t *dst, highscore_eeprom_t *eeprom) {    
    i2c_write_blocking(eeprom->i2c_port, eeprom->i2c_addr, addr, 1,true);
    i2c_read_blocking(eeprom->i2c_port, eeprom->i2c_addr, dst, 1, false);
}

void eeprom_read_page(uint8_t *addr, uint8_t dst[], highscore_eeprom_t *eeprom) {
    i2c_write_blocking(eeprom->i2c_port, eeprom->i2c_addr, addr, 1, true);
    i2c_read_blocking(eeprom->i2c_port, eeprom->i2c_addr, dst, EEPROM_PAGE_SIZE, false);
}


//highscore 
//read SCROES_BYTES from EEPROM into out[]
int highscores_load(highscore_eeprom_t *eeprom, uint32_t out[HS_COUNT]) {
    if (!eeprom) 
    {
        return -1;
    }
    uint8_t buf[SCORES_BYTES];
    for (size_t i = 0; i < SCORES_BYTES; i++) 
    {
        uint8_t addr = (uint8_t)(SCORES_BASE_ADDR + i);
        uint8_t v = 0;
        eeprom_read_byte(&addr, &v, eeprom);
        buf[i] = v;
    }
    // decode little-endian 4-byte words
    for (int i = 0; i < HS_COUNT; i++) 
    {
        size_t off = i * 4;
        uint32_t val = (uint32_t)buf[off] |
                       ((uint32_t)buf[off+1] << 8) |
                       ((uint32_t)buf[off+2] << 16) |
                       ((uint32_t)buf[off+3] << 24);
        out[i] = val;
    }
    return 0;
}

//save SCORES_BYTES from in[] to EEPROM
int highscores_save(highscore_eeprom_t *eeprom, const uint32_t in[HS_COUNT]) {
    if (!eeprom) 
    {
        return -1;
    }

    uint8_t buf[SCORES_BYTES];
    for (int i = 0; i < HS_COUNT; i++) 
    {
        size_t off = i * 4;
        uint32_t v = in[i];
        buf[off] = (uint8_t)(v & 0xFF);
        buf[off+1] = (uint8_t)((v >> 8) & 0xFF);
        buf[off+2] = (uint8_t)((v >> 16) & 0xFF);
        buf[off+3] = (uint8_t)((v >> 24) & 0xFF);
    }

    //write byte-by-byte
    for (size_t i = 0; i < SCORES_BYTES; i++) 
    {
        uint8_t addr = (uint8_t)(SCORES_BASE_ADDR + i);
        eeprom_write_byte(addr, buf[i], eeprom);
    }
    return 0;
}

//insert new_score into scores[], shifting down and dropping last
//will return 1 if there is a change, 0 if none.
int highscores_insert(uint32_t scores[HS_COUNT], uint32_t new_score) {
    int pos = HS_COUNT;
    for (int i = 0; i < HS_COUNT; i++) 
    {
        if (new_score > scores[i]) 
        { 
            pos = i; 
            break;
        }
    }
    if (pos >= HS_COUNT) 
    {
        return 0; //no change
    }

    //shift down
    for (int i = HS_COUNT - 1; i > pos; i--) 
    {
        scores[i] = scores[i-1];
    }
    scores[pos] = new_score;

    return 1;
}

int highscores_insert_and_save(highscore_eeprom_t *eeprom, uint32_t new_score) {
    if (!eeprom) 
    {
        return -1;
    }

    uint32_t scores[HS_COUNT];
    if (highscores_load(eeprom, scores) < 0) 
    {
        return -1;
    }
    int changed = highscores_insert(scores, new_score);

    if (!changed) 
    {
        return 0;
    }

    if (highscores_save(eeprom, scores) < 0) 
    {
        return -1;
    } 
    return 1;
}
