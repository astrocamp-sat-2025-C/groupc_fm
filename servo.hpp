// servo.hpp
// Simple servo control API for Raspberry Pi Pico.
// Provides functions to initialize a GPIO for PWM and set servo position
// using either a normalized speed (-1..1) or direct microseconds pulse width.

#ifndef SERVO_HPP
#define SERVO_HPP

#include <stdint.h>
#include "pico/stdlib.h"

// Initialize the given GPIO pin for servo PWM. Safe to call multiple times.
// pin: GPIO number supporting PWM.
void servo_init(uint pin);

// Set servo using a normalized speed/position in range [-1.0, 1.0].
// -1.0 => about 700us, 0.0 => about 1500us, 1.0 => about 2300us (configurable in impl).
void servo_set_speed(uint pin, float speed);

// Set servo pulse directly in microseconds (typical 500..2500 range).
void servo_set_pulse_us(uint pin, uint16_t us);

#endif // SERVO_HPP

