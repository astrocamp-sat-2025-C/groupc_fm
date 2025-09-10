// servo.hpp
// Simple servo control API for Raspberry Pi Pico.
// Provides functions to initialize a GPIO for PWM and set servo position
// using either a normalized speed (-1..1) or direct microseconds pulse width.

#ifndef SERVO_HPP
#define SERVO_HPP

#include <stdint.h>
#include "pico/stdlib.h"

void servo_init(uint pin);

void servo_set_speed(uint pin, float speed);

void servo_set_pulse_us(uint pin, uint16_t us);

void servo_rotate_forward();

void servo_rotate_reverse();

void servo_rotate_forward_diff() ;

void servo_rotate_reverse_diff();

#endif // SERVO_HPP

