#pragma once

#include <optional>

#include "Eigen.h"

Eigen::Quaternion<float> small_rotation(Eigen::Vector3<float> axis) {
  // return a quaternion which rotates about the axis;
  // this is a reasonable approximation for small axis.norm()
  return Eigen::Quaternion<float>{1, axis[0] / 2, axis[1] / 2, axis[2] / 2};
}

// Note that orientation doesn't need to be normalized here
Eigen::Vector3<float> apply_rotation(Eigen::Vector3<float> v,
                                     Eigen::Quaternion<float> orientation) {
  Eigen::Quaternion<float> vq{0, v[0], v[1], v[2]};
  return (orientation.inverse() * vq * orientation).vec();
}

void update(Eigen::Quaternion<float> *orientation, Eigen::Vector3<float> axis) {
  // incorporate the rotation measured by the gyro
  *orientation = *orientation * small_rotation(axis);
}

void correct(Eigen::Quaternion<float> *orientation, Eigen::Vector3<float> acceleration,
             std::optional<Eigen::Vector3<float>> magnetic) {
  constexpr float kAccelerationDistrust = 200;

  acceleration.normalize();
  auto accelerationExpected = apply_rotation({0, 0, 1}, *orientation);
  auto accelerationCorrection = acceleration.cross(accelerationExpected);
  Eigen::Vector3<float> correction = accelerationCorrection / kAccelerationDistrust;

  if (magnetic) {
    constexpr float kMagneticDistrust = 2000;

    magnetic->normalize();
    auto magnetic_z = accelerationExpected.dot(*magnetic);
    auto magnetic_x = sqrt(1 - magnetic_z * magnetic_z);

    auto magneticExpected = magnetic_x * apply_rotation({0, 0, 1}, *orientation)
                          + magnetic_z * accelerationExpected;
    auto magneticCorrection = magnetic->cross(magneticExpected);
    correction += magneticCorrection / kMagneticDistrust;
  }

  *orientation = *orientation * small_rotation(correction);
}
