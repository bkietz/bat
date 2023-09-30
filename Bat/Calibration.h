#pragma once

#include "eigen.h"
#include "sensors.h"

#include "Adafruit_NeoPixel.h"
#include "Adafruit_USBD_CDC.h"

// The calibration namespace contains variables which
// can be used to correct raw readings from the sensors:
//
//   rawAccelGyro().first - calibration::acc_offset
//   rawAccelGyro().second - calibration::gyro_offset
//   calibration::mag_semiaxes * (rawMag() - calibration::mag_offset)
//
// Calibration is triggered by a button sequence.
// The precise button sequence and logic for entering
// calibration should not live here. However, we should
// expose a function for loop() to call when calibration
// is requested:
//
//     void loop() {
//         // ...
//         if (need_calibration) return calibration::start();
//         // ...
//     }
//
// While calibrating, loop() should call calibration::loop()
// like so:
//
//     void loop() {
//       if (calibration::loop()) return;
//     }
//
// calibration::loop() returns true if calibration is happening
// and the rest of the loop should be skipped.
//
// We also want ::load and ::save functions instead of hard coding
// values. It might also be good to have ::check, which measures
// how well calibrated you are.
namespace calibration {

void start();
bool loop();

enum class State : char {
  HoldStill0,
  FlipOver,
  HoldStill180,
  MoveAround,

  NotRunning,
} state;

Adafruit_NeoPixel* pixel;

Eigen::Vector3<float> acc_sum, gyro_sum;
float acc_sum2, gyro_sum2;
int count;

Eigen::Vector3<float> acc_mean_0, acc_mean_180;
Eigen::Vector3<float> acc_offset{ 0.3947125, -0.193089, 0.2429600 };
Eigen::Vector3<float> gyro_offset{ 0.111325, -0.099864, -0.142607 };
float gyro_noise = 0.013386, acc_noise = 0.01;

Eigen::Matrix3<float> mag_semiaxes;
Eigen::Vector3<float> mag_offset;

enum class Status : char {
  Ongoing,
  Stabilized,
  Noisy,
};

Status _measure_acc_gyro() {
  auto [acc, gyro] = rawAccelGyro();

  acc_sum += acc;
  acc_sum2 += acc.squaredNorm();

  gyro_sum += gyro;
  gyro_sum2 += gyro.squaredNorm();

  ++count;

  if (count % 128) return Status::Ongoing;

  auto& acc_mean = state == State::HoldStill0 ? acc_mean_0 : acc_mean_180;

  acc_mean = acc_sum / count;
  float acc_mean2 = acc_mean.squaredNorm();
  float acc_variance = acc_sum2 / count - acc_mean2;
  acc_noise = sqrt(acc_variance);

  gyro_offset = gyro_sum / count;
  float gyro_offset2 = gyro_offset.squaredNorm();
  float gyro_variance = gyro_sum2 / count - gyro_offset2;
  gyro_noise = sqrt(gyro_variance);

  float noise_ratio = sqrt(acc_variance / acc_mean2
                           + gyro_variance / gyro_offset2);

  // if noise_ratio is below 5% we can proceed to the next step
  if (noise_ratio < 0.05) return Status::Stabilized;

  // if noise_ratio is above 30% start this step over
  if (noise_ratio > 0.3) return Status::Noisy;

  return Status::Ongoing;
}

EllipsoidFitter ellipsoid_fitter;

Status _measure_mag() {
  ellipsoid_fitter.measure(rawMag());

  if (count % 128) return Status::Ongoing;

  auto const& soln = ellipsoid_fitter.solve();
  mag_semiaxes = soln.semiaxes;
  mag_offset = soln.center;
  return Status::Stabilized;
}

void _goto(State s) {
  state = s;

  switch (state) {
    case State::HoldStill0:
    case State::FlipOver:
    case State::HoldStill180:
      acc_sum = gyro_sum = { 0, 0, 0 };
      acc_sum2 = gyro_sum2 = 0;
      count = 0;
      break;
    case State::MoveAround:
      ellipsoid_fitter = {};
      break;
    default:
    case State::NotRunning:
      // Finished!
      break;
  }

  if (pixel) {
    pixel->setPixelColor(0,
                         [&] {
                           switch (state) {
                             case State::HoldStill0:
                               return pixel->Color(32, 0, 0);
                             case State::FlipOver:
                               return pixel->Color(16, 16, 0);
                             case State::HoldStill180:
                               return pixel->Color(32, 0, 0);
                             case State::MoveAround:
                               return pixel->Color(0, 32, 0);
                             default:
                             case State::NotRunning:
                               return pixel->Color(0, 0, 0);
                           }
                         }());
    pixel->show();
  }
}

void _goto_next() {
  auto next_state = static_cast<State>(static_cast<char>(state) + 1);
  _goto(next_state);
}

void _goto_restart_current() {
  _goto(state);
}

void start(Adafruit_NeoPixel* p = nullptr) {
  pixel = p;
  _goto(State::HoldStill0);
}

bool loop() {
  if (state == State::NotRunning) return false;

  auto& measure = state == State::MoveAround
                    ? _measure_mag
                    : _measure_acc_gyro;

  switch (measure()) {
    case Status::Ongoing:
      break;

    case Status::Noisy:
      _goto_restart_current();
      break;

    case Status::Stabilized:
      if (state == State::HoldStill180) {
        // Since we've held still twice, we
        // can take the difference of the two
        // means and compute the accelerometer offset
        acc_offset = (acc_mean_0 - acc_mean_180) / 2;
      }
      _goto_next();
      break;
  }
  return true;
}

}  // namespace calibration
