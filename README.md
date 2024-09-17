To update FreeModbus code:
 - update FreeModbus git submodule
 - launch fetch_modbus.sh

To prepare firmware for upload:
 - build with Arduino
 - find elf file location
 - launch make_firmware.sh \<input elf file\> \<output firmware file\>

To upload firmware:
 - launch ./upload_firmware.py -d \<path to serial device\> -f \<path to firmware file\> -s \<slave id\>

To read current firmware version:
 - launch ./upload_firmware.py -d \<path to serial device\> -s \<slave id\> -v


Default Modbus parameters (if eeprom cleared):
Serial: 9600 8E1
Slave ID: 1

To read current Modbus parameters read single holding register at address 100:
 - mbpoll /dev/ttyUSB0 -b 9600 -P even -a 1 -t4 -r 100

To change Modbus parameters write to holding register at address 100:
 - mbpoll /dev/ttyUSB0 -b 9600 -P even -a 1 -t4 -r 100 <value>

The content of the holding register 100 is as follows:
13:12 - parity (0 - none, 1 - odd, 2 - even)
11:8 - baudrate (0 - 2400, 1 - 4800, 2 - 9600, 3 - 14400, 4 - 19200, 5 - 28800, 6 - 38400, 7 - 57600, 8 - 76800,
                 9 - 115200, 10 - 230400, 11 - 250000, 12 - 460800, 13 - 500000, 14 - 921600, 15 - 1000000)
7:0 - slave id

e.g. to set parity none, baudrate 9600 and slave id 5:
 - mbpoll /dev/ttyUSB0 -b 9600 -P even -a 1 -t4 -r 100 517
