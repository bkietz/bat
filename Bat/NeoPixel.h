#pragma once

#include "Singleton.h"
#include <Adafruit_NeoPixel.h>
#include <cstdint>

namespace bat {

/// Stripped down API for the Feather Sense, where we just have one.
/// Also a trivial test bed for sharing Arduino utilties.
struct PIXEL {
  static constexpr Singleton<Adafruit_NeoPixel, NEOPIXEL_NUM, PIN_NEOPIXEL,
                             NEO_GRB + NEO_KHZ800>
      IMPL{};

  static void begin() { IMPL->begin(); }

  /// Kwarg struct to match Adafruit_NeoPixel::Color.
  struct Color {
    uint32_t red = 0, green = 0, blue = 0;
    uint32_t value() const { return red << 16 | green << 8 | blue; }
  };

  void operator=(Color color) const {
    IMPL->setPixelColor(0, color.value());
    IMPL->show();
  }
} constexpr PIXEL;

} // namespace bat
