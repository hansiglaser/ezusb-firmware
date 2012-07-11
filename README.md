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


