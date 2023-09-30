#pragma once

#include <nrf_sdm.h>
#include <nrf_soc.h>

#include "eigen.h"

constexpr float kMagnetometerFullScale = 4;  // Gauss
constexpr float kMagnetometerSensitivity =
  LIS3MDL_MAG_SENSITIVITY_FOR_FS_4G;

LIS3MDLSensor Mag(&Wire);

Eigen::Vector3<float> rawMag() {
  std::array<int16_t, 3> buf;
  if (Mag.GetAxesRaw(buf.data()) != 0) {
    Serial.println("Error reading magnetometer data");
    return { 0, 0, 0 };
  }
  return {
    kMagnetometerSensitivity * buf[0],
    kMagnetometerSensitivity * buf[1],
    kMagnetometerSensitivity * buf[2]
  };
}

constexpr float kAccelerometerFullScale = 2;
constexpr float kAccelerometerSensitivity =
  // (10^3 gravity / LSB) * (meter second^-2 / gravity) / (10^3)
  //    -> (meter second^-2 / LSB)
  LSM6DS3_ACC_SENSITIVITY_FOR_FS_2G * 9.81 / 1000;

constexpr float kGyroFullScale = 1000;
constexpr float kGyroSensitivity =
  // (10^-3 degree second^-1 / LSB) * (radian / degree) * (10^3)
  //    -> (radian second^-1 / LSB)
  LSM6DS3_GYRO_SENSITIVITY_FOR_FS_1000DPS * (PI / 180) / 1000;

LSM6DS3Sensor AccGyr(&Wire, LSM6DS3_ACC_GYRO_I2C_ADDRESS_LOW);

/// Returns accelerometer readings
/// - Accelerometer in units of meters per second squared
/// - Gyro in units of radians per second
std::pair<Eigen::Vector3<float>, Eigen::Vector3<float>> rawAccelGyro() {
  std::array<int16_t, 6> buf;

  if (AccGyr.IO_Read(reinterpret_cast<uint8_t*>(buf.data()), LSM6DS3_ACC_GYRO_OUTX_L_G, sizeof(buf)) != 0) {
    Serial.println("Error reading accelerometer/gyro data");
    return { { 0, 0, 0 }, { 0, 0, 0 } };
  }

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  // The LSM6DS3's registers store these integers in little endian order,
  // so if that isn't the processor's native ordering then we'll need swap
  // the bytes before we use the integers.
  for (int16_t& i : buf) {
    i = i << 8 | i >> 8;
  }
#endif

  Eigen::Vector3<float> gyro{
    kGyroSensitivity * buf[0],
    kGyroSensitivity * buf[1],
    kGyroSensitivity * buf[2]
  };
  Eigen::Vector3<float> acc{
    kAccelerometerSensitivity * buf[3],
    kAccelerometerSensitivity * buf[4],
    kAccelerometerSensitivity * buf[5]
  };
  return { acc, gyro };
}
