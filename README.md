## Sduino: Small Devices Arduino

Getting started on the STM8 CPU the easy way by using an Arduino-like
programming API.

This project is based on free tools that are available for Linux, MacOS, and
Windows: SDCC, make, and stm8flash. Since it's based on the SDCC Small
Devices compiler, I called it "Small Devices -uino" or "Small-duino".

It fully integrates with the Arduino IDE or can be used
independently (GUI-less) with very easy Makefiles based on the amazing
[Arduino.mk makefile](https://github.com/sudar/Arduino-Makefile) by
[Sudar](http://sudarmuthu.com>).

This project is not supposed to be “better than Arduino”. It’s purpose
is to give you a head start into a different CPU architecture if you happen
to have a professional need or a private desire for it.

Please find more detailed information about the supported boards, the needed
tools and the library APIs on the
[project website](https://tenbaht.github.io/sduino/).

For bug reports and other issues tightly related to the repository content
please use the
[github issue tracker](https://github.com/tenbaht/sduino/issues).

For general discussions and suggestions on Sduino it might be more
convinient to open a thread on the
[STM8 board](http://stm32duino.com/viewforum.php?f=52) of the stm32duino
forum. It will help to catch my attention to prefix your topic with
"[sduino]".


## Installation

Starting with version 0.3.0 automatic IDE integration is supported via the
Arduino Boards Manager. This is the recommanded way of installation now. For
a manual non-IDE installation please check the full [installation
instructions](https://tenbaht.github.io/sduino/install/).

Arduino IDE versions 1.8.5 and 1.6.13 are tested, but any version >=1.6.6
should work.

Supported Systems:

* Linux 64 bit: Tested on Ubuntu 16.04
* Linux 32 bit: Tested on Ubuntu 16.04
* Windows: Tested on Windows 7. Sduino might work on XP (not tested), but
  the ST-Link/V2 driver is not available anymore for XP.
* MacOS: Not tested, but should be very close to working. Will need a manual
  install, but IDE integration should be easy now. Any volunteers?

Start the Arduino-IDE. In File->Preferences, Settings tab, enter

	https://github.com/tenbaht/sduino/raw/master/package_sduino_stm8_index.json

as an Additional Boards Manager URL.

Open Tools->Board:...->Boards Manager. Find Sduino by typing 'sd' into
the search line. Click on the list entry, Click on Install.

Now you should find a new entry "STM8S Boards" in the list at
Tools->Board:... Choose *STM8S103F3 Breakout Board* from the list, open the
standard Blink example from File->Examples->01. Basics->Blink and compile it
by hitting 'Verify'.



### Uploading to the board

To upload the compiled sketch to the hardware you need an ST-Link/V2
compatible flash programmer. They are very cheap, starting at $2.50 on
Aliexpress or eBay.

#### ST-Link/V2 for Linux

Required lines in /etc/udev/rules.d/99-stlink.rules:

        # ST-Link/V2 programming adapter

        # ST-Link V1, ST-Discovery boards
        ATTR{idVendor}=="0483", ATTR{idProduct}=="3744", MODE="0666", GROUP="pl

        # ST-Link/V2, the china adapter with the green plastic housing
        ATTR{idVendor}=="0483", ATTR{idProduct}=="3748", MODE="0666", GROUP="pl



#### ST-Link/V2 for Windows

Download and install the ST-Link/v2 driver **before you plug in the flash
tool**: [STSW-LINK009 driver download]
(http://www.st.com/en/development-tools/stsw-link009.html) (Registration
required, but very easy)


#### Serial upload

Most larger STM8 CPUs (unfortunately not the popular STM8S103) even come
with a builtin bootloader. Support for programming via a simple serial
adapter for these devices is planned, but not implemented yet.



## Compatibility with the Arduino world

Unfortunatly there is no free C++ compiler for the STM8. This makes it
impossible to do a full 1:1 port of the whole enviroment as is has been done
for the STM32 and the ESP8266.

This is not a drop-in replacement for an AVR, but thanks to some C
preprocessor magic the programming API is still very, very similar and it is
often enough to just move over the opening bracket of the class
instanciation statement and to replace the dot in a method call for an
underscore.



## Library support

The most important libraries and parts of the Arduino core system are
ported for the STM8 already: analog input and output, digital input and
output, delay(), millis(), Serial, SPI, and I2C.

There are ported versions of the standard libraries Servo, Stepper, and
LiquidCrystal for motor control and the popular text LCDs based on the
HD44780 controller.

For more sophisticated output there are libraries for Nokia-5110-type
graphical LCDs and SSD1306-based graphical OLED displays.
