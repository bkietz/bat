#include <Arduino.h>
#include <Wire.h>
#include "bat/Print.h"
#include "bat/NeoPixel.h"
#include "bat/Eigen.h"

#include "bat/Orientation.h"
#include "bat/BeginEach.h"

#include <Adafruit_LSM6DS33.h>
#include <Adafruit_LIS3MDL.h>

Adafruit_LSM6DS33 lsm6ds33;
Adafruit_LIS3MDL lis3mdl;

void setup() {
  bat::begin_each(bat::print, bat::PIXEL);
  while (!Serial) {
    delay(5);
  }
  bat::print("sensor test");

  if (!lsm6ds33.begin_I2C()) {
   Serial.println("Failed to find LSM6DS33 chip");
    Serial.flush();
    while (1) {
      delay(10);
    }
  }
  if (!lis3mdl.begin_I2C()) {
    Serial.println("Failed to find LIS3MDL chip");
    Serial.flush();
    while (1) {
      delay(10);
    }
  }
  lsm6ds33.setAccelDataRate(LSM6DS_RATE_833_HZ);
  lsm6ds33.setGyroDataRate(LSM6DS_RATE_833_HZ);
  lsm6ds33.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  lsm6ds33.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);

  lis3mdl.setDataRate(LIS3MDL_DATARATE_300_HZ);
  lis3mdl.setPerformanceMode(LIS3MDL_ULTRAHIGHMODE);

  bat::print(" ... finished sensor init");
  bat::PIXEL = { .red = 32, .green = 6 };
}

void loop() {
  Eigen::Vector3<float> acc, gyr, mag;
  sensors_event_t acc_e, gyro_e, temp_e, mag_e;
  lsm6ds33.getEvent(&acc_e, &gyro_e, &temp_e);
  acc = Eigen::Vector3<float>{ acc_e.acceleration.x, acc_e.acceleration.y, acc_e.acceleration.z };
  gyr = Eigen::Vector3<float>{ gyro_e.gyro.x, gyro_e.gyro.y, gyro_e.gyro.z };

  static Eigen::Quaternion<float> orientation{ -1, 0, 0, 0 };

  const bat::Vector3<float> acc_offset{ 0.395, -0.193, 0.243 };
  const bat::Vector3<float> gyr_offset{ 0.113555, -0.098932, -0.117986 };

  static auto last_update = micros();
  update(&orientation, (gyr - gyr_offset) * (micros() - last_update) / 1'000'000.F);
  last_update = micros();

  static uint32_t update_count = 0;
  ++update_count;

  if (update_count % 10) {
    correct(&orientation, acc - acc_offset, {});
  } else {
    lis3mdl.getEvent(&mag_e);
    mag = Eigen::Vector3<float>{ mag_e.magnetic.x, mag_e.magnetic.y, mag_e.magnetic.z };
    correct(&orientation, acc - acc_offset, mag);
  }

  static auto last_print = millis();
  if (millis() - last_print > 1000) {
    Serial.printf("@%d: #%d\n", millis(), update_count);
    last_print = millis();
    update_count = 0;

    auto rotation_of = [](Eigen::Vector3<float> unit, auto q) {
      return acos(apply_rotation(unit, q).dot(unit)) * 180 / M_PI;
    };
    Serial.printf("acc: [\t%+f\t%+f\t%+f]\n", acc[0], acc[1], acc[2]);
    Serial.printf("attitude: %f°\n", rotation_of({0, 0, 1}, orientation));

    orientation.normalize();
    static auto last_orientation = orientation;
    Eigen::Vector3<float> delta = (orientation * last_orientation.conjugate()).vec();
    last_orientation = orientation;

    float total = asin(delta.norm()) * 2 * 180 / M_PI;
    delta = delta / delta.norm() * total;

    auto yaw = delta[2];
    auto pitch = delta[0] - delta[1];
    Serial.printf("total, yaw, pitch: %f°\t%+f°\t%+f°\n", total, yaw, pitch);
    Serial.println();
  }
}
