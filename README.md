Weil es ein Fledermaus ist.

# Hands-free pointing device

## Organization:

Direcories contain arduino sketches which are components of functionality I built up 
individually, and whatever code can be is extracted to headers I can unit test on
my laptop.

Links from the directory where Arduino IDE expects these things to be convinces the
IDE that they're projects.

> /mouse_test

- Connect as a bluetooth mouse
- Maintain serial (USB) connection 
- Trigger mouse events based on commands through serial

> /power_level

- Connect as a bluetooth mouse
- Report power level via bluetooth alone
- https://gist.github.com/mristau/8fe061636f4f0376e1ef8817ce1c2fec

## Arduino aaargh / cheat sheet

because I forget this quickly

Install the CLI with:

```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.bin sh
```


```
Timed out waiting for acknowledgement from device.

Failed to upgrade target. Error is: No data received on serial port. Not able to proceed.
Traceback (most recent call last):
  File "/home/ben/mambaforge/lib/python3.10/site-packages/nordicsemi/__main__.py", line 296, in serial
    dfu.dfu_send_images()
  File "/home/ben/mambaforge/lib/python3.10/site-packages/nordicsemi/dfu/dfu.py", line 235, in dfu_send_images
    self._dfu_send_image(HexType.APPLICATION, self.manifest.application)
  File "/home/ben/mambaforge/lib/python3.10/site-packages/nordicsemi/dfu/dfu.py", line 199, in _dfu_send_image
    self.dfu_transport.send_start_dfu(program_mode, softdevice_size, bootloader_size,
  File "/home/ben/mambaforge/lib/python3.10/site-packages/nordicsemi/dfu/dfu_transport_serial.py", line 179, in send_start_dfu
    self.send_packet(packet)
  File "/home/ben/mambaforge/lib/python3.10/site-packages/nordicsemi/dfu/dfu_transport_serial.py", line 243, in send_packet
    ack = self.get_ack_nr()
  File "/home/ben/mambaforge/lib/python3.10/site-packages/nordicsemi/dfu/dfu_transport_serial.py", line 282, in get_ack_nr
    raise NordicSemiException("No data received on serial port. Not able to proceed.")
nordicsemi.exceptions.NordicSemiException: No data received on serial port. Not able to proceed.

Possible causes:
- Selected Bootloader version does not match the one on Bluefruit device.
    Please upgrade the Bootloader or select correct version in Tools->Bootloader.
- Baud rate must be 115200, Flow control must be off.
- Target is not in DFU mode. Ground DFU pin and RESET and release both to enter DFU mode.
```
