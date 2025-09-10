#include "accel.hpp" // commented out sensor usage below
#include "adc.hpp"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "servo.hpp"
#include <pico/types.h>
#include <stdbool.h>
#include <stdio.h>

#define SERVO_PIN 11

// IMU scaling: set to match your sensor's configured full-scale ranges
static constexpr float ACCEL_FS_RANGE = 16.0f;  // e.g., ±16 g
static constexpr float GYRO_FS_RANGE = 2000.0f; // e.g., ±2000 dps
static constexpr float ACCEL_SENSITIVITY =
    32768.0f / ACCEL_FS_RANGE; // counts per g
static constexpr float GYRO_SENSITIVITY =
    32768.0f / GYRO_FS_RANGE; // counts per dps

int main() {
  stdio_init_all();
  gpio_set_function(12, UART_FUNCSEL_NUM(uart0, 0));
  gpio_set_function(13, UART_FUNCSEL_NUM(uart0, 1));

  // Initialise UART 0
  uart_init(uart0, 115200);
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

  i2c_scan();
  photoreflector_init();

  if (!icm42688_init()) {
    printf("ICM-42688-P init failed\n");
  }

  while (true) {
    uint16_t photodiode_result;
    uint16_t photoreflector_results[4];

    photodiode_read(&photodiode_result);
    photoreflector_read(photoreflector_results);

    //printf("Photodiode: %d\n", photodiode_result);
    // printf("Photoreflector: %d %d %d %d\n", photoreflector_results[0],
    //        photoreflector_results[1], photoreflector_results[2],
    //        photoreflector_results[3]);

    int16_t ax, ay, az, gx, gy, gz;
    if (read_accel_gyro_burst(&ax, &ay, &az, &gx, &gy, &gz)) {
      // Convert raw counts to physical units using configured sensitivity
      float ax_g = (float)ax / ACCEL_SENSITIVITY;
      float ay_g = (float)ay / ACCEL_SENSITIVITY;
      float az_g = (float)az / ACCEL_SENSITIVITY;

      float gx_dps = (float)gx / GYRO_SENSITIVITY;
      float gy_dps = (float)gy / GYRO_SENSITIVITY;
      float gz_dps = (float)gz / GYRO_SENSITIVITY;

      printf("%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
             photoreflector_results[0], photoreflector_results[1],
             photoreflector_results[2], photoreflector_results[3], ax_g, ay_g,
             az_g, gx_dps, gy_dps, gz_dps);
    } else {
      printf("read_accel_gyro_burst failed\n");
      uint8_t who = 0;
      if (probe_who_am_i(&who))
        printf("WHO_AM_I=0x%02X at I2C 0x%02X\n", who, I2C_ADDR);
    }
    // sleep_ms(100);
    // servo_init(SERVO_PIN);

    // servo_set_speed(SERVO_PIN, 1.0);   // 正転
    // printf("FORWARD\n");
    // sleep_ms(400);

    // servo_set_speed(SERVO_PIN, -1.0);  // 正転補正
    // printf("REVERSE\n");
    // sleep_ms(30);

    // servo_set_speed(SERVO_PIN, 0.0);   // 停止
    // printf("STOP1\n");
    // sleep_ms(1500);

    // servo_set_speed(SERVO_PIN, 1.0);   // 正転
    // printf("FORWARD\n");
    // sleep_ms(400);

    // servo_set_speed(SERVO_PIN, 0.0);   // 停止
    // printf("STOP1\n");
    // sleep_ms(1000);

    // servo_set_speed(SERVO_PIN, -1.0);  // 逆転
    // printf("REVERSE\n");
    // sleep_ms(400);

    // servo_set_speed(SERVO_PIN, 1.0);   // 逆転補正
    // printf("FORWARD\n");
    // sleep_ms(30);

    // servo_set_speed(SERVO_PIN, 0.0);   // 停止
    // printf("STOP2\n");
    // sleep_ms(1000);
  }
  return 0;
}