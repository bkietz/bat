#pragma once

#include <bluefruit.h>

#include "BeginEach.h"
#include "Singleton.h"

namespace bat {

struct BLE {
  static void begin() {
    Bluefruit.begin();

    Bluefruit.Periph.setConnInterval(6, 12);  // min = 6*1.25=7.5 ms, max = 12*1.25=15ms
    Bluefruit.setTxPower(0);
    Bluefruit.setName("Bat Pointer");
    Bluefruit.setAppearance(BLE_APPEARANCE_HID_MOUSE);

    DIS->setManufacturer("Kietzmenagerie");
    DIS->setModel("Bat v0");
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
    Bluefruit.Advertising.setInterval(32, 244);  // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);    // number of seconds in fast mode
    Bluefruit.Advertising.start(0);              // 0 = Advertise forever
  }

  static constexpr Singleton<BLEDis> DIS{};

  struct Hid : BLEHidAdafruit {
    enum {
      REPORT_ID_KEYBOARD = 1,
      REPORT_ID_CONSUMER_CONTROL,
      REPORT_ID_MOUSE,
    };
    auto &mouse_characteristic() { return _chr_inputs[REPORT_ID_MOUSE - 1]; }
  };

  static constexpr Singleton<Hid> HID{};
} constexpr BLE;

}  // namespace bat
