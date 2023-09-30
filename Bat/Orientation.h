#pragma once

#include <optional>

#include "Eigen.h"

constexpr float kDistrust = 20;

Eigen::Quaternion<float> small(Eigen::Vector3<float> axis) {
  // return a quaternion which rotates about the axis;
  // this is a reasonable approximation for small axis.norm()
  return Eigen::Quaternion<float>{ 1, axis[0] / 2, axis[1] / 2, axis[2] / 2 };
}

void update(Eigen::Quaternion<float>* orientation,
            Eigen::Vector3<float> axis) {
  // incorporate the rotation measured by the gyro
  *orientation = *orientation * small(axis);
}

void correct(Eigen::Quaternion<float>* orientation,
             Eigen::Vector3<float> acceleration,
             std::optional<Eigen::Vector3<float>> magnetic) {
  auto matrix = orientation->toRotationMatrix();

  acceleration.normalize();
  auto accelerationExpected = matrix.col(2);
  auto accelerationCorrection = acceleration.cross(accelerationExpected);
  Eigen::Vector3<float> correction = accelerationCorrection / kDistrust;

  if (magnetic) {
    magnetic->normalize();
    auto magnetic_z = accelerationExpected.dot(*magnetic);
    auto magnetic_x = sqrt(1 - magnetic_z * magnetic_z);
    auto magneticExpected = magnetic_x * matrix.col(0) + magnetic_z * matrix.col(2);
    auto magneticCorrection = magnetic->cross(magneticExpected);
    correction += magneticCorrection / kDistrust;
  }

  *orientation = *orientation * small(correction);
}
