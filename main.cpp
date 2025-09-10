#include "accel.hpp" // commented out sensor usage below
#include "adc.hpp"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "servo.hpp"
#include <pico/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <utility>
#include <vector>

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
    // 数字のみ出力に統一するため文字列のプリントはしない
  }

  // サーボ初期化（方向探索・整列制御で使用）
  servo_init(SERVO_PIN);

  // 制御状態: (1) まず(2,3)の組がbest_pairになるまでsum(2,3)を最大化、(2)
  // 達成後は差を最小化
  enum class ControlState : uint8_t { SeekPair23Max = 0, AlignMinDiff = 1 };
  static ControlState state = ControlState::SeekPair23Max;
  // 単純なヒルクライム用の履歴と回転方向
  static float servo_dir = 1.0f;    // +1 / -1
  static int32_t last_sum23 = -1;   // 前回の s2+s3
  static int32_t last_diff23 = -1;  // 前回の |s2-s3|
  const float SEARCH_SPEED = 0.20f; // 探索時の回転速度（-1..1）
  const float ALIGN_SPEED = 0.12f;  // 整列時の回転速度（-1..1）

  while (true) {
    uint16_t photodiode_result;
    uint16_t photoreflector_results[4];

    photodiode_read(&photodiode_result);
    photoreflector_read(photoreflector_results);
    const std::vector<std::pair<int, int>> index_pairs = {
        {0, 1}, {1, 2}, {2, 3}, {3, 1}};

    // 最大の和とその時のインデックスペアを保存する変数
    uint32_t max_sum = 0; // 和を格納するため、より大きい型 (uint32_t) を使う
    std::pair<int, int> best_pair = {-1, -1}; // 見つかった最適なペアを保存

    // 各ペアをループして最大の和を探す
    for (const auto &p : index_pairs) {
      int index1 = p.first;
      int index2 = p.second;

      // 現在のペアの和を計算
      uint32_t current_sum =
          photoreflector_results[index1] + photoreflector_results[index2];

      // これまでに見つかった最大の和 (max_sum) より大きいかチェック
      // best_pair.first == -1 は最初の1回目のチェック
      if (best_pair.first == -1 || current_sum > max_sum) {
        max_sum = current_sum; // 最大値を更新
        best_pair = p;         // 最適なペアを更新
      }
    }

    // (2,3)の和・差・2%判定フラグを計算
    const int32_t s2 = photoreflector_results[2];
    const int32_t s3 = photoreflector_results[3];
    const int32_t sum23 = s2 + s3;
    const int32_t diff23 = (s2 > s3) ? (s2 - s3) : (s3 - s2);
    const int32_t threshold = (sum23 * 10) / 100; // 2%
    const bool flag_23_within_2pct =
        (sum23 == 0) ? (diff23 == 0) : (diff23 <= threshold);

    // 状態に応じたサーボ制御
    if (state == ControlState::SeekPair23Max) {
      // すでに(2,3)がbest_pairになっていれば次の段階へ
      if (best_pair.first == 2 && best_pair.second == 3) {
        state = ControlState::AlignMinDiff;
        last_diff23 = -1; // 次ステージの履歴をリセット
      } else {
        // sum(2,3) をヒルクライムで増やす: 減ったら回転方向を反転
        if (last_sum23 >= 0) {
          if (sum23 < last_sum23)
            servo_dir = -servo_dir;
        }
        last_sum23 = sum23;
        servo_set_speed(SERVO_PIN, servo_dir * SEARCH_SPEED);
      }
    } else { // AlignMinDiff
      // (2,3)がbest_pairから外れたら再度探索に戻る（任意のフォールバック）
      if (!(best_pair.first == 2 && best_pair.second == 3)) {
        state = ControlState::SeekPair23Max;
        last_sum23 = -1;
      } else if (flag_23_within_2pct) {
        // 条件達成: 2%以内なら停止
        servo_set_speed(SERVO_PIN, 0.0f);
      } else {
        // |s2 - s3| を最小化: 増えたら回転方向を反転
        if (last_diff23 >= 0) {
          if (diff23 > last_diff23)
            servo_dir = -servo_dir;
        }
        last_diff23 = diff23;
        servo_set_speed(SERVO_PIN, servo_dir * ALIGN_SPEED);
      }
    }

    int16_t ax, ay, az, gx, gy, gz;
    if (read_accel_gyro_burst(&ax, &ay, &az, &gx, &gy, &gz)) {
      // Convert raw counts to physical units using configured sensitivity
      float ax_g = (float)ax / ACCEL_SENSITIVITY;
      float ay_g = (float)ay / ACCEL_SENSITIVITY;
      float az_g = (float)az / ACCEL_SENSITIVITY;

      float gx_dps = (float)gx / GYRO_SENSITIVITY;
      float gy_dps = (float)gy / GYRO_SENSITIVITY;
      float gz_dps = (float)gz / GYRO_SENSITIVITY;

      printf(
          "%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%u,%ld,%ld,%d,%d\n",
          photoreflector_results[0], photoreflector_results[1],
          photoreflector_results[2], photoreflector_results[3], ax_g, ay_g,
          az_g, gx_dps, gy_dps, gz_dps, best_pair.first, best_pair.second,
          (unsigned)max_sum, (long)sum23, (long)diff23,
          flag_23_within_2pct ? 1 : 0,
          (state == ControlState::SeekPair23Max) ? 0 : 1);
      // 更新周期（制御応答用に短め）
      sleep_ms(1000);
    } else {
      // IMU失敗時も数値のみ・同じ列構成で出力（IMU値は0）
      printf(
          "%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%u,%ld,%ld,%d,%d\n",
          photoreflector_results[0], photoreflector_results[1],
          photoreflector_results[2], photoreflector_results[3], 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, best_pair.first, best_pair.second,
          (unsigned)max_sum, (long)sum23, (long)diff23,
          flag_23_within_2pct ? 1 : 0,
          (state == ControlState::SeekPair23Max) ? 0 : 1);
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