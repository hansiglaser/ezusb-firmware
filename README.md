ezusb-firmware
==============

Firmware skeleton for the Cypress EZ-USB microcontroller

Sources
-------

This is a fork of the OpenULINK firmware, which is part of OpenOCD.

Martin Schmölzer implemented an open source firmware for the Keil ULINK adapter
as a student project. His supervisor Johann Glaser offered a private firmware,
which was not yet publicly released, as basis. Martin greatly improved it,
adopted to SDCC 3.x and replaced all parts with questionable license
(especially Cypress source code with register definitions). He released the
OpenULINK firmware under the terms of the GPL v2 or later.

This improved firmware is now used again by Johann Glaser as starting point for
the development of a firmware skeleton for the Cypress EZ-USB microcontroller.

Clone GIT repository of OpenOCD
  $ git clone ssh://USERNAME@openocd.git.sourceforge.net/gitroot/openocd/openocd
Change directory to OpenULINK
  $ cd openocd/src/jtag/drivers/OpenULINK
Create a branch with the latest commit of the original author Martin Schmölzer,
note that this is after release 0.5.0.
  $ git branch hansi 70d9d808e523a056257308acb4402d6a4465001d

This was committed to GitHub as 00158fb7f68baf77a6ffd621293561d7e5706dbf.

Compilation
-----------

To compile the firmware, the SDCC compiler package is required. Most Linux
distributions include SDCC in their official package repositories. The SDCC
source code can be found at http://sdcc.sourceforge.net/
Simply type "make hex" in the firmware directory to compile.
"make clean" will remove all generated files except the Intel HEX file required
for downloading the firmware to the EZ-USB device.

Note that the EZ-USB microcontroller does not have on-chip flash, nor do most
devices include on-board memory to store the firmware program of the EZ-USB.
Instead, upon initial connection of the device to the host PC via USB,
the EZ-USB core has enough intelligence to act as a stand-alone USB device,
responding to USB control requests and allowing firmware download via a special
VENDOR-type control request. Then, the EZ-USB microcontroller simulates a
disconnect and re-connect to the USB bus. It may take up to two seconds for the
host to recognize the newly connected device before it can proceed to
use the device.

Once the user disconnects the device, all its memory contents are lost and
the firmware download process has to be executed again.
