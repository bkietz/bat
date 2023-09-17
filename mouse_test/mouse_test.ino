#include <Adafruit_NeoPixel.h>
#include <Wire.h>

void power_off() {
  sd_power_system_off();
}

Adafruit_NeoPixel neoPixel{ NEOPIXEL_NUM, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800 };

void setup(void) {
  neoPixel.begin();
  Serial.begin(115200);
  
  while (!Serial) {
    delay(5);
  }

  neoPixel.setPixelColor(0, neoPixel.Color(0, 16, 0));
  neoPixel.show();

  Serial.println("mouse test");
  Serial.flush();

  Wire.begin();
}

void loop(void) {
  delay(500);
  neoPixel.setPixelColor(0, neoPixel.Color(0, 0, 16));
  neoPixel.show();
  delay(500);
  neoPixel.setPixelColor(0, neoPixel.Color(0, 16, 0));
  neoPixel.show();
}
