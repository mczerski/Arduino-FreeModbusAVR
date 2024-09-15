To update FreeModbus code:
 - update FreeModbus git submodule
 - launch fetch_modbus.sh

To prepare firmware for upload:
 - build with Arduino
 - find elf file location
 - launch make_firmware.sh \<input elf file\> \<output firmware file\>

To upload firmware:
 - launch ./upload_firmware.py -d \<path to serial device\> -f \<path to firmware file\> -s \<slave id\>
