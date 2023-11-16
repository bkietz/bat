#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <bluefruit.h>
#include <bat/color.h>

BLEDis bledis;
BLEBas blebas;
BLEHidAdafruit blehid;

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

  //neoPixel.setPixelColor(0, 0'16'0_rgb);
  //neoPixel.show();

  Serial.println("battery test");
  Serial.flush();

  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
  Bluefruit.begin();
  Bluefruit.Periph.setConnInterval(9, 16); // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
  Bluefruit.setTxPower(4);

  Bluefruit.Periph.setConnectCallback([](uint16_t conn_handle) {
    BLEConnection* connection = Bluefruit.Connection(conn_handle);
    char central_name[32] = {};
    connection->getPeerName(central_name, sizeof(central_name));
    Serial.print("Connected to ");
    Serial.println(central_name);
  });
  Bluefruit.Periph.setDisconnectCallback([](uint16_t conn_handle, uint8_t reason) {
    Serial.print("Disconnected, reason = 0x");
    Serial.println(reason);
  });

  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit feather 52");
  bledis.begin();

  blehid.begin();

  blebas.begin();
  blebas.write(100);

  Wire.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addService(blebas);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

void loop(void) {
  while (Serial.available()) {
    delay(2);
    uint8_t buf[10];
    int count = Serial.readBytes(buf, sizeof(buf));
    blebas.write(count * 10);
  }
}
