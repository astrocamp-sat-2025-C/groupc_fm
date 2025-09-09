#include <pico/stdlib.h>
#include "hardware/spi.h"
#include <pico/types.h>
#include <stdio.h>
#include <stdbool.h>
#include "diode.hpp"

void photoreflector_init(void)
{
    spi_init(SPI_PORT, 1000 * 1000); // 1MHz
    gpio_set_function(PIN_PHOTOREFLECTOR_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_PHOTOREFLECTOR_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_PHOTOREFLECTOR_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_PHOTOREFLECTOR_CS, GPIO_FUNC_SIO);
    gpio_init(PIN_PHOTOREFLECTOR_CS);
    gpio_set_dir(PIN_PHOTOREFLECTOR_CS, GPIO_OUT);
    gpio_put(PIN_PHOTOREFLECTOR_CS, 1);
}

uint16_t diode_read(uint8_t channel)
{
    uint8_t txbuf[3];
    uint8_t rxbuf[3];
    txbuf[0] = 0x01; // start bit
    txbuf[1] = 0x80 | ((channel & 0x07) << 4);
    txbuf[2] = 0x00; // don't care

    gpio_put(PIN_PHOTOREFLECTOR_CS, 0); // CS active
    spi_write_read_blocking(SPI_PORT, txbuf, rxbuf, 3);
    gpio_put(PIN_PHOTOREFLECTOR_CS, 1); // CS inactive

    // rxbuf[1] contains the top two bits of the 10-bit result in its LSBs (bits 0-1)
    // rxbuf[2] contains the lower 8 bits.
    uint16_t result = ((rxbuf[1] & 0x03) << 8) | rxbuf[2];
    return result;
}

void photoreflector_read(uint16_t *results)
{
    for (uint8_t ch = 0; ch <= 3; ++ch)
    {
        results[ch] = diode_read(ch);
    }
}

void photodiode_read(uint16_t *result)
{
    *result = diode_read(4);
}
