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

#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>

typedef enum {I2C_OK,I2C_BUSY,I2C_BERROR,I2C_NACK} I2C_Status;

void i2c_init();
I2C_Status i2c_start_read (uint8_t addr, uint8_t length, __xdata uint8_t* ptr);
I2C_Status i2c_start_write(uint8_t addr, uint8_t length, __xdata uint8_t* ptr);
I2C_Status i2c_read (uint8_t addr, uint8_t length, __xdata uint8_t* ptr);
I2C_Status i2c_write(uint8_t addr, uint8_t length, __xdata uint8_t* ptr);

#endif  // __I2C_H

