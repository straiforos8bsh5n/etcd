# Status and Todo

## Current status

#### tested and working

* `pinMode()`
* `digitalWrite()`
* `analogRead()`
* `delay()`
* `analogWrite()`
* `ShiftOut()`
* WMath: `map()`
* `serialEvent()`
* `pulseInLong()`
* [HardwareSerial](../api/HardwareSerial.md):
  Standard UART interface with Print functions
* [Print](../api/Print.md):
  Formatting functions to be used with the other output libraries
* [SPI](../api/SPI.md):
  working, no interrupt support  
* [LiquidCrystal](../api/LiquidCrystal.md):
  Text LCD based on the HD44780 controller  
* [PCD8544](../api/PCD8544.md):
  Nokia 5110 type displays  
* [Mini_SSD1306](../api/Mini_SSD1306.md):
  Monochrome OLED displays based on the SSD1306 controller
* [Stepper](../api/Stepper.md):
  Multi-instance design for more than one stepper at a time  
* [Servo](../api/Servo.md):
  Multi-instance design for more than one servo at a time)  

#### implemented and partly working

* [Wire/I2C](../api/I2C.md)

#### tested, but not working

* `alternateFunctions()`

#### not tested

* `ShiftIn()`
* `random()`
* `srandom()`

#### not implemented

* `tone()`
* `noTone()`
* `pulseIn()`
* module WCharacter
* module WString



## Todo

### Unresolved problems

The compile environment needs to detect which interrupts are actively used
and link only the needed ones into the binary. See test/digitalWrite:
Compiling with the straight Makefile.classic does not add UART interrupt
routines. But when using the sduino.mk Makefile the two UART interrupt
routines are pulled into the binary by the interrupt table in main.c.

