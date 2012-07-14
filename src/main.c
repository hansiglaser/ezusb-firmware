/***************************************************************************
 *   Copyright (C) 2011 by Martin Schmoelzer                               *
 *   <martin.schmoelzer@student.tuwien.ac.at>                              *
 *   Copyright (C) 2012 by Johann Glaser <Johann.Glaser@gmx.at>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "io.h"
#include "usb.h"
#include "commands.h"

/**
 * Interrupt Vectors
 *
 * All ISRs must be declared in the file where main() is
 * (see http://sdcc.sourceforge.net/doc/sdccman.html/node65.html)
 *
 * Note about USB interrupts:
 * We don't write interrupt numbers here because we don't want the compiler to
 * automatically place LJPMs into the interrupt vector table. These ISRs are
 * called from the USB interrupt vector table defined in USBJmpTb.a51. The
 * downside of using this self-built jump table is that we have to provide
 * ISRs (more precise: labels) for all the possible interrupts.
 *
 * Unfortunately when linking this file to the final .ihx file, some compiler
 * specific libraries are added (see usb.lnk: libsdcc, libint, liblong,
 * libfloat). These would be placed to some free space which is incidentially
 * exactly where the 8051 interrupt vector table is. Therefore we use _one_
 * ISR vector (here 13) to "reserve" that space.
 */
// I2C
//extern void i2c_isr(void)      __interrupt I2C_VECT;
// USB
extern void sudav_isr(void)    __interrupt SUDAV_ISR;
extern void sof_isr(void)      __interrupt;
extern void sutok_isr(void)    __interrupt;
extern void suspend_isr(void)  __interrupt;
extern void usbreset_isr(void) __interrupt;
extern void ibn_isr(void)      __interrupt;
extern void ep0in_isr(void)    __interrupt;
extern void ep0out_isr(void)   __interrupt;
extern void ep1in_isr(void)    __interrupt;
extern void ep1out_isr(void)   __interrupt;
extern void ep2in_isr(void)    __interrupt;
extern void ep2out_isr(void)   __interrupt;
extern void ep3in_isr(void)    __interrupt;
extern void ep3out_isr(void)   __interrupt;
extern void ep4in_isr(void)    __interrupt;
extern void ep4out_isr(void)   __interrupt;
extern void ep5in_isr(void)    __interrupt;
extern void ep5out_isr(void)   __interrupt;
extern void ep6in_isr(void)    __interrupt;
extern void ep6out_isr(void)   __interrupt;
extern void ep7in_isr(void)    __interrupt;
extern void ep7out_isr(void)   __interrupt;

static void io_init(void) {
  /* PORTxCFG register bits select alternate functions (1 == alternate function,
   *                                                    0 == standard I/O)
   * OEx register bits turn on/off output buffer (1 == output, 0 == input)
   * OUTx register bits determine pin state of output
   * PINx register bits reflect pin state (high == 1, low == 0) */

  /* Port A: ... */
  PORTACFG = PORTA_SPECIAL_FUNC;
  OEA      = PORTA_OE;
  OUTA     = PORTA_INIT;

  /* Port B: ... */
  PORTBCFG = PORTB_SPECIAL_FUNC;
  OEB      = PORTB_OE;
  OUTB     = PORTB_INIT;

  /* Port C: ... */
  PORTCCFG = PORTC_SPECIAL_FUNC;
  OEC      = PORTC_OE;
  OUTC     = PORTC_INIT;

  /* Enable CLK24 output */
  CPUCS |= CLK24OE;

  /* External Memory Interface Wait States for Fast Read */
  CKCON = 0x00;      // CKCON[2..0]: 0 wait states
}

int main(void) {
  io_init();
  usb_init();

  /* Globally enable interrupts */
  EA = 1;

  /* Begin executing command(s). This function never returns. */
  command_loop();

  /* Never reached, but SDCC complains about missing return statement */
  return 0;
}
