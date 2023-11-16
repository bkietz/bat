Weil es ein Fledermaus ist.

# Hands-free pointing device

## Goals:

I'd prefer to keep everything as self contained as possible. For example, most
uses of AHRS with microcontrollers require hard coded calibration values which
are assembled with entirely different firmware. That implicitly binds to a
specific physical arrangement, since any introduction of metal or stress to the
board's sensors might throw those values off until new calibration values can be
laboriously assembled and re-hard coded. I strongly prefer to keep calibration
as a subroutine which can be triggered any time I suspect it's necessary.

I'm absolutely not going back from C++17 and I'd prefer to use C++20.

I'd like to keep this compatible with arduino IDE since using the CLI seems like
it may scare off people who'd otherwise use this. However it's so damn opinionated
that this is secondary.

## Organization/Style:

`bool(Serial)` should be treated as a volatile, like button state. I want things to
proceed even if serial's not available, then dump stuff to it whenever something's
connected.

Direcories contain arduino sketches which are components of functionality I built up 
individually, and whatever code can be is extracted to headers I can unit test on
my laptop. This is the intent of the Arduino IDE's sketches being bundled with
libraries and doing this in my own repo is redundant, but I won't understand them
until I rewrite them. Also I want to be able to reuse headers between sketches, which
Arduino IDE is terrible at.

Headers and sketches will be named as `CamelCase.h`, because other extensions are not
always recognized. Shared headers are in the /Bat directory, which is symlinked into
each sketch in this repo. This way every sketch can include the same header with
`#include "Bat/NeoPixel.h"`.

Don't make large nested structures to compartmentalize things.
The TU itself is the object; there's no need for members.

Compilation flags are set in platform.local.txt, which should be placed alongside the
board's platform.txt. Eventually it'd be neat to script generation of platform.local.txt
and also its emplacement.

> /MouseTest

- Connect as a bluetooth mouse
- Maintain serial (USB) connection 
- Trigger mouse events based on commands through serial
- Trigger mouse events based on button presses

> /BatteryTest

- Connect as a bluetooth mouse
- Report power level via bluetooth alone, at levels drawn from USB
- Non adafruit example of bluetooth power reporting
  https://gist.github.com/mristau/8fe061636f4f0376e1ef8817ce1c2fec
- Try using notify instead of just write

> /CalibrationTest

- `Adafruit_Sensor_Calibration.h`
- Load and print the calibration on start, if available
- Start calibration on a button sequence
- Save the calibration so I can turn off and see it's still there

## Arduino aaargh / cheat sheet

Install the CLI with:
```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.bin sh
```

Plug the board in then get info with:
```
arduino-cli board list
arduino-cli board details -b adafruit:nrf52:feather52840sense
```

The location of platform.txt is one of the first lines of output when building with verbose logging.

In the verbose build log, grep for `MouseTest.ino.cpp` to get the location of a complete and pure-ish
c++ project generated from the arduino sketch. This can be used if you miss a real language server.

If you see a message starting with:
```
Timed out waiting for acknowledgement from device.
```
- Check your cable is not power only
- check your baud rate
- ensure the tty you're connected to is `/dev/ttyACM0` or maybe `/dev/ttyUSB0`
- check `dmesg | grep usb` while plugging and unplugging
- ensure nothing else has a lock on that tty; ArduinoIDE grabs it quite aggressively
  and CLIs do not override it. `arduino-cli monitor -p /dev/ttyACM0` can be used to
  get a definitely-temporary connection to the serial port.

Panic reset the bootloader:
```
adafruit-nrfutil --verbose dfu serial \
  --package ~/Downloads/feather_nrf52840_sense_bootloader-0.8.0_s140_6.1.1.zip \
  -p /dev/ttyACM0 -b 115200 --singlebank --touch 1200
```

Periodically I2C communication with the acc/gyro sensor craps the bed.
Try pushing the reset button, or running one of the acc/gyro examples in the IDE,
or unplugging and replugging the board.

My phone can pair with it no problem. With bluez on Linux it *should* be as simple as:
```
$ sudo bluetoothctl
[bluetooth]# menu scan
[bluetooth]# transport le
[bluetooth]# back
[bluetooth]# scan on
[NEW] Device CC:5A:74:2F:AE:3E Feather nRF5284
[bluetooth]# info CC:5A:74:2F:AE:3E
```
... but the laptop only saw it for a moment then I've never gotten it to come
up in scan again. Replacing it with an explicitly BLE compatible dongle fixed
the problem immediately.

Annoyingly, no matter what I request the connection interval winds up as 45ms.
This is probably a limitation of the ble dongle on
the host side; the interval is 15ms when connected to my phone.
I hope I can find a dongle which can do better; 22 Hz is horribly choppy for an
input device.
https://unix.stackexchange.com/questions/288978/how-to-configure-a-connection-interval-in-a-ble-connection/458056#458056
Got it (thanks adafruit discord): using `Bluefruit.Periph.setConnInterval(6, 12)` does
not *require* the connection interval to be anything; it calls a preference setting
function. Instead we need `con->requestConnectionParameter(6)`, which can force update
the interval.

A related annoyance is the limited queue size for GATT notifications.
I can get about 200 successful hid reports if I retry every millisecond, or
160 if I retry every 5ms, so the queue would appear to have a capacity of just 1.
At this rate when communicating with the laptop I am able to move the cursor at a
maximum speed of `160 * 127 = 20,320` pixels per second, which is sufficient but
cutting it close.
https://devzone.nordicsemi.com/f/nordic-q-a/64837/change-queue-length-of-handle-value-notifications-sdk-15-3-0
Also tried this, no dice:
```
  extern uint32_t  __data_start__[]; // defined in linker
  uint32_t ram_start = (uint32_t) __data_start__;
  ble_cfg_t blecfg;
  blecfg.conn_cfg.conn_cfg_tag = CONN_CFG_PERIPHERAL;
  blecfg.conn_cfg.params.gatts_conn_cfg.hvn_tx_queue_size = 3;
  auto status = sd_ble_cfg_set(BLE_CONN_CFG_GATTS, &blecfg, ram_start);
```

The Adafruit OO wrappers around Nordic's C api can sometimes be massively slower.
In the critical case of sending HID reports, I was seeing 90ms delay between reports.
By contrast, calling Nordic's functions directly had no delay at all.
