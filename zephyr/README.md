Or... there's zephyr. That's a much more inpsiring platform than arduino,
and they build with GCC 12.

Zephyr has main branch support for the almost-equivalent board
[Adafruit Feather nRF52840 Express](https://github.com/zephyrproject-rtos/zephyr/blob/main/boards/arm/adafruit_feather_nrf52840/doc/index.rst).

There's a fork which explicitly adds the Sense
https://github.com/rtenlab/zephyr/tree/8c091f9ef2926191a37103981e7de0e496f439ad#notes-on-adafruit-feather-nrf52840-sense
... but the files don't seem to differ at all from the Express'!
This issue showed some more modifications includuing a clock config difference
https://github.com/zephyrproject-rtos/zephyr/issues/32279

Following that and [zephyr's own getting started guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html),
```
# used conda for environment management
conda create --name=zephyr python cmake
conda activate zephyr
pip install -y west adafruit-nrfutil

# get zephyr and install python dependencies 
mkdir zephyrproject
west init zephyrproject
cd zephyrproject
west update
# This creates a cmake exported package in ~/.cmake
# I think it'd be better to create that in the conda env
west zephyr-export
pip install -r zephyr/scripts/requirements.txt
```

Setup the Zephyr SDK once:
```
cd
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.3/zephyr-sdk-0.16.3_linux-x86_64.tar.xz
wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.3/sha256.sum | shasum --check --ignore-missing
tar xvf zephyr-sdk-0.16.3_linux-x86_64.tar.xz
cd zephyr-sdk-0.16.3
./setup.sh
sudo cp ~/zephyr-sdk-0.16.3/sysroots/x86_64-pokysdk-linux/usr/share/openocd/contrib/60-openocd.rules /etc/udev/rules.d
sudo udevadm control --reload
```

Set up for flashing the Sense
```
cd zephyrproject/zephyr
west build -p always -b adafruit_feather_nrf52840 samples/basic/blinky
adafruit-nrfutil dfu genpkg --dev-type 0x0052 --sd-req 0x00B6 --application build/zephyr/zephyr.hex build/zephyr/zephyr.zip
adafruit-nrfutil --verbose dfu serial -pkg build/zephyr/zephyr.zip -p /dev/ttyACM0 -b 115200 --singlebank
```

... compared to the output of the arduino IDE while building
```
/home/ben/.arduino15/packages/adafruit/tools/arm-none-eabi-gcc/9-2019q4/bin/arm-none-eabi-objcopy \
  -O ihex /tmp/arduino/sketches/A186DE617F0051D2152DF2EDCA6EAEC3/MouseTest.ino.elf \
  /tmp/arduino/sketches/A186DE617F0051D2152DF2EDCA6EAEC3/MouseTest.ino.hex

adafruit-nrfutil dfu genpkg --dev-type 0x0052 --sd-req 0x00B6 --application \
  /tmp/arduino/sketches/A186DE617F0051D2152DF2EDCA6EAEC3/MouseTest.ino.hex \
  /tmp/arduino/sketches/A186DE617F0051D2152DF2EDCA6EAEC3/MouseTest.ino.zip

adafruit-nrfutil --verbose dfu serial -pkg \
  /tmp/arduino/sketches/A186DE617F0051D2152DF2EDCA6EAEC3/MouseTest.ino.zip -p /dev/ttyACM0 -b 115200 --singlebank
```

We have a UF2 bootloader, so we can double tap reset then get a USB mass storage device.
UF2 files can then simply be dragged-and-dropped in, after which they get auto flashed.
West can be configured to emit UF2 or they can be converted by hand:
```
python scripts/build/uf2conv.py -c \
  -f 0xada52840 -b 0xc000 \
  -o ~/Downloads/MouseTest.uf2 \
  /tmp/arduino/sketches/A186DE617F0051D2152DF2EDCA6EAEC3/MouseTest.ino.hex 
```
... but although the uf2 created by the arduinoIDE flashes successfully, the one built
by west does not. Also, after attempting to use the uf2 created by west I must
re-flash the bootloader before arduinoIDE can do anything with it:
```
adafruit-nrfutil --verbose dfu serial \
  --package ~/Downloads/feather_nrf52840_sense_bootloader-0.8.0_s140_6.1.1.zip \
  -p /dev/ttyACM0 -b 115200 --singlebank --touch 1200
```

Well, I've opened a PR for it.
https://github.com/zephyrproject-rtos/zephyr/pull/63819

# zephyr errors list:
- everything in the PR
- xiao sense's default runner should be uf2 probably
- lsm6dsl's example has typos:
  - the escape code passed to printf is wrong. Should be \x1b[2J\x1b[H
  - doesn't need to use printf or `CONFIG_STDOUT_CONSOLE=y`
  - the sensor outputs radians per second rather than degrees
