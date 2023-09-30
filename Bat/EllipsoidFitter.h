#pragma once

#include "eigen.h"
#include "assert.h"

class EllipsoidFitter {
public:
  void measure(Eigen::Vector3<float> xyz) { measure(xyz[0], xyz[1], xyz[2]); }

  void measure(float x, float y, float z) {
    float const z2 = z * z;
    Eigen::Vector<float, 9> const m{
        1.F, x, y, z, x * y, y * z, z * x, x * x - z2, y * y - z2,
    };
    lhs_ += (m * m.transpose()).triangularView<Eigen::Upper>();
    rhs_ -= z2 * m;
  }

  uint32_t count() const { return static_cast<uint32_t>(lhs_(0, 0)); }

  struct Solution {
    Eigen::Matrix3<float> semiaxes;
    Eigen::Vector3<float> center;
    // TODO: Solution::goodness(x, y, z)
  };

  // TODO: Fitter::condition()

  Solution const &solve() {
    lhs_.triangularView<Eigen::Lower | Eigen::ZeroDiag>() =
        lhs_.triangularView<Eigen::Upper | Eigen::ZeroDiag>().transpose();
    Eigen::FullPivLU<decltype(lhs_)> moment_solver{lhs_};
    Eigen::Vector<float, 9> const coefficients = moment_solver.solve(rhs_);

    auto const k1 = coefficients[0];
    auto const kx = coefficients[1];
    auto const ky = coefficients[2];
    auto const kz = coefficients[3];
    auto const kxy = coefficients[4];
    auto const kyz = coefficients[5];
    auto const kzx = coefficients[6];
    auto const kx2_minus_z2 = coefficients[7];
    auto const ky2_minus_z2 = coefficients[8];

    // translate these coefficients into a more natural formulation for an
    // ellipsoid:
    //     || A(x - c) ||^2 = 1
    // where c is the ellipsoid's center and A is a symmetric matrix encoding
    // the semiaxes.
    //
    //     || A(x - c) ||^2 = 1
    //     || AK(x - c) ||^2 = K^2
    //     transpose(AK * (x - c)) * (AK * (x - c)) = K^2
    //     transpose(x - c) * transpose(AK) * AK * (x - c) = K^2
    //     transpose(x - c) * AK * AK * (x - c) = K^2
    //     transpose(x - c) * AK^2 * (x - c) = K^2
    //     transpose(x) * AK^2 * x
    //       + transpose(x) * AK^2 * (-c)
    //       + transpose(-c) * AK^2 * x
    //       + transpose(-c) * AK^2 * (-c) = K^2
    //     transpose(x) * AK^2 * x                   // Quadratic terms
    //       + (-2 * transpose(c) * AK^2) * x        // Linear terms
    //       + transpose(c) * AK^2 * c - K^2 = 0     // Constant term

    // quadratic terms
    Eigen::Matrix3<float> AK2;
    AK2(0, 0) = kx2_minus_z2;
    AK2(1, 1) = ky2_minus_z2;
    AK2(2, 2) = 1 - kx2_minus_z2 - ky2_minus_z2;
    AK2(0, 1) = AK2(1, 0) = kxy / 2;
    AK2(1, 2) = AK2(2, 1) = kyz / 2;
    AK2(2, 0) = AK2(0, 2) = kzx / 2;

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3<float>> solver{AK2};
    auto const AK2_inv = solver.eigenvectors() *
                         solver.eigenvalues().cwiseInverse().asDiagonal() *
                         solver.eigenvectors().transpose();

    // linear terms
    Eigen::Vector3<float> const minus_2_cT_AK2{kx, ky, kz};
    Eigen::Vector3<float> const c = -0.5 * AK2_inv * minus_2_cT_AK2;

    // constant term
    float const K2 = c.transpose() * AK2 * c - k1;

    // Scale from AK^2 to A^2 and take the square root to get A
    Eigen::Vector3<float> const A2_eigenvalues = solver.eigenvalues() / K2;
    assert(A2_eigenvalues.minCoeff() > 0);

    auto const A = solver.eigenvectors() *
                   A2_eigenvalues.cwiseSqrt().asDiagonal() *
                   solver.eigenvectors().transpose();

    solution_.center = c;
    solution_.semiaxes = A;
    return solution_;
  }

private:
  Eigen::Matrix<float, 9, 9> lhs_ = Eigen::Matrix<float, 9, 9>::Zero();
  Eigen::Vector<float, 9> rhs_ = Eigen::Vector<float, 9>::Zero();
  Solution solution_;
};
