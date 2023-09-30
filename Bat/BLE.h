#pragma once

#include "BeginEach.h"
#include "Singleton.h"
#include <bluefruit.h>

namespace bat {

struct BLE {
  static void begin() {
    Bluefruit.begin();

    Bluefruit.Periph.setConnInterval(
        9, 16); // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
    Bluefruit.setTxPower(4);

    DIS->setManufacturer("Kietzmenagerie");
    DIS->setModel("Bat Pointing Device");
    DIS->begin();

    HID->begin();

    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);

    // Include BLE HID service
    Bluefruit.Advertising.addService(*HID);

    // There is enough room for 'Name' in the advertising packet
    Bluefruit.Advertising.addName();

    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30); // number of seconds in fast mode
    Bluefruit.Advertising.start(0);           // 0 = Advertise forever
  }

  static constexpr Singleton<BLEDis> DIS{};
  static constexpr Singleton<BLEHidAdafruit> HID{};
} constexpr BLE;

struct MOUSE {
  static void move(int8_t x, int8_t y) { BLE.HID->mouseMove(x, y); }
} constexpr MOUSE;

} // namespace bat
