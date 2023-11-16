#pragma once

#ifdef ARDUINO
#include <ArduinoEigen.h>
#else
#include <eigen3/Eigen/Eigen>
#endif

namespace bat {

using Eigen::Vector;
using Eigen::Matrix;
using Eigen::Vector2;
using Eigen::Matrix2;
using Eigen::Vector3;
using Eigen::Matrix3;
using Eigen::Quaternion;

template <typename T>
using MatrixX = Eigen::MatrixX<T>;

} // namespace bat
