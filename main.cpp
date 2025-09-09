#include <pico/types.h>
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "diode.hpp"
#include "accel.hpp" // commented out sensor usage below
#include "servo.hpp"

#define SERVO_PIN 11

int main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    i2c_scan();
    photoreflector_init();

    // icm42688_init() and 6-axis sensor initialization commented out
    // if (!icm42688_init())
    // {
    //     printf("ICM-42688-P init failed\n");
    // }

    while (true)
    {
        uint16_t photodiode_result;
        uint16_t photoreflector_results[4];

        photodiode_read(&photodiode_result);
        photoreflector_read(photoreflector_results);

        printf("Photodiode: %d\n", photodiode_result);
        printf("Photoreflector: %d %d %d %d\n",
               photoreflector_results[0], photoreflector_results[1],
               photoreflector_results[2], photoreflector_results[3]);

        sleep_ms(1000);
        /* 6-axis sensor readout commented out
        int16_t ax, ay, az, gx, gy, gz;
        if (read_accel_gyro_burst(&ax, &ay, &az, &gx, &gy, &gz))
        {
            // convert raw int16 readings to float and print as floats
            // Note: these are raw counts; divide by the sensor's sensitivity
            // if you want units like g or deg/s (sensitivity depends on configured range).
            float ax_f = (float)ax;
            float ay_f = (float)ay;
            float az_f = (float)az;
            float gx_f = (float)gx;
            float gy_f = (float)gy;
            float gz_f = (float)gz;
            printf("ACC: %8.3f %8.3f %8.3f  GYR: %8.3f %8.3f %8.3f\n",
                   ax_f, ay_f, az_f, gx_f, gy_f, gz_f);
        }
        else
        {
            printf("read_accel_gyro_burst failed\n");
            uint8_t who = 0;
            if (probe_who_am_i(&who))
                printf("WHO_AM_I=0x%02X at I2C 0x%02X\n", who, I2C_ADDR);
        }
        sleep_ms(100);
        */
    // --- Begin requested servo sequence ------------------------------------------------
    // Initialize servo pin (safe to call repeatedly)
    servo_init(SERVO_PIN);

    servo_set_speed(SERVO_PIN, 1.0);   // 正転
    printf("FORWARD\n");
    sleep_ms(400);

    servo_set_speed(SERVO_PIN, -1.0);  // 正転補正
    printf("REVERSE\n");
    sleep_ms(30);
        
    servo_set_speed(SERVO_PIN, 0.0);   // 停止
    printf("STOP1\n");
    sleep_ms(1500);

    servo_set_speed(SERVO_PIN, 1.0);   // 正転
    printf("FORWARD\n");
    sleep_ms(400);

    servo_set_speed(SERVO_PIN, 0.0);   // 停止
    printf("STOP1\n");
    sleep_ms(1000);

    servo_set_speed(SERVO_PIN, -1.0);  // 逆転
    printf("REVERSE\n");
    sleep_ms(400);

    servo_set_speed(SERVO_PIN, 1.0);   // 逆転補正
    printf("FORWARD\n");
    sleep_ms(30);

    servo_set_speed(SERVO_PIN, 0.0);   // 停止
    printf("STOP2\n");
    sleep_ms(1000);
    // --- End requested servo sequence --------------------------------------------------
    }
    return 0;
}