#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.hpp"
#include "hardware/pwm.h"

// Internal defaults for PWM timing
static const float DEFAULT_CLKDIV = 40.0f; // 1 tick = 320ns when sys clock 125MHz
static const uint32_t DEFAULT_WRAP = 62500; // 62500 * 320ns = 20ms period

void servo_init(uint pin) {
    // configure GPIO for PWM and enable slice with default timing
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_clkdiv(slice, DEFAULT_CLKDIV);
    pwm_set_wrap(slice, DEFAULT_WRAP);
    pwm_set_enabled(slice, true);
}

void servo_set_pulse_us(uint pin, uint16_t us) {
    // Convert microseconds to channel level using DEFAULT_CLKDIV/WRAP
    uint slice = pwm_gpio_to_slice_num(pin);
    const float tick_ns = 320.0f; // matches DEFAULT_CLKDIV at 125MHz
    uint32_t level = (uint32_t)(((float)us * 1000.0f) / tick_ns);
    if (level > DEFAULT_WRAP) level = DEFAULT_WRAP;
    // set channel level on the correct channel for this GPIO
    pwm_set_chan_level(slice, (pwm_gpio_to_channel(pin) == PWM_CHAN_A) ? PWM_CHAN_A : PWM_CHAN_B, level);
}

void servo_set_speed(uint pin, float speed) {
    if (speed > 1.0f) speed = 1.0f;
    if (speed < -1.0f) speed = -1.0f;
    // map -1..1 to 700..2300 us (typical continuous-rotation-like mapping)
    const float mid = 1500.0f;
    const float span = 800.0f; // +/- 800us
    float usf = mid + span * speed;
    servo_set_pulse_us(pin, (uint16_t)usf);
}


void init_servo_pwm(uint pin) {
    gpio_set_function(11, GPIO_FUNC_PWM); // PWM対応ピン
    uint slice_num = pwm_gpio_to_slice_num(11);
    pwm_set_clkdiv(slice_num, 40.0f);         // 1カウント = 320ns
    pwm_set_wrap(slice_num, 62500);           // 62500 × 320ns = 20ms周期
    pwm_set_enabled(slice_num, true);
}

void set_servo_speed(uint pin, float speed) {
    uint slice_num = pwm_gpio_to_slice_num(11);
    uint pulse_us = (1500 + (int)(800 * speed)) * 1000.0 / 320.0;
    pwm_set_clkdiv(slice_num, 40.0f);         // 1カウント = 320ns
    pwm_set_chan_level(slice_num, PWM_CHAN_B, pulse_us); // 4688 × 320ns ≒ 1500μs など
    printf("pluse_us:%d\n",pulse_us);
}

void gradinc_servo_speed(uint pin, float speed){ //0.0 to speed
    const float span = 800.0f;
    for(int i = 0; i < 100; i ++){
        float step = 800.0f * speed / 100.0;
        float usf = 1500.0f + step * i;
        servo_set_pulse_us(pin, (uint16_t)usf);
        i++;
        sleep_ms(30);
    }  
}



 void graddec_servo_speed(uint pin, float speed){
     //speed to 0.0
    const float span = 800.0f;
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 1.0f) speed = 1.0f;
    const int steps = 100;
    // start from given speed and decrease to 0.0 over 'steps' iterations
    for (int i = 0; i <= steps; ++i) {
       float t = speed * (1.0f - (float)i / (float)steps); // current normalized speed
      float usf = 1500.0f + span * t;
       servo_set_pulse_us(pin, (uint16_t)usf);
       sleep_ms(30);
    }
}

void graddec_servo_speed_rev(uint pin, float speed){
    // 逆回転（負方向）から 0.0 に戻す
    const float span = 800.0f;
    if (speed < 0.0f) speed = -speed;      // 負の入力を受けても扱えるように絶対値化
    if (speed > 1.0f) speed = 1.0f;
    const int steps = 100;
    for (int i = 0; i <= steps; ++i) {
        float t = speed * (1.0f - (float)i / (float)steps); // 正規化された現在の逆方向強さ
        float usf = 1500.0f - span * t; // 逆回転は 1500 - span * t
        servo_set_pulse_us(pin, (uint16_t)usf);
        sleep_ms(30);
    }
}

void servo_rotate_forward() {
    set_servo_speed(11, 1.0f); // 前進
    sleep_ms(500);
    set_servo_speed(11, 0.0f); // 停止
    sleep_ms(1000);
}

void servo_rotate_reverse() {
    set_servo_speed(11, -1.0f); // 後退
    sleep_ms(500);
    set_servo_speed(11, 0.0f); // 停止
    sleep_ms(1000);
}

void servo_rotate_forward_diff() {
  set_servo_speed(11, 0.3f); // 前進
  sleep_ms(500);
  set_servo_speed(11, 0.0f); // 停止
  sleep_ms(1000);
}

void servo_rotate_reverse_diff() {
  set_servo_speed(11, -0.3f); // 後退
  sleep_ms(500);
  set_servo_speed(11, 0.0f); // 停止
  sleep_ms(1000);
}

void servo_stop() {
    set_servo_speed(11, 0.0f); // 停止
    sleep_ms(1000);
}