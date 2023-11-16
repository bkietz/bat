#pragma once

#include <cstdint>
#include <limits>

#include "BLE.h"
#include "Eigen.h"

namespace bat {
struct MOUSE {
  static void move(Vector<int8_t, 2> xy) { BLE.HID->mouseMove(xy[0], xy[1]); }
} constexpr MOUSE;

struct MouseGesture {
  constexpr static int8_t _clamp(float i) {
    using Limits = std::numeric_limits<int8_t>;
    if (i > Limits::max()) return Limits::max();
    if (i < Limits::min()) return Limits::min();
    return static_cast<int8_t>(i);
  }

  // /home/ben/.arduino15/packages/adafruit/tools/arm-none-eabi-gcc/9-2019q4/bin/arm-none-eabi-g++
  // -mcpu=cortex-m4 -mthumb -c -g -Werror=return-type -mfloat-abi=hard -mfpu=fpv4-sp-d16
  // -u _printf_float -std=gnu++2a -ffunction-sections -fdata-sections
  // -fno-threadsafe-statics -nostdlib --param max-inline-insns-single=500 -fno-rtti
  // -fno-exceptions -w -x c++ -E -CC -DF_CPU=64000000 -DARDUINO=10607
  // -DARDUINO_NRF52840_FEATHER_SENSE -DARDUINO_ARCH_NRF52 -DARDUINO_BSP_VERSION="1.5.0"
  // -DNRF52840_XXAA -DUSBCON -DUSE_TINYUSB -DUSB_VID=0x239A -DUSB_PID=0x8087
  // -DUSB_MANUFACTURER="Adafruit" -DUSB_PRODUCT="Feather nRF52840 Sense"
  // -DSOFTDEVICE_PRESENT -DARDUINO_NRF52_ADAFRUIT -DNRF52_SERIES -DDX_CC_TEE
  // -DLFS_NAME_MAX=64 -Ofast -DCFG_DEBUG=0 -DCFG_LOGGER=0 -DCFG_SYSVIEW=0
  // -I/home/ben/.arduino15/packages/adafruit/tools/CMSIS/5.7.0/CMSIS/Core/Include/
  // -I/home/ben/.arduino15/packages/adafruit/tools/CMSIS/5.7.0/CMSIS/DSP/Include/
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic/nrfx
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic/nrfx/hal
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic/nrfx/mdk
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic/nrfx/soc
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic/nrfx/drivers/include
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic/nrfx/drivers/src
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic/softdevice/s140_nrf52_6.1.1_API/include
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/nordic/softdevice/s140_nrf52_6.1.1_API/include/nrf52
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/freertos/Source/include
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/freertos/config
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/freertos/portable/GCC/nrf52
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/freertos/portable/CMSIS/nrf52
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/sysview/SEGGER
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5/sysview/Config
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/libraries/Adafruit_TinyUSB_Arduino/src/arduino
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/cores/nRF5
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/variants/feather_nrf52840_sense
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/libraries/Wire
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/libraries/Bluefruit52Lib/src
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/libraries/Adafruit_nRFCrypto/src
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/libraries/Adafruit_TinyUSB_Arduino/src
  // -I/home/ben/Arduino/libraries/Adafruit_NeoPixel -I/home/ben/Arduino/libraries/Eigen
  // -I/home/ben/Arduino/libraries/ArxTypeTraits
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/libraries/Adafruit_LittleFS/src
  // -I/home/ben/.arduino15/packages/adafruit/hardware/nrf52/1.5.0/libraries/InternalFileSytem/src
  // /tmp/arduino/sketches/A186DE617F0051D2152DF2EDCA6EAEC3/sketch/MouseTest.ino.cpp -o
  // /tmp/335707591/sketch_merged.cpp

  void move_to(Vector<float, 2> xy) {
    auto delta = xy - previous;
    int8_t x = _clamp(delta[0]);
    int8_t y = _clamp(delta[1]);

    BLE.HID->mouseMove(x, y);
    previous[0] += x;
    previous[0] += y;
  }

  Vector<float, 2> previous = {};
};

}  // namespace bat
