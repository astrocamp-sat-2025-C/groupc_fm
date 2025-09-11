#include "accel.hpp" // IMU API
#include "adc.hpp"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "servo.hpp"
#include <pico/types.h>
#include <stdbool.h>
#include <stdio.h>
#include "MadgwickAHRS.h"
#include "track.hpp"

#define SERVO_PIN 11

float phase_angle = 180.0f; // 太陽から惑星に対する角度

static inline bool detect_sun_pair(const uint16_t vals[4]) {
  // Candidates: (0,1), (1,2), (2,3), (3,1)
  struct {
    int a;
    int b;
  } candidates[4] = {{0, 1}, {1, 2}, {2, 3}, {3, 1}};
  int best_first = -1;
  int best_second = -1;
  uint32_t max_sum = 0;
  for (int i = 0; i < 4; ++i) {
    int a = candidates[i].a;
    int b = candidates[i].b;
    uint32_t s = (uint32_t)vals[a] + (uint32_t)vals[b];
    if (best_first == -1 || s > max_sum) {
      max_sum = s;
      best_first = a;
      best_second = b;
    }
  }
  bool r = (best_first == 2 && best_second == 3);
  return r;
}

static inline bool is_within_10pct(int32_t s2, int32_t s3) {
  int32_t sum23 = s2 + s3;
  int32_t diff23 = (s2 > s3) ? (s2 - s3) : (s3 - s2);
  return (sum23 == 0) ? (diff23 == 0) : (diff23 <= (sum23 * 10) / 100);
}

int main() {
  stdio_init_all();
  gpio_set_function(12, UART_FUNCSEL_NUM(uart0, 0));
  gpio_set_function(13, UART_FUNCSEL_NUM(uart0, 1));

  uart_init(uart0, 115200);
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

  // i2c_scan();
  photoreflector_init();
  if (!icm42688_init()) {
    printf("IMU init failed!\n");
  }

  servo_init(SERVO_PIN);

  static int pair_count = 0;
  static int diff_count = 0;
  static int pair_lost_count = 0;
  static int diff_lost_count = 0;
  static bool pair_stable = false;
  static bool diff_stable = false;

  while (true) {
    uint16_t photodiode_result;
    uint16_t photoreflector_results[4];

    photodiode_read(&photodiode_result);
    photoreflector_read(photoreflector_results);
    bool current_pair = detect_sun_pair(photoreflector_results);
    bool current_diff =
        is_within_10pct(photoreflector_results[2], photoreflector_results[3]);

    // 安定状態管理
    if (!pair_stable) {
      if (current_pair) {
        if (pair_count < 10) {
          pair_count++;
        } else {
          pair_stable = true;
          pair_lost_count = 0;
        }
      } else {
        // servo_rotate_forward();
        pair_count = 0;
      }
    } else {
      if (current_pair) {
        pair_lost_count = 0;
      } else {
        pair_lost_count++;
        if (pair_lost_count >= 5) {
          pair_stable = false;
          pair_count = 0;
          pair_lost_count = 0;
        }
      }
    }

    if (!diff_stable) {
      if (pair_stable && current_diff) {
        if (diff_count < 10) {
          diff_count++;
        } else {
          diff_stable = true;
          diff_lost_count = 0;
        }
      } else if (!pair_stable) {
        // pairが不安定ならdiffもリセット
        diff_count = 0;
        diff_stable = false;
      } else {
        if (photoreflector_results[2] > photoreflector_results[3]) {
          // servo_rotate_forward_diff();
        } else if (photoreflector_results[3] > photoreflector_results[2]) {
          // servo_rotate_reverse_diff();
        }
        diff_count = 0;
      }
    } else {
      // diff安定状態の場合
      if (current_diff && pair_stable) {
        diff_lost_count = 0;
      } else {
        diff_lost_count++;
        if (diff_lost_count >= 5) { // 5回連続で条件を満たさない場合
          diff_stable = false;
          diff_count = 0;
          diff_lost_count = 0;
        }
      }
    }

  // Read raw accel/gyro burst and append values to CSV output.
  float ax = 0.0f, ay = 0.0f, az = 0.0f, gx = 0.0f, gy = 0.0f, gz = 0.0f;
  bool imu_ok = read_accel_gyro_burst(&ax, &ay, &az, &gx, &gy, &gz);

  //目標角度まで衛星を回転させるプログラムtrack.cpp
  track(phase_angle); // 例: 現在の方位から90度回転させる
  
  }
  return 0;
}