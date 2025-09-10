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
