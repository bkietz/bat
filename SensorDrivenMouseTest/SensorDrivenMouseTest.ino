#include <Arduino.h>
#include <Wire.h>
#include <bluefruit.h>

#include "bat/Print.h"
#include "bat/NeoPixel.h"
#include "bat/BLE.h"
#include "bat/Eigen.h"

#include "bat/Orientation.h"
#include "bat/BeginEach.h"

#include <Adafruit_LSM6DS33.h>
#include <Adafruit_LIS3MDL.h>

Adafruit_LSM6DS33 lsm6ds33;
Adafruit_LIS3MDL lis3mdl;

struct {
  int setup_line;
  #define STATUS_LINE() Status.setup_line = __LINE__
  bool lsm6ds33 = false;
  bool lis3mdl = false;
} Status;

volatile bool disabled = false;

void setup() {
  STATUS_LINE();

  bat::begin_each(bat::print, bat::PIXEL, bat::BLE);
  STATUS_LINE();

  Status.lsm6ds33 = lsm6ds33.begin_I2C();
  STATUS_LINE();

  Status.lis3mdl = lis3mdl.begin_I2C();
  STATUS_LINE();

  lsm6ds33.setAccelDataRate(LSM6DS_RATE_208_HZ);
  lsm6ds33.setGyroDataRate(LSM6DS_RATE_208_HZ);
  lsm6ds33.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  //lsm6ds33.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
  lsm6ds33.setGyroRange(LSM6DS_GYRO_RANGE_125_DPS);
  lsm6ds33.highPassFilter(false, {});
  STATUS_LINE();

  lis3mdl.setDataRate(LIS3MDL_DATARATE_300_HZ);
  lis3mdl.setPerformanceMode(LIS3MDL_ULTRAHIGHMODE);
  STATUS_LINE();

  bat::PIXEL = { .red = 32, .green = 6 };
  STATUS_LINE();

  auto* con = Bluefruit.Connection(Bluefruit.connHandle());
  con->requestConnectionParameter(6);
  bat::BLE.HID->setKeyboardLedCallback([](uint16_t conn, uint8_t leds) {
    disabled = leds & 2; // Use caps lock for now
    if (Serial) {
      bat::print(" led update: ", leds);
    }
  });

  STATUS_LINE();
}

void first_print() {
  if (not Serial) return;

  static bool once = false;
  if (once) return;
  once = true;

  bat::print("sensor driven mouse test");
  bat::print("setup made it to line ", Status.setup_line);
  if (not Status.lsm6ds33) {
    bat::print("lsm6ds33 init failed");
  } else {
    switch (lsm6ds33.getGyroRange()) {
    case LSM6DS_GYRO_RANGE_125_DPS:
      bat::print("LSM6DS_GYRO_RANGE_125_DPS");
      break;
    case LSM6DS_GYRO_RANGE_250_DPS:
      bat::print("LSM6DS_GYRO_RANGE_250_DPS");
      break;
    case LSM6DS_GYRO_RANGE_500_DPS:
      bat::print("LSM6DS_GYRO_RANGE_500_DPS");
      break;
    case LSM6DS_GYRO_RANGE_1000_DPS:
      bat::print("LSM6DS_GYRO_RANGE_1000_DPS");
      break;
    case LSM6DS_GYRO_RANGE_2000_DPS:
      bat::print("LSM6DS_GYRO_RANGE_2000_DPS");
      break;
    }
  }

  if (not Status.lis3mdl) {
    bat::print("lis3mdl init failed");
  }

  Bluefruit.printInfo();
  Bluefruit.Periph.printInfo();
}

bool move2(int8_t x, int8_t y) {
  uint16_t size = sizeof(hid_mouse_report_t);
  hid_mouse_report_t report = { .x = x, .y = y };

  ble_gatts_hvx_params_t hvx_params = {
    .handle = bat::BLE.HID->mouse_characteristic().handles().value_handle,
    .type = BLE_GATT_HVX_NOTIFICATION,
    .offset = 0,
    .p_len = &size,
    .p_data = (uint8_t*)&report,
  };
  return sd_ble_gatts_hvx(Bluefruit.connHandle(), &hvx_params) == NRF_SUCCESS;
}

void loop() {
  first_print();

  auto* con = Bluefruit.Connection(Bluefruit.connHandle());
  if (con->getConnectionInterval() > 12) {
    if (Serial) {
      bat::print(" ... requesting faster connection, ", con->getConnectionInterval() * 1.25);
    }
    con->requestConnectionParameter(6);
  }

  static float x_sensitivity = 4000, y_sensitivity = 4000;
  if (Serial.available()) {
    int x = Serial.parseInt(), y = Serial.parseInt();

    if (x > 100 and y > 100) {
      if (Serial) {
        bat::print("Setting x,y sensitivity ", x, ", ", y);
      }
      x_sensitivity = x;
      y_sensitivity = y;
    }
  }

  Eigen::Vector3<float> acc, gyr;
  std::optional<Eigen::Vector3<float>> mag;
  sensors_event_t acc_e, gyro_e, temp_e, mag_e;
  lsm6ds33.getEvent(&acc_e, &gyro_e, &temp_e);
  acc = Eigen::Vector3<float>{ acc_e.acceleration.x, acc_e.acceleration.y, acc_e.acceleration.z };
  gyr = Eigen::Vector3<float>{ gyro_e.gyro.x, gyro_e.gyro.y, gyro_e.gyro.z };
  // lis3mdl.getEvent(&mag_e);
  // mag = Eigen::Vector3<float>{ mag_e.magnetic.x, mag_e.magnetic.y, mag_e.magnetic.z };
  
  // TODO try just accumulating rotations with no orientation tracking;
  // that might be usable and more predictable. I never want this to be
  // confused about the sign or pitch or yaw

  static Eigen::Quaternion<float> orientation{ 1, 0, 0, 0 };
  static float gyr_z = 0;

  const bat::Vector3<float> acc_offset{ 0.395, -0.193, 0.243 };
  const bat::Vector3<float> gyr_offset{ 0.113555, -0.098932, -0.117986 };

  static auto last_update = micros();
  bat::Vector3<float> gyr_step = (gyr - gyr_offset) * (micros() - last_update) / 1'000'000.F;
  update(&orientation, gyr_step);
  last_update = micros();

  correct(&orientation, acc - acc_offset, mag);

  gyr_z += gyr_step[2];

  if (disabled) return;

  static auto last_hid = millis();
  if (millis() - last_hid < 7) return;

  orientation.normalize();
  static auto last_orientation = orientation;
  Eigen::Vector3<float> delta = (orientation * last_orientation.conjugate()).vec();

  float total = asin(delta.norm()) * 2 * 180 / M_PI;
  delta = delta / delta.norm() * total;

  auto yaw = delta[2];

  Eigen::Vector3<float> z{0, 0, 1};
  auto pitch_axis = apply_rotation({0, 0, 1}, orientation);
  pitch_axis -= z.dot(pitch_axis) * pitch_axis;
  pitch_axis.normalize();

  auto pitch = delta.dot(pitch_axis);
  pitch = delta[0];
  pitch = gyr_z * 180 / M_PI;

  auto x = -yaw * x_sensitivity / 90;
  auto y = pitch * y_sensitivity / 90;
  for (auto* c : {&x, &y}) {
    if (*c > 127) *c = 127;
    if (*c < -127) *c = -127;
  }
  if (not move2(x, y)) return;

  last_orientation = orientation;
  gyr_z = 0;
}
