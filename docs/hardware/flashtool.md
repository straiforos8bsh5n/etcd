# Flash tool

You need a special flash tools in order to program the CPU. As far as I know
there is no third-party product or software that implements the needed
communication protocol. So you can't just use a regular Arduino board as a
flash tool as you could for the ATmega. But this is not a problem, as these
tools are are easily available and unbeliveably cheap (well under $3 on
aliexpress, search for st-link).

There are two versions of the ST-Link V2 compatible flash tool available:
One in a USB-Drive-like metal housing (often pink or blue) and one made by
Baite in a green plastic housing. Both work well, but they use a different
pinout.

![Picture of both flash tools](ST-LinkV2_pinout_01.jpg)

Both flash tools support the SWIM protocol for STM8 CPUs and the SWD
protocol for the STM32 CPUs. The programmer from Baite additionally supports
JTAG and is slightly superior to the more common one in the metal housing.
More information on the hardware and the pinouts:
https://wiki.cuvoodoo.info/doku.php?id=jtag


## Pinout

Pinout of Chinese ST-Link V2-clone made by Baite with green plasic housing
(supports SWIM, SWD and JTAG):

                +-----+
        T_JRST  | 1  2|	3V3
        5V      | 3  4|	T_JTCK/T_SWCLK
        SWIM      5  6|	T_JTMS/T_SWDIO
        GND     | 7  8|	T_JTDO
        SWIM RST| 9 10|	T_JTDI
                +-----+

Pinout of Chinese ST-Link V2-clone with metal housing (supports SWIM and
SWD):

                +-----+
        RST     | 1  2|	SWDIO
        GND     | 3  4|	GND
        SWIM      5  6|	SWCLK
        3V3     | 7  8|	3V3
        5V      | 9 10|	5V
                +-----+


## Installation for Linux

Save this as root in in `/etc/udev/rules.d/99-stlink.rules`:

	# ST-Link/V2 programming adapter

	# ST-Link V1, if using a STM8S discovery board
	# important: It needs a special entry in /etc/modprob/blacklist
	ATTR{idVendor}=="0483", ATTR{idProduct}=="3744", MODE="0666", GROUP="plugdev"

	# ST-Link/V2, the china adapter with the green plastic housing
	ATTR{idVendor}=="0483", ATTR{idProduct}=="3748", MODE="0666", GROUP="plugdev"

Using the STM8S-Discovery board as a flash programmer is a special case and
requires [some additional
blacklisting](Dstm8sdiscovery#usage-with-linux-and-stm8flash).



## Installation for Windows

Download and install the ST-Link/v2 driver **before you plug in the flash
tool**:
[STSW-LINK009 driver download](http://www.st.com/en/development-tools/stsw-link009.html)
(Registration required, but very easy)



## Connection to the CPU board

The pinout of the SWIM connector P3 on my STM8S103 breakout board fits the
pinout of the flash tool in the metal housing perfectly:

| Signal | SWIM connector P3 | Baite ST-Link | Metal ST-Link
| ------ | :---------------: | :-----------: | :-----------:
| 3V3	 | 1		     |	2	     |	7
| SWIM	 | 2		     |	5	     |	5
| GND	 | 3		     |	7	     |	3
| NRST	 | 4		     |	9	     |	1


The Discovery boards made by ST all feature a ST-Link interface as well, but
only the Discovery STM8S105 supports the SWIM protocol. The Discovery
STM32F0308 implements SWD only and is not usable for the STM8.

|Pin out CN3	| SWD
|-----------	| --------------
|1		| ? detect oder so?
|2		|JTCK/SWCLK
|3		|GND
|4		|JTMS/SWDIO
|5		|NRST
|6		|SWO

