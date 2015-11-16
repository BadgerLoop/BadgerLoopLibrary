// ==================================================================
// @(#)ds1324.h
//
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 18/09/2010
// $Id$
//
// libbuspirate
// Copyright (C) 2010 Bruno Quoitin
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
// 02111-1307  USA
// ==================================================================

#ifndef __BUSPIRATE_DS1624_H__
#define __BUSPIRATE_DS1624_H__

#include <buspirate.h>

#define DS1624_ADDR_PREFIX       0x90

#define DS1624_CMD_STOP_CONVERT  0x22
#define DS1624_CMD_READ_TEMP     0xAA
#define DS1624_CMD_START_CONVERT 0xEE

#define DS1631_ADDR_PREFIX       0x90

#define DS1631_CMD_STOP_CONVERT  0x22
#define DS1631_CMD_READ_TEMP     0xAA
#define DS1631_CMD_START_CONVERT 0x51

#ifdef __cplusplus
extern "C" {
#endif

  int ds1624_demo(BP * bp, int argc, char * argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __BUSPIRATE_DS1624_H__ */
