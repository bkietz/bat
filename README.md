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

I'm absolutely not going back from c++17 and I'd prefer to use c++20.

I'd like to keep this compatible with arduino IDE since using the CLI seems like
it may scare off people who'd otherwise use this. However it's so damn opinionated
that this is secondary.

## Organization/Style:

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

> /mouse_test

- Connect as a bluetooth mouse
- Maintain serial (USB) connection 
- Trigger mouse events based on commands through serial
- Trigger mouse events based on button presses

> /power_level

- Connect as a bluetooth mouse
- Report power level via bluetooth alone
- Non adafruit example of bluetooth power reporting
  https://gist.github.com/mristau/8fe061636f4f0376e1ef8817ce1c2fec

> calibration_test

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

If you see a message starting with:
```
Timed out waiting for acknowledgement from device.
```
- Check your cable is not power only
- check your baud rate
- ensure the tty you're connected to is `/dev/ttyACM0` or maybe `/dev/ttyUSB0`
- check `dmesg | grep usb` while plugging and unplugging

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

