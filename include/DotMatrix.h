#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace bat {

struct DotMatrix {
  static constexpr size_t CHARACTER_WIDTH = 72;
  static constexpr size_t CHARACTER_HEIGHT = 36;
  static constexpr size_t WIDTH = CHARACTER_WIDTH * 2;
  static constexpr size_t HEIGHT = CHARACTER_HEIGHT * 3;

  char lines[CHARACTER_HEIGHT][CHARACTER_WIDTH * 3];

  void reset() {
    for (auto &line : lines) {
      constexpr uint8_t BASE[] = {0xe2, 0xa0, 0x80};
      for (size_t i = 0; i != sizeof(line) / sizeof(BASE); ++i) {
        memcpy(line + i * sizeof(BASE), &BASE, sizeof(BASE));
      }
    }
  }

  void set(float x, float y) {
    static float const ALMOST_1 = std::nextafter(1.F, 0.F);
    int ix = ALMOST_1 * x * WIDTH;
    int iy = ALMOST_1 * y * HEIGHT;
    if (ix < 0 or ix >= WIDTH or iy < 0 or iy >= HEIGHT) return;

    static constexpr char MASK[3][2] = {
        {1, 8 },
        {2, 16},
        {4, 32},
    };
    lines[iy / 3][(ix / 2) * 3 + 2] |= MASK[iy % 3][ix % 2];
  }

  friend inline std::ostream &operator<<(std::ostream &os, DotMatrix const &m) {
    char horizontal_rule[1 + CHARACTER_WIDTH + 2];
    memset(&horizontal_rule, '-', sizeof(horizontal_rule));
    horizontal_rule[0] = '\n';

    os.write(horizontal_rule, sizeof(horizontal_rule));

    for (auto const &line : m.lines) {
      os << "\n|";
      os.write(line, sizeof(line));
      os << '|';
    }

    os.write(horizontal_rule, sizeof(horizontal_rule));
    return os << '\n';
  }
};

}  // namespace bat
