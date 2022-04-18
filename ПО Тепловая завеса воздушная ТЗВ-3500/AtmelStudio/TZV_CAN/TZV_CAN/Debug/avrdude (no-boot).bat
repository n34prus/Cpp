avrdude.exe -C avrdude.conf -p c128 -b 57600 -c usbasp -U flash:w:"TZV_CAN.hex":a -U eeprom:w:"TZV_CAN.eep" -U lfuse:w:0xFF:m -U hfuse:w:0x12:m -U efuse:w:0xF7:m -q
PAUSE
