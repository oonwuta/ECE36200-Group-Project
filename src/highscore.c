#include "highscore.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//kind of based off of https://github.com/antgon/pico-eeprom24xx01/blob/main/lib/include/eeprom24xx01.h
//but need to insert more than just an integer value 

#define MAX_REASONABLE_SCORE 1000000U

static inline bool is_valid_name_char(uint8_t c) {
    // allow uppercase letters, digits, and space
    if (c == ' ') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= '0' && c <= '9') return true;
    return false;
}

static bool raw_looks_erased_or_zero(const uint8_t *raw, size_t len) {
    size_t ff = 0, zz = 0;
    for (size_t i = 0; i < len; ++i) {
        if (raw[i] == 0xFF) ff++;
        if (raw[i] == 0x00) zz++;
    }
    // if most of the bytes are 0xFF or 0x00, consider it erased/garbage
    return (ff * 100 / len) > 80 || (zz * 100 / len) > 80;
}

static int eeprom_wait_ack(i2c_inst_t *i2c, uint8_t addr, int timeout_ms) {
    int waited = 0;
    uint8_t dummy = 0;
    while (waited < timeout_ms) {
        //zero-length write
        int r = i2c_write_blocking(i2c, addr, &dummy, 0, false);
        if (r >= 0) {
            return 0;
        }
        sleep_ms(1);
        waited += 1;
    }
    return -1;
}

void eeprom_init(i2c_inst_t *i2c_port, uint8_t i2c_addr, highscore_eeprom_t *e) {
    e->i2c_port = i2c_port;
    e->i2c_addr = i2c_addr;

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    i2c_init(i2c0, I2C_BAUDRATE);
}

static int eeprom_write_bytes(highscore_eeprom_t *e, uint16_t mem_addr, const uint8_t *buf, size_t len) {
    if (!e || !e->i2c_port) {
        return -1;
    }
    while (len > 0) {
        //page math
        uint16_t page_offset = mem_addr % EEPROM_PAGE_SIZE;
        size_t space = EEPROM_PAGE_SIZE - page_offset;

        size_t chunk = 0;
        if (len < space) {
            chunk = len;
        }
        else {
            chunk = space;
        }

        //txbuf
        uint8_t tx[2 + EEPROM_PAGE_SIZE];
        tx[0] = (uint8_t)(mem_addr >> 8);
        tx[1] = (uint8_t)(mem_addr & 0xFF);
        memcpy(&tx[2], buf, chunk);
        i2c_write_blocking(e->i2c_port, e->i2c_addr, tx, (int)(2 + chunk), false);

        //wait for internal write to finish
        if (eeprom_wait_ack(e->i2c_port, e->i2c_addr, EEPROM_WRITE_MS + 10) < 0) {
            sleep_ms(EEPROM_WRITE_MS);
        }

        mem_addr = (uint16_t)(mem_addr + chunk);
        buf += chunk;
        len -= chunk;
    }
    return 0;
}

static int eeprom_read_bytes(highscore_eeprom_t *e, uint16_t mem_addr, uint8_t *buf, size_t len) {
    if (!e || !e->i2c_port) {
        return -1;
    }

    uint8_t addrbuf[2];
    addrbuf[0] = (uint8_t)(mem_addr >> 8);
    addrbuf[1] = (uint8_t)(mem_addr & 0xFF);

    int r = i2c_write_blocking(e->i2c_port, e->i2c_addr, addrbuf, 2, true); // repeated start
    if (r < 0) {
        return -1;
    }

    int n = i2c_read_blocking(e->i2c_port, e->i2c_addr, buf, (int)len, false);
    if (n < 0 || (size_t)n != len) {
        return -1;
    }

    return 0;
}


//highscore operations
int highscores_load(highscore_eeprom_t *e, hs_entry_t out[HS_COUNT]) {
    printf("HIGHSCORES LOAD\n");
    if (!e) {
        return -1;
    }
    uint8_t raw[HS_TOTAL_BYTES];
    if (eeprom_read_bytes(e, (uint16_t)EEPROM_ADDR, raw, HS_TOTAL_BYTES) < 0) {
        return -1;
    }

    // quick detect common erased/cleared garbage
    if (raw_looks_erased_or_zero(raw, HS_TOTAL_BYTES)) {
        printf("Highscores: raw looks erased/zero -> fail\n");
        return -1;
    }

    // for (int i = 0; i < HS_COUNT; i++) {
    //     int off = i * HS_ENTRY_BYTES;
    //     memcpy(out[i].name, &raw[off], 3);
    //     uint32_t s = (uint32_t)raw[off+3] << 24 | (uint32_t)raw[off+2] << 16 | (uint32_t)raw[off+1] << 8 | (uint32_t)raw[off];
    //     s = (uint32_t)raw[off] | ((uint32_t)raw[off+1] << 8) | ((uint32_t)raw[off+2] << 16) | ((uint32_t)raw[off+3] << 24);
    //     printf("name and score from eeprom: %c %c %c, %d\n", out[i].name[0], out[i].name[1], out[i].name[2], out[i].score);
    //     out[i].score = s;
    // }

    // decode and validate
    for (int i = 0; i < HS_COUNT; i++) {
        int off = i * HS_ENTRY_BYTES;
        // copy name bytes first and validate chars
        for (int n = 0; n < 3; ++n) {
            uint8_t nc = raw[off + n];
            out[i].name[n] = nc;
        }
        uint32_t s = (uint32_t)raw[off+3] | ((uint32_t)raw[off+4] << 8) | ((uint32_t)raw[off+5] << 16) | ((uint32_t)raw[off+6] << 24);
        // validate score range
        if (s > MAX_REASONABLE_SCORE) {
            printf("Highscores: invalid score %u at entry %d\n", s, i);
            return -1;
        }
        out[i].score = s;
    }


    for (int i = 0; i < HS_COUNT; i++) {
        printf("name and score from eeprom: %c %c %c, %d\n", out[i].name[0] + 'A', out[i].name[1] + 'A', out[i].name[2] + 'A', out[i].score);
    }

    return 0;
}

int highscores_save(highscore_eeprom_t *e, const hs_entry_t in[HS_COUNT]) {
    printf("HIGHSCORES SAVE\n");
    if (!e) {
        return -1;
    }
    uint8_t raw[HS_TOTAL_BYTES];
    for (int i = 0; i < HS_COUNT; i++) {
        int off = i * HS_ENTRY_BYTES;

        //copy exactly 3 bytes of name
        memcpy(&raw[off], in[i].name, 3);

        uint32_t s = in[i].score;
        raw[off+6] = (uint8_t)((s >> 24) & 0xFF);
        raw[off+5] = (uint8_t)((s >> 16) & 0xFF);
        raw[off+4] = (uint8_t)((s >> 8) & 0xFF);
        raw[off+3] = (uint8_t)(s & 0xFF);
    }

    //write entire block (28 bytes)
    return eeprom_write_bytes(e, (uint16_t)EEPROM_ADDR, raw, HS_TOTAL_BYTES);
}

int highscores_insert_inplace(hs_entry_t arr[HS_COUNT], const hs_entry_t *newe) {
    //find position by score (descending)
    int pos = HS_COUNT;
    for (int i = 0; i < HS_COUNT; i++) {
        if (newe->score > arr[i].score) { 
            pos = i; 
            break; 
        }
    }

    //if score is not high enough
    if (pos >= HS_COUNT) {
        return 0;
    }

    //shift down
    for (int i = HS_COUNT - 1; i > pos; i--) {
        arr[i] = arr[i-1];
    }
    arr[pos] = *newe;

    return 1;
}

int highscores_insert_and_save(highscore_eeprom_t *e, const hs_entry_t *newe) {
    if (!e || !newe) {
        return -1;
    }
    hs_entry_t arr[HS_COUNT];

    //if load fails, initialize with zeros
    if (highscores_load(e, arr) < 0) {
        for (int i = 0; i < HS_COUNT; i++) {
            memset(arr[i].name, ' ', 3);
            arr[i].score = 0;
        }
    }

    int changed = highscores_insert_inplace(arr, newe);
    if (!changed) {
        return 0;
    }

    if (highscores_save(e, arr) < 0) {
        return -1;
    }

    return 1;
}
