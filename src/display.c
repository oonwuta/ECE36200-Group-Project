#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"

const int SPI_LED_SCK = -1; //replace with the SCK pin number for the LED display
const int SPI_LED_CSn = -1; //replace with the CSn pin number for the LED display
const int SPI_LED_TX = -1; //replace with the TX pin number for the LED display

//majority of this code is from lab 6

//if we want to do SPI
void init_led_pins() {
    gpio_set_function(SPI_LED_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_LED_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI_LED_CSn, GPIO_FUNC_SPI);

    spi_init(spi0, 10000); //can change baud rate (might also need to change spi channel)
    spi_set_format(spi0, 9, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); //temp format values for now
}

void send_spi_cmd(spi_inst_t* spi, uint16_t value) {
    for(;;)
    {
        if(!spi_is_busy(spi))
        {
            spi_write16_blocking(spi, &value, 1);
            break;
        }
    }
}

void send_spi_data(spi_inst_t* spi, uint16_t value) {
    value |= 0x100;
    send_spi_cmd(spi, value);
}

//need functions for menu display (mostly static, but should highlight option being selected)



//need functions for game itself (snake will constantly be moving, and points need to be randomly put on screen)



//need functions for current score being displayed at top row