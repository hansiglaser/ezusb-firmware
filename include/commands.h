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

#ifndef __COMMANDS_H
#define __COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Command definition
 */
#define CMD_GET_VERSION          0x80
#define CMD_GET_VERSION_STRING   0x81
#define CMD_GET_STATUS           0x82
// ... add further commands here and handlers in HandleCmd() in commands.c ...
// 0xA0 .. 0xAF are reserved by Anchor / Cypress

/* Command: GetVersion ******************************************************/
typedef struct {
  uint16_t Firmware;     // Firmware Version
  // ... add further fields, e.g. version of linked libraries, external
  // devices, ... and fill these fields in GetVersion() in commands.c ...
} TGetVersion;

#define FIRMWARE_VERSION 0x0001   // 0x00 . 0x01 -> 0.1

/* Command: GetStatus *******************************************************/
typedef struct {
  uint8_t  MyStatus;     // dummy field
  // ... add further fields with various status information and fill these
  // fields in GetStatus() in commands.c ...
} TGetStatus;

/* Common *******************************************************************/

void command_loop(void);

#endif  // __COMMANDS_H
