#include <iostream>
#include <vector>

#include <cxxopts.hpp>

#include "EllipsoidFitter.h"

Eigen::Vector3<float> random(float norm) {
  Eigen::Vector3<float> v;
  do {
    v = Eigen::Vector3<float>::Random();
  } while (v.squaredNorm() < 0.1);
  return v.normalized() * norm;
}

float angle(Eigen::Vector3<float> u, Eigen::Vector3<float> v) {
  return 180 / M_PI * std::acos(u.dot(v) / u.norm() / v.norm());
}

// FIXME what I'm really interested in here is the stability of a solution
// across multiple calibration passes; otherwise I won't see the consistent
// output from calibration passes.
void test_ellipsoid(int sample_count, float noise_level, int test_count) {
  auto noise = [&] { return random(noise_level); };

  auto offset = random(1);
  std::cout << "true offset:\n" << offset << std::endl;
  std::cout << "----------------------------------" << std::endl;

  std::cout << "true iron:" << std::endl;
  Eigen::Matrix3<float> not_sym = Eigen::Matrix3<float>::Random();
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3<float>> solver0{
      not_sym.transpose() + not_sym};

  // drag the random eigenvalues close-ish to 1,
  // which is what we expect for a soft iron matrix
  Eigen::Vector3<float> eigenvalues =
      (Eigen::Vector3<float>::Ones() + solver0.eigenvalues() / 4);

  Eigen::Matrix3<float> iron{solver0.eigenvectors() * //
                             eigenvalues.asDiagonal() *
                             solver0.eigenvectors().transpose()};

  std::cout << iron << std::endl;
  std::cout << "eigenvalues:\n" << eigenvalues << std::endl;
  std::cout << "eigenvectors:\n" << solver0.eigenvectors() << std::endl;

  Eigen::Matrix<float, 10, Eigen::Dynamic> measurements;
  measurements.resize(10, sample_count);

  EllipsoidFitter ellipsoid;
  while (sample_count-- != 0) {
    auto v = iron * random(1) + offset + noise();
    ellipsoid.measure(v);

    float x = v[0], y = v[1], z = v[2];
    measurements.col(sample_count) = Eigen::Vector<float, 10>{
        1.F, x, y, z, x * x, y * y, z * z, x * y, y * z, z * x,
    };
  }

  {
    using SolverType = Eigen::JacobiSVD<decltype(measurements)>;
    SolverType svd{measurements, Eigen::ComputeFullU};
    std::cout << "measurements condition: "
              << svd.singularValues().maxCoeff() /
                     svd.singularValues().minCoeff()
              << std::endl;
    std::cout << "minimum singular value: " << svd.singularValues().minCoeff()
              << std::endl;
  }

  auto [A, c] = ellipsoid.solve();
  std::cout << "----------------------------------" << std::endl;
  std::cout << "estimated offset:" << std::endl;
  std::cout << c << std::endl;
  std::cout << "----------------------------------" << std::endl;
  std::cout << "estimated iron:" << std::endl;
  Eigen::Matrix3<float> estimated_iron = A.inverse();
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3<float>> solver{estimated_iron};
  std::cout << estimated_iron << std::endl;
  std::cout << "eigenvalues:\n" << solver.eigenvalues() << std::endl;
  std::cout << "     delta: "
            << (solver.eigenvalues() - solver0.eigenvalues()).norm() /
                   solver0.eigenvalues().norm()
            << std::endl;
  std::cout << "eigenvectors:\n" << solver.eigenvectors() << std::endl;
  std::cout << "     delta: "
            << 1 - (solver.eigenvectors().transpose() * solver0.eigenvectors())
                           .trace() /
                       3
            << std::endl;

  std::cout << "----------------------------------" << std::endl;
  std::cout << "Using the estimated and true offset/iron to reconstruct\n";
  std::cout << "from measurements, assess the angular deficit in degrees\n";
  std::cout << "(the reconstruction from true offset/iron proivides a\n";
  std::cout << "baseline for the best possible given the noise)\n";

  while (test_count-- != 0) {
    Eigen::Vector3<float> v = Eigen::Vector3<float>::Random();
    Eigen::Vector3<float> measured = iron * v + offset + noise();

    Eigen::Vector3<float> estimated_v = A * (measured - c);
    Eigen::Vector3<float> reconstructed_v =
        iron.inverse() * (measured - offset);

    std::cout << angle(v, estimated_v) << "\t  vs\t";
    std::cout << angle(v, reconstructed_v) << std::endl;
  }
}

int main(int argc, char **argv) {
  cxxopts::Options options("CalibrationTest",
                           "Test performance of sphere/ellipsoid fitters");

  options.add_options()("h,help", "print usage")(
      "sample_count",
      "for sphere this is used to reduce noise through averaging, for ellipse "
      "this is the number of random samples",
      cxxopts::value<int>()->default_value("100"))(
      "test_count", "how many reconstructions should be tested",
      cxxopts::value<int>()->default_value("10"))(
      "seed", "random seed", cxxopts::value<int>()->default_value("0"))(
      "noise", "max noise level",
      cxxopts::value<float>()->default_value("0.1"));

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  std::srand(result["seed"].as<int>());

  auto noise_level = result["noise"].as<float>();
  int sample_count = result["sample_count"].as<int>();
  int test_count = result["test_count"].as<int>();

  test_ellipsoid(sample_count, noise_level, test_count);
  return 0;
}
