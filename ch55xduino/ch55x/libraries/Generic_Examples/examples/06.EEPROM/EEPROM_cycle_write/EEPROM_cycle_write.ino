/*
  EEPROM_cycle_write

  A simple example write eeprom content one by one.
  It will continue after power cycle.

  created 2020
  by Deqing Sun for use with CH55xduino

  This example code is in the public domain.

*/

uint8_t writePtr = 0;
uint8_t writeData = 0;

void dumpEEPROM() {
  USBSerial_println_s("DataFalsh Dump:");
  for (uint8_t i = 0; i < 128; i++) {
    uint8_t eepromData = eeprom_read_byte(i);
    if (eepromData < 0x10) USBSerial_print_c('0');
    USBSerial_print_ub(eepromData, HEX);
    USBSerial_print_c(',');
    if ((i & 15) == 15) USBSerial_println();
  }
  USBSerial_flush();
}

void setup() {
  for (writePtr = 0; writePtr < 128; writePtr++) {
    uint8_t eepromData = eeprom_read_byte(writePtr);
    if (writePtr != eepromData) break;
  }
  writeData = writePtr;
  if (writePtr >= 128) writePtr = 0;
}

void loop() {
  delay(5000);
  USBSerial_print_s("Write ");
  USBSerial_print_ub(writeData, HEX);
  USBSerial_print_s(" to addr: ");
  USBSerial_println_ub(writePtr, HEX);
  eeprom_write_byte(writePtr, writeData);
  writeData++;
  writePtr++;
  if (writePtr >= 128) writePtr = 0;
  dumpEEPROM();
}
