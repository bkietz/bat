#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <iostream>
#endif

namespace bat {

#ifdef ARDUINO
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
#else  // ARDUINO
template <typename... A> void print(A const &...args) {
  (std::cout << ... << args) << std::endl;
}
#endif // ARDUINO

} // namespace bat
