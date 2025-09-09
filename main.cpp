#include <pico/types.h>
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "diode.hpp"
#include "accel.hpp"

int main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    i2c_scan();
    photoreflector_init();

    if (!icm42688_init())
    {
        printf("ICM-42688-P init failed\n");
    }

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
    }
    return 0;
}