# Installation

Supported Systems:

* Linux 64 bit: Tested on Ubuntu 16.04
* Linux 32 bit: Tested on Ubuntu 16.04
* Windows: Tested on Windows 7. Sduino might work on XP (not tested), but
  the ST-Link/V2 driver is not available anymore for XP.
* MacOS: Not tested, but should be very close to working. Will need a manual
  install, but IDE integration should be easy now. Any volunteers?
* Raspian/Raspberry Pi: Untested. Will require a manual install.


## GUI install

Starting with version 0.3.0 automatic IDE integration is supported via the
Arduino Boards Manager. This is the recommanded way of installation now.
Arduino IDE versions 1.8.5 and 1.6.13 are tested, but any version >=1.6.6
should work.

Start the Arduino-IDE. In File->Preferences, Settings tab, enter

	https://github.com/tenbaht/sduino/raw/master/package_sduino_stm8_index.json

as an Additional Boards Manager URL.

Open Tools->Board:...->Boards Manager. Find Sduino by typing 'sd' into
the search line. Click on the list entry, Click on Install.

Now you should find a new entry "STM8S Boards" in the list at
Tools->Board:... Choose *STM8S103F3 Breakout Board* from the list, open the
standard Blink example from File->Examples->01. Basics->Blink and compile it
by hitting 'Verify'.


### using make on the command line

All the needed tools are now installed, but they are not in your path or
where the Makefile expects them to be. There is work needed here. #23



## Manual install

Only required if you want to use a current repository snapshot or if the GUI
install fails for your system (please take the time to file a [bug
report](https://github.com/tenbaht/sduino/issues)).

All tools are available for Linux, MacOS and Windows, most of them even for
Raspbian on a RaspberryPi.

- Clone or download and extract the
  [repository](https://github.com/tenbaht/sduino) to a convenient location.
- Download [the latest snapshot build](http://sdcc.sourceforge.net/snap.php)
  for your system from the [SDCC project site on
  sourceforge](http://sdcc.sourceforge.net/). Support for the STM8 improves
  significantly from version to version. Do not use the older version of
  SDCC that might be included in your distribution (Version 3.6 is
  definitely too old).

Later in the process you will unpack the SDCC archive. You only need support
for the stm8. You can safely delete all the files related to the PIC CPUs in
the `pic*` and `non-free` directories. That safes more than 90% of the used
disk space and leaves only 20MB out of 240MB.



### Linux

Link the hardware/sduino folder of the downloaded repository to your local
Arduino hardware folder (usually in `~/sketchbook` or `~/Arduino`). Create
the hardware folder, if not already present:

	cd ~/Arduino
	mkdir hardware
	cd hardware
	ln -s [the-extracted-repository]/sduino/hardware/sduino .

Extract the [downloaded SDCC archive](http://sdcc.sourceforge.net/snap.php)
under `/opt`:

	sudo mkdir /opt
	sudo tar xvjf ~/Downloads/sdcc-snapshot* -C /opt

Create a link to SDCC from the tools directory of the sduino repository:

	cd [the-extracted-repository]/sduino/hardware/sduino/tools
	ln -s /opt/sdcc .

`stm8flash` and `stm8gal` are both included as precompiled binaries in this
tools directory. Create a link `linux` to the linux tools directory that
matches your system and copy the binaries to a convient place in your path,
e.g. `/usr/local/bin`:

	ln -s linux64 linux
	cp -av linux/* /usr/local/bin


`make` and libusb-1.0 (needed by stm8flash) are part of the standard
repository for any Linux distribution. Example for Debian-type systems (like
Ubuntu, Mint, Elementary etc.):

	sudo apt-get install make libusb-1.0-0

Finally, add a udev rule in order to access the USB port for your flash
tool. Save this as root in in `/etc/udev/rules.d/99-stlink.rules`:

	# ST-Link/V2 programming adapter

	# ST-Link V1, if using a STM8S discovery board
	# important: It needs a special entry in /etc/modprobe.d
	ATTR{idVendor}=="0483", ATTR{idProduct}=="3744", MODE="0666", GROUP="plugdev"

	# ST-Link/V2, the china adapter with the green plastic housing
	ATTR{idVendor}=="0483", ATTR{idProduct}=="3748", MODE="0666", GROUP="plugdev"

Using the STM8S-Discovery board as a flash programmer requires a [special
modprobe configuration
entry](hardware/stm8sdiscovery#usage-with-linux-and-stm8flash).



### MacOS

Quite similar to the Linux install. [Install Homebrew](https://brew.sh/) if
you haven't done it yet, than install make and unpack the sdcc snapshot:

	brew make
	mkdir /opt
	tar xvjf sdcc-snapshot* -C /opt

Adopt the other steps from the Linux install. The arduino hardware folder is
in your Documents folder at `/Users/<username>/Documents/Arduino/hardware`.

Not sure about stm8flash, I didn't try it. Have a look at the [stm8flash
repository](https://github.com/vdudouyt/stm8flash). It would be great if
someone could summarize the procedure and send it to me together with a
compiled binary for the repository.



### Windows

There is more work to do here, as it is lacking all the tools for the build
system, a package system to install them and even a file system concept
where to store them.

#### SDCC 

Unpack the downloaded SDCC archive into hardware/sduino/tools.

#### MinGW

The bare minimum of needed unix-like tools is collected in a [separate
repository](https://github.com/tenbaht/sduino-windowstools). This is used
for building the archive file for the Board Manger based install. Download
it and copy the directory tree into the sduino directory.


#### ST-Link, stm8flash

If not already done, install the ST-Link driver from the ST website for
hardware access.

You are not bound to `stm8flash`, you can use the regular flash tool by ST
as well.


#### Using the Makefile-based build system

For IDE based builds you are done now. For use of the Makefile-based build
system you need to update your `%PATH%`:

- Add a link `c:\sdcc` to your SDCC installation directory and add this to
  your `%PATH%`.
- Add the `sduino/tools/win` directory to your path or move/copy/link the
  files in there to a directory that is already in your path.



#### The individual pieces

All the tools in the convinience package are 32 bit. If you need the 64 bit
versions you might have to collect them yourself.

You need `make` with some basic tools and maybe `stm8flash`. `make` is a
standard tool included in either MinGW/Msys or cygwin. Both are fine,
MinGW/Msys is smaller. `stm8flash` from the [stm8flash
repository](https://github.com/vdudouyt/stm8flash).




#### MinGW

[MinGW/MSys](http://www.mingw.org/wiki/MSYS) and
[cygwin](https://www.cygwin.com/) are both fine. cygwin aims to be an almost
complete POSIX environment (which is nice, but we don't need it here). MinGW
wants to be more compact and works with the native Windows API. That is good
enough for this purpose.

1. Check the
  [MinGW Installation Notes](http://www.mingw.org/wiki/Getting_Started)
2. Download
  [mingw-get-setup.exe](https://sourceforge.net/projects/mingw/files/Installer/)
  from https://sourceforge.net/projects/mingw/files/Installer/
3. Start it. You can safely deactivate the graphical option.
4. Add this at the end to your path: `;c:\mingw\bin;c:\mingw\msys\1.0\bin`
  (follow the instructions in "Environment Setting" on the [Installation
  Notes page[(http://www.mingw.org/wiki/Getting_Started))
5. Open a command line and install the package msys-base by issuing this
  command: `mingw-get install msys-base`
6. Now `ls` or `make` should work.

For efficiency, the Makefile is configured to use dash instead of bash as a
shell. `egrep` is replaced by `grep -E`. The bare minimum of tools you will
need to run the Arduino.mk makefile:

	dash make
	awk cat cut expr grep head ls mkdir sed tr uname which




#### stm8flash

A precompiled windows binary was in the stm8flash repository for a while but
got removed at one point. I used an
[old version](https://github.com/vdudouyt/stm8flash/raw/39b1a9ec1dd26030065c3e476fc3b7b89626e21d/stm8flash.exe)
of the file. You might prefer to compile your own version from the
[stm8flash repository](https://github.com/vdudouyt/stm8flash) using either
MinGW or cygwin. For compiling, you will need the libusb windows binary:
http://libusb.info/ (I used the MinGW32 dll)




### General problems using Windows

It works, but using the Arduino.mk makefile with Windows is slow. **Very**
slow. Painfully slow. Compiling-the-Blink-example-takes-about-40-seconds
kind of slow. Yes, seriously. No kidding. 40 seconds. Measured on a 3GHz
machine with 4GB RAM.

There is no easy fix, the underlying problem is a fundamental one. It is not
about the compilation itself, it is the way Makefiles are written and
executed. The whole concept relies on forking subprocesses for all the shell
calls. Unfortunately, there is nothing like a fork in Windows and to work
around that is painfully slow.

It would be great if somebody could manage to modify the original Arduino
build system to support non-C++-builds. Or to integrate it somehow into the
STVD IDE. Or replace the painfully slow parameter checking part of the
makefile (that causes the majority of forking) by a single shell script that
gets called by the makefile and delivers the results in no time. Or use
[cmake](www.cmake.org). Or whatever.

Until than the least annoying way out might be using a virtual machine
running a simple Linux system. Ubuntu Mate or a basic Debian install for
example. Virtual Box is great for this purpose and freely available.



## Possible improvements

### All-inclusive packages

One of the big convinience points of the Arduino project are the
easy-to-install all-inclusive packages without any external dependencies.
They don't require any prior knowledge to set up a full cross-compiler tool
chain.

Something like that would be very nice for the STM8 as well, but I didn't go
that far. You still have to do some work yourself. You definitly need the
compiler SDCC. `stm8flash` is a simple tool to transfer the compiled program
to the CPU, but any other flash tool supporting the SWIM protocol will do.
The build process is controlled by `make`. It is a standard tool with any
UNIX system but available for windows as well.

Alternative ways of controlling the build process exist, but then you are
on your own to set it up.
