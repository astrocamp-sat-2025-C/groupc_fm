#include <pico/stdlib.h>
#include "hardware/spi.h"
#include <stdio.h>
#include <stdbool.h>

#define SPI_PORT spi0
#define PIN_PHOTOREFLECTOR_CS 17
#define PIN_PHOTOREFLECTOR_MOSI 19
#define PIN_PHOTOREFLECTOR_SCK 18
#define PIN_PHOTOREFLECTOR_MISO 16

void photoreflector_init(void);
void photoreflector_read(uint16_t *results);
void photodiode_read(uint16_t *result);
