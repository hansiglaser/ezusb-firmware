/***************************************************************************
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

#include <stdbool.h>
#include "reg_ezusb.h"
#include "i2c.h"

/**
 * State of the I2C driver
 */
typedef enum {
  stIdle,
  stRecvFirst,
  stReceiving,
  stSending,
  stStop,
  stBusError,
  stNAck
} I2C_State;

volatile static I2C_State        i2c_state;
volatile static uint8_t          i2c_length;
volatile static __xdata uint8_t* i2c_ptr;
volatile static uint8_t          i2c_count;

// Forward Declarations

static void i2c_wait_stop();
static I2C_Status i2c_wait_finished();

/*****************************************************************************/
/***  Driver Functions  ******************************************************/
/*****************************************************************************/

/**
 * Initialize I2C driver
 */
void i2c_init() {
  // initialize internal variables
  i2c_state = stIdle;

  // enable I2C interrupt
  EI2C = 1;
}

/**
 * Initiate an I2C read transfer
 *
 *
 */
I2C_Status i2c_start_read (uint8_t addr, uint8_t length, __xdata uint8_t* ptr) {
  // wait previous transfer to finish
  i2c_wait_stop();
  // return if a transfer is still active
  if (i2c_state != stIdle)
    return I2C_BUSY;

  // set the start bit and send address byte
  I2CS  = I2C_START;
  I2DAT = (addr << 1) | 0x01;   // LSB=1 -> read transfer
  // store information about the transfer
  i2c_state  = stRecvFirst;
  i2c_length = length;
  i2c_ptr    = ptr;
  i2c_count  = 0;
  
  return I2C_OK;
}

/**
 * Initiate an I2C write transfer
 *
 *
 */
I2C_Status i2c_start_write(uint8_t addr, uint8_t length, __xdata uint8_t* ptr) {
  // wait previous transfer to finish
  i2c_wait_stop();
  // return if a transfer is still active
  if (i2c_state != stIdle)
    return I2C_BUSY;

  // set the start bit and send address byte
  I2CS  = I2C_START;
  I2DAT = (addr << 1) | 0x00;   // LSB=0 -> write transfer
  // store information about the transfer
  i2c_state  = stSending;
  i2c_length = length;
  i2c_ptr    = ptr;
  i2c_count  = 0;
  
  return I2C_OK;
}

/**
 * Perform an I2C read transfer
 *
 * This function initiates an I2C read transfer and waits until it has
 * finished.
 */
I2C_Status i2c_read (uint8_t addr, uint8_t length, __xdata uint8_t* ptr) {
  i2c_start_read(addr,length,ptr);
  return i2c_wait_finished();
}

/**
 * Perform an I2C write transfer
 *
 * This function initiates an I2C write transfer and waits until it has
 * finished.
 */
I2C_Status i2c_write(uint8_t addr, uint8_t length, __xdata uint8_t* ptr) {
  i2c_start_write(addr,length,ptr);
  return i2c_wait_finished();
}

/*****************************************************************************/
/***  Internal Functions  ****************************************************/
/*****************************************************************************/

/**
 * Wait until the current transfer is finished.
 *
 * This function assumes that currently the last byte is transfered and the
 * I2C core will generate an I2C Stop condition after that byte. It waits until
 * the I2C core signals that the stop condition is done.
 */
static void i2c_wait_stop() {
  while (I2CS & I2C_STOP) ;
}

/**
 * Wait until the current transfer is finished and return its status
 */
static I2C_Status i2c_wait_finished() {
  while (true) {
    switch (i2c_state) {
      case stIdle:
        return I2C_OK;
      case stBusError:
        i2c_state = stIdle;
        return I2C_BERROR;
      case stNAck:
        i2c_state = stIdle;
        return I2C_NACK;
      default:
        // do nothing, just continue waiting
    }
  }
}

/*****************************************************************************/
/***  Interrupt Service Routine  *********************************************/
/*****************************************************************************/

/**
 * Interrupt Service Routine
 *
 * see EZ-USB Technical Reference Manual v1.10 p. 4-10.
 */
void i2c_isr(void)      __interrupt I2C_VECTOR {
  // check for bus error
  if (I2CS & BERR) {
    i2c_state = stBusError;
    // terminate transfer
    I2CS |= I2C_STOP;
    goto isr_done;
  }
  // check for missing NACK
  if ((i2c_state != stReceiving) && (!(I2CS & ACK))) {
    i2c_state = stNAck;
    // terminate transfer
    I2CS |= I2C_STOP;
    goto isr_done;
  }
  // I2C state machine
  switch (i2c_state) {
    case stRecvFirst:
      // for a 1-byte read, tell the I2C master to not acknowledge to tell the
      // slave its the last byte
      if (i2c_length == 1)
        I2CS |= LASTRD;
      // read from I2DAT and discard the value -> initiate first burst of 9 SCL
      // pulses to clock in the first byte from the slave
      if (I2DAT) ;
      i2c_state = stReceiving;
      break;
    case stReceiving:
      // set LASTRD for the second-last byte
      if (i2c_count == i2c_length-2)
        I2CS |= LASTRD;
      // set STOP bit for the last byte -> will generate I2C stop condition
      // after it was received
      if (i2c_count == i2c_length-1) {
        I2CS |= I2C_STOP;
        i2c_state = stIdle;
      }
      // store the received byte
      i2c_ptr[i2c_count++] = I2DAT;
      break;
    case stSending:
      // send next byte
      I2DAT = i2c_ptr[i2c_count++];
      // if last byte was sent, next state is stop
      if (i2c_count == i2c_length)
        i2c_state = stStop;
      break;
    case stStop:
      // tell I2C master to generate I2C stop condition
      I2CS |= I2C_STOP;
      i2c_state = stIdle;
      break;
  }
isr_done:
  EXIF &= ~I2CINT;  // clear interrupt flag
}

