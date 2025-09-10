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
    float yaw_0 = yaw * (180.0f / 3.14159265f) * 3.0f;
    if(yaw_0 < 0){
        yaw_0 += 360.0f;
    }
    float target = yaw_0 + delta_yaw;

    while (count < target_count) {
        // 毎ループでIMU更新
        if (read_imu_scaled(&imu)) {
            MadgwickAHRSupdateIMU(imu.gx_dps * DEG2RAD, imu.gy_dps * DEG2RAD, imu.gz_dps * DEG2RAD,
                                  imu.ax_g, imu.ay_g, imu.az_g);
            MadgwickGetEuler(&yaw, &pitch, &roll);
            float current_yaw = yaw * (180.0f / 3.14159265f) * 3.0f;
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
                // servo_rotate_reverse();
                printf("still --");
            } else if (current_yaw > target + margin) {
                // servo_rotate_forward();
                printf("still ++");
            } else {
                servo_stop();
            }
        }

        sleep_ms(10); // 更新間隔を調整（10ms程度）
    }

    servo_stop(); // 最終停止
}


/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.hpp"
#include "hardware/pwm.h"
#include <cmath>
#include "track.hpp"

//Sun detector の制御の肝はカウントのさせ方にあると思う
//実装してより精度の出るプログラムに書き換えたいがとりあえずのバージョン

void track(float delta_yaw) {
    const float margin = 2.0f;
    const int target_count = 5;
    int count = 0;

    float yaw_0 = *g_yaw_deg * 3;  //yaw_deg基準点の設定
    float target = yaw_0 + delta_yaw;

    while (count < target_count) {
        float current_yaw = *g_yaw_deg * 3; // 毎ループで更新

        // 目標範囲に入ったらカウント
        if (fabs(current_yaw - target) <= margin) {
            count++;
            printf("I found you ! count: %d\n", count);
        }

        // 回転方向の判定
        if (current_yaw < target - margin) {
            // servo_rotate_forward();
        } else if (current_yaw > target + margin) {
            // servo_rotate_reverse();
        } else {
            servo_stop(); // 範囲内なら停止
        }
    }

    servo_stop(); // 最終停止
}
*/