#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define I2C_PORT       i2c0
#define I2C_SDA_PIN -1
#define I2C_SCL_PIN -1
#define I2C_BAUDRATE 100000 //not sure
#define EEPROM_I2C_ADDR 0x50  //24LC256 typical (A2,A1,A0 = 0)
#define EEPROM_PAGE_SIZE 64 //bytes per write page
#define EEPROM_WRITE_CYCLE_MS 5 //wait after page write
#define HS_COUNT 5 //top N scores, not sure what max would be
#define EEPROM_BASE_ADDR 0x0000 // starting memory offset in EEPROM
#define EEPROM_MAGIC 0xDEADBEEFU //magic number is known default value (found online)
#define MAGIC_ADDR (EEPROM_BASE_ADDR)
#define SCORES_ADDR (EEPROM_BASE_ADDR + 4)
#define SCORES_BYTES (HS_COUNT * sizeof(uint32_t))

//eeprom
//write buffer to eeprom
static int eeprom_write(uint16_t mem_addr, const uint8_t *buf, size_t len) {
    while(len > 0) 
    {
        //page boundary math
        uint16_t page_offset = mem_addr % EEPROM_PAGE_SIZE;
        size_t space_in_page = EEPROM_PAGE_SIZE - page_offset;
        size_t chunk = (len < space_in_page) ? len : space_in_page;

        //build tx buffer: [addr_high, addr_low, data...]
        size_t tx_len = 2 + chunk;
        uint8_t txbuf[2 + EEPROM_PAGE_SIZE];
        txbuf[0] = (uint8_t)(mem_addr >> 8);
        txbuf[1] = (uint8_t)(mem_addr & 0xFF);
        memcpy(&txbuf[2], buf, chunk);

        int written = i2c_write_blocking(I2C_PORT, EEPROM_I2C_ADDR, txbuf, tx_len, false);
        if(written < 0) 
        {
            return -1; // i2c error
        }

        //EEPROM needs internal write cycle time to commit the page.
        sleep_ms(EEPROM_WRITE_CYCLE_MS);

        //advance pointers
        mem_addr += chunk;
        buf += chunk;
        len -= chunk;
    }
    return 0;
}

//read len bytes from EEPROM starting at mem_addr into buf
static int eeprom_read(uint16_t mem_addr, uint8_t *buf, size_t len) {
    uint8_t addrbuf[2];
    addrbuf[0] = (uint8_t)(mem_addr >> 8);
    addrbuf[1] = (uint8_t)(mem_addr & 0xFF);
    //send mem address with no stop
    int r = i2c_write_blocking(I2C_PORT, EEPROM_I2C_ADDR, addrbuf, 2, true);
    if(r < 0) 
    {
        return -1;
    }

    //then read the data
    int n = i2c_read_blocking(I2C_PORT, EEPROM_I2C_ADDR, buf, len, false);
    if(n < 0) 
    {
        return -1;
    }
    if((size_t)n != len) 
    {
        return -1;
    }
    return 0;
}

//high score

// Read the stored magic value; return 0 on success, -1 on i2c error
static int eeprom_read_magic(uint32_t *magic_out) {
    uint8_t buf[4];
    if(eeprom_read(MAGIC_ADDR, buf, 4) < 0) 
    {
        return -1;
    }

    //little-endian decode
    *magic_out = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    return 0;
}

static int eeprom_write_magic(uint32_t magic) {
    uint8_t buf[4];
    buf[0] = (uint8_t)(magic & 0xFF);
    buf[1] = (uint8_t)((magic >> 8) & 0xFF);
    buf[2] = (uint8_t)((magic >> 16) & 0xFF);
    buf[3] = (uint8_t)((magic >> 24) & 0xFF);
    return eeprom_write(MAGIC_ADDR, buf, 4);
}

// Load high scores into provided array (length HS_COUNT). Returns 0 on success.
int load_highscores(uint32_t scores_out[HS_COUNT]) {
    uint8_t buf[SCORES_BYTES];

    //test
    if(eeprom_read(SCORES_ADDR, buf, SCORES_BYTES) < 0) 
    {
        return -1;
    }

    for(int i = 0; i < HS_COUNT; i++) 
    {
        size_t off = i * 4;
        //decode little-endian stored uint32
        scores_out[i] = (uint32_t)buf[off] | ((uint32_t)buf[off+1] << 8) | ((uint32_t)buf[off+2] << 16) | ((uint32_t)buf[off+3] << 24);
    }

    return 0;
}

//save high scores from array into EEPROM (should just return 0 on success)
int save_highscores(const uint32_t scores[HS_COUNT]) {
    uint8_t buf[SCORES_BYTES];
    for(int i = 0; i < HS_COUNT; i++) 
    {
        size_t off = i * 4;
        uint32_t v = scores[i];
        buf[off]   = (uint8_t)(v & 0xFF);
        buf[off+1] = (uint8_t)((v >> 8) & 0xFF);
        buf[off+2] = (uint8_t)((v >> 16) & 0xFF);
        buf[off+3] = (uint8_t)((v >> 24) & 0xFF);
    }

    int r = eeprom_write(SCORES_ADDR, buf, SCORES_BYTES);
    return r;
}

//insert a new score into the top list (descending order, saves back to EEPROM)

int highscores_insert_and_save(uint32_t new_score) {
    uint32_t scores[HS_COUNT];
    //load existing
    //if i2c error treat as empty list of zeros
    if(load_highscores(scores) < 0) 
    {
        for(int i = 0; i < HS_COUNT; i++) 
        {
            scores[i] = 0;
        }
    }

    //insert new_score in descending order
    int pos = HS_COUNT;
    for(int i = 0; i < HS_COUNT; i++) 
    {
        if(new_score > scores[i]) 
        {
            pos = i;
            break;
        }
    }
    if(pos < HS_COUNT) 
    {
        //shift down
        for(int i = HS_COUNT - 1; i > pos; i--) 
        {
            scores[i] = scores[i - 1];
        }
        scores[pos] = new_score;

        //similar as load
        if(save_highscores(scores) < 0) 
        {
            return -1;
        }
    }

    return 0;
}

//clears highscores and writes default zeros and magic

int highscores_init_defaults() {
    uint32_t zeros[HS_COUNT];
    for(int i = 0; i < HS_COUNT; i++) 
    {
        zeros[i] = 0;
    }

    if(save_highscores(zeros) < 0) 
    {
        return -1;
    }

    if(eeprom_write_magic(EEPROM_MAGIC) < 0) 
    {
        return -1;
    }
    return 0;
}

//test init

int highscores_ensure_initialized() {
    uint32_t magic = 0;
    if(eeprom_read_magic(&magic) < 0) 
    {
        return -1;
    }

    if(magic != EEPROM_MAGIC) 
    {
        return highscores_init_defaults();
    }

    return 0;
}

void i2c_init_pins() {
    i2c_init(I2C_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
}