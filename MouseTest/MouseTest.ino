#include <Wire.h>
#include <bluefruit.h>

#include "Bat/NeoPixel.h"
#include "Bat/Print.h"
#include "Bat/BeginEach.h"
#include "Bat/BLE.h"

void power_off() {
  sd_power_system_off();
}

void setup(void) {
  bat::begin_each(Wire, bat::BLE, bat::PIXEL, bat::print);
  bat::print("mouse test");
  bat::PIXEL = { .green = 16 };
}

void loop(void) {
  /*
  delay(500);
  bat::PIXEL = { .red = 16 };
  delay(500);
  bat::PIXEL = { .green = 16 };
  */
  if (Serial.available()) {
    char ch = (char)Serial.read();

    // convert to upper case
    ch = (char)toupper(ch);

    for (int8_t x = 0; x < 10; ++x) {
      bat::MOUSE.move(10, x & 1 ? 10 : -10);
    }
  }
}
