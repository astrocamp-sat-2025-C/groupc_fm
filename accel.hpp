#ifndef ACCEL_HPP
#define ACCEL_HPP

#include <pico/types.h>
#include <stdbool.h>

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for
// information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 20
#define I2C_SCL 21
#define I2C_ADDR 0x69
#define ACCEL_XOUT_H 0x1F

// Additional registers for init and FIFO/direct access (Bank 0)
#define DEVICE_CONFIG 0x11
#define INT_CONFIG 0x14
#define FIFO_CONFIG 0x16
#define TEMP_DATA1 0x1D
#define TEMP_DATA0 0x1E
#define FIFO_COUNTH 0x2E
#define FIFO_COUNTL 0x2F
#define FIFO_DATA 0x30
#define PWR_MGMT0 0x4E
#define FIFO_CONFIG1 0x5F
#define WHO_AM_I 0x75

// Function declarations
bool probe_who_am_i(uint8_t *who);
bool icm42688_init(void);
bool read_accel_gyro_burst(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx,
                           int16_t *gy, int16_t *gz);
void i2c_scan(void);

// Scaled IMU data (accel in g, gyro in dps)
typedef struct {
  float ax_g;
  float ay_g;
  float az_g;
  float gx_dps;
  float gy_dps;
  float gz_dps;
} ImuScaled;

// Read accel/gyro and return scaled values (g, dps). Returns false on I2C/IMU
// read failure.
bool read_imu_scaled(ImuScaled *out);

#endif // ACCEL_HPP
