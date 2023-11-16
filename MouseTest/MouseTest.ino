#include <Wire.h>
#include <bluefruit.h>

#include "bat/NeoPixel.h"
#include "bat/NrfError.h"
#include "bat/Print.h"
#include "bat/BeginEach.h"
#include "bat/BLE.h"

void power_off() {
  sd_power_system_off();
}

void setup(void) {
  bat::begin_each(Wire, bat::BLE, bat::PIXEL, bat::print);
  while (!Serial) {
    delay(5);
  }

  bat::print("mouse test");
  bat::PIXEL = { .red = 32, .green = 6 };
  Bluefruit.printInfo();
  Bluefruit.Periph.printInfo();

  auto* con = Bluefruit.Connection(Bluefruit.connHandle());
  auto max_payload = con->getMtu() - 3;
  bat::print("max payload size is ", max_payload, " vs report size ", sizeof(hid_mouse_report_t));
}

bool move2(int8_t x, int8_t y) {
  hid_mouse_report_t report = {
    .x = x,
    .y = y,
  };
  //bat::BLE.HID->mouse_characteristic().notify(&report, sizeof(report));
  //bat::BLE.HID->mouse_characteristic().write(&report, sizeof(report));

  uint16_t size = sizeof(report);
  ble_gatts_hvx_params_t hvx_params = {
    .handle = bat::BLE.HID->mouse_characteristic().handles().value_handle,
    .type = BLE_GATT_HVX_NOTIFICATION,
    .offset = 0,
    .p_len = &size,
    .p_data = (uint8_t*)&report,
  };

  if (uint32_t status = sd_ble_gatts_hvx(Bluefruit.connHandle(), &hvx_params)) {
    // bat::print("status ", bat::nrf_error_message(status));
    return false;
  }
  return true;
}

void loop(void) {
  if (Serial.available()) {
    char ch = (char)Serial.read();
    bat::print("starting mouse gesture, notify ", bat::BLE.HID->mouse_characteristic().notifyEnabled() ? "enabled" : "disabled");
  auto* con = Bluefruit.Connection(Bluefruit.connHandle());
  con->requestConnectionParameter(6);
  bat::print("connection interval is ", con->getConnectionInterval() * 1.25);

    // convert to upper case
    ch = (char)toupper(ch);

    // move 127 right to start, then spiral in toward the original position
    int old_x = 127, old_y = 0;
    auto start = millis();
    while (not move2(old_x, old_y));

    int error_count = 0, total_count = 0;
    for (auto now = start; now - start < 1000; now = millis()) {
      auto t = (now - start) / 1000.f;
      auto angle = t * M_PI * 4;
      float r = 127 * (1 - t);
      int x = r * cos(angle), y = r * sin(angle);
      if (move2(x - old_x, y - old_y)) {
        old_x = x;
        old_y = y;
        bat::print("updated at ", now - start);
      } else {
      ++error_count;
      }
        ++total_count;
      delay(4);
    }
    while (not move2(-old_x, -old_y));
    bat::print("errors: ", error_count, "/", total_count, " = ", float(error_count) / total_count);
  }
}
