//AI新提案
#include "MadgwickAHRS.h"
#include "accel.hpp" // IMU API
#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.hpp"
#include "hardware/pwm.h"
#include <cmath>
#include "track.hpp"

void track(float delta_yaw) {
    const float margin = 2.0f;
    const int target_count = 5;
    int count = 0;

    const float DEG2RAD = 3.14159265f / 180.0f;

    // 初期ヨー角の取得
    ImuScaled imu;
    if (!read_imu_scaled(&imu)) return;
    MadgwickAHRSupdateIMU(imu.gx_dps * DEG2RAD, imu.gy_dps * DEG2RAD, imu.gz_dps * DEG2RAD,
                          imu.ax_g, imu.ay_g, imu.az_g);
    float yaw, pitch, roll;
    MadgwickGetEuler(&yaw, &pitch, &roll);
    float yaw_0 = yaw * (180.0f / 3.14159265f) * 6.0f;
    if(yaw_0 < 0){
        yaw_0 += 360.0f;
    }
    float target = yaw_0 + delta_yaw;
    target = fmod(target, 360.0f); // 0-360度に正規化
    printf("target%f_n", target);

    while (count < target_count) {
        // 毎ループでIMU更新
        if (read_imu_scaled(&imu)) {
            MadgwickAHRSupdateIMU(imu.gx_dps * DEG2RAD, imu.gy_dps * DEG2RAD, imu.gz_dps * DEG2RAD,
                                  imu.ax_g, imu.ay_g, imu.az_g);
            MadgwickGetEuler(&yaw, &pitch, &roll);
            float current_yaw = yaw * (180.0f / 3.14159265f) * 6.0f;
            if(current_yaw < 0){
                current_yaw += 360.0f;
            }       
            printf("yaw_deg = %f\n", current_yaw);

            // 目標範囲に入ったらカウント
            if (fabs(current_yaw - target) <= margin) {
                count++;
                printf("I found you ! count: %d\n", count);
            }

            // 回転方向の判定
            if (current_yaw < target - margin) {
                servo_rotate_reverse();
                printf("still --");
            } else if (current_yaw > target + margin) {
                // servo_rotate_forward();
                printf("still ++");
            } else {
                servo_stop();
            }
        }

    }

    servo_stop(); // 最終停止
}


