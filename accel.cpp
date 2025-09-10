#include "accel.hpp"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

// I2C helpers
static uint8_t addr = I2C_ADDR;

static bool i2c_write_reg(uint8_t reg, uint8_t val) {
  uint8_t buf[2] = {reg, val};
  int w = i2c_write_blocking(I2C_PORT, addr, buf, 2, false);
  return w == 2;
}

static bool i2c_read_regs(uint8_t reg, uint8_t *buf, int len) {
  int w = i2c_write_blocking(I2C_PORT, addr, &reg, 1, false);
  if (w != 1)
    return false;
  int r = i2c_read_blocking(I2C_PORT, addr, buf, len, false);
  return r == len;
}

bool probe_who_am_i(uint8_t *who) {
  uint8_t reg = WHO_AM_I;
  int w = i2c_write_blocking(I2C_PORT, addr, &reg, 1, true);
  if (w != 1)
    return false;
  uint8_t val = 0x00;
  int r = i2c_read_blocking(I2C_PORT, addr, &val, 1, false);
  if (r == 1) {
    addr = addr;
    if (who)
      *who = val;
    return true;
  }
  return false;
}

bool icm42688_init(void) {
  uint8_t who = 0;
  if (!probe_who_am_i(&who))
    return false;
  printf("WHO_AM_I=0x%02X at I2C 0x%02X\n", who, addr);
  if (who != 0x47) {
    return false;
  }

  // Enable accel & gyro in Low Noise (LN) mode
  if (!i2c_write_reg(PWR_MGMT0, 0x0F))
    return false;
  sleep_ms(1);

  return true;
}

bool read_accel_gyro_burst(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx,
                           int16_t *gy, int16_t *gz) {
  uint8_t buf[12];
  if (!i2c_read_regs(ACCEL_XOUT_H, buf, 12))
    return false;
  *ax = (int16_t)((buf[0] << 8) | buf[1]);
  *ay = (int16_t)((buf[2] << 8) | buf[3]);
  *az = (int16_t)((buf[4] << 8) | buf[5]);
  *gx = (int16_t)((buf[6] << 8) | buf[7]);
  *gy = (int16_t)((buf[8] << 8) | buf[9]);
  *gz = (int16_t)((buf[10] << 8) | buf[11]);
  return true;
}

// IMU scaling: match sensor full-scale ranges (keep consistent with
// configuration)
static constexpr float ACCEL_FS_RANGE = 16.0f;  // ±16 g
static constexpr float GYRO_FS_RANGE = 2000.0f; // ±2000 dps
static constexpr float ACCEL_SENSITIVITY =
    32768.0f / ACCEL_FS_RANGE; // counts per g
static constexpr float GYRO_SENSITIVITY =
    32768.0f / GYRO_FS_RANGE; // counts per dps

bool read_imu_scaled(ImuScaled *out) {
  if (!out)
    return false;
  int16_t ax, ay, az, gx, gy, gz;
  if (!read_accel_gyro_burst(&ax, &ay, &az, &gx, &gy, &gz))
    return false;

  out->ax_g = (float)ax / ACCEL_SENSITIVITY;
  out->ay_g = (float)ay / ACCEL_SENSITIVITY;
  out->az_g = (float)az / ACCEL_SENSITIVITY;
  out->gx_dps = (float)gx / GYRO_SENSITIVITY;
  out->gy_dps = (float)gy / GYRO_SENSITIVITY;
  out->gz_dps = (float)gz / GYRO_SENSITIVITY;
  return true;
}

// Simple I2C bus scanner: probe 7-bit addresses 0x01..0x7E and print responding
// addresses.
void i2c_scan(void) {
  printf("I2C scan start\n");
  for (uint8_t a = 1; a < 127; ++a) {
    uint8_t buf[1] = {0};
    // send zero-length write to probe address; check return >= 0 (success/ack)
    int r = i2c_write_blocking(I2C_PORT, a, buf, 1, false);
    int r2 = i2c_read_blocking(I2C_PORT, a, buf, 1, false);
    if (r > 0 && r2 > 0) {
      printf("found device at 0x%02X\n", a);
    }
    // if(r > 0){
    //     printf("found device at 0x%02X\n", a);
    // }
  }
  printf("I2C scan done\n");
}
