#pragma once

#include <Arduino.h>

namespace bat {

struct {
  void begin() const {
    Serial.begin(115200);
    while (!Serial) {
      delay(5);
    }
  }

  template <typename... A> void operator()(A const &...args) const {
    (Serial.print(args), ...);
    Serial.println();
    Serial.flush();
  }
} constexpr print;

} // namespace bat
