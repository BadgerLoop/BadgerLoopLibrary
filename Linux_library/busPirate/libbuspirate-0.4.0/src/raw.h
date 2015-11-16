// ==================================================================
// @(#)raw.h
//
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 01/08/2014
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

#ifndef __BUSPIRATE_RAW_H__
#define __BUSPIRATE_RAW_H__

#include <buspirate.h>

#define BP_BIN_RAW_VERSION          0x01
#define BP_BIN_RAW_I2C_START_STOP   0x02
#define BP_BIN_RAW_CS_LOW_HIGH      0x04
#define BP_BIN_RAW_READ_BYTE        0x05
#define BP_BIN_RAW_READ_BIT         0x07
#define BP_BIN_RAW_PEEK             0x08
#define BP_BIN_RAW_CLOCK_TICK       0x09
#define BP_BIN_RAW_CLOCK_LOW_HIGH   0x0A
#define BP_BIN_RAW_DATA_LOW_HIGH    0x0C
#define BP_BIN_RAW_BULK_BYTES       0x10
#define BP_BIN_RAW_BULK_CLOCK_TICKS 0x20
#define BP_BIN_RAW_BULK_BITS        0x30 /* only in version >= 4.5 */
#define BP_BIN_RAW_SET_PERIPH       0x40
#define BP_BIN_RAW_SET_SPEED        0x60
#define BP_BIN_RAW_SET_CONFIG       0x80

#define BP_BIN_RAW_SPEED_5K   0
#define BP_BIN_RAW_SPEED_10K  1
#define BP_BIN_RAW_SPEED_100K 2
#define BP_BIN_RAW_SPEED_400K 3


int bp_bin_raw_version(BP * bp, unsigned char * version);
int bp_bin_raw_set_periph(BP * bp, unsigned char config);
int bp_bin_raw_set_speed(BP * bp, unsigned char speed);
int bp_bin_raw_set_config(BP * bp, unsigned char config);

int bp_bin_raw_cs(BP * bp, int enabled);
int bp_bin_raw_bulk_bytes(BP * bp, unsigned char * data, unsigned char len);
int bp_bin_raw_bulk_bits(BP * bp, unsigned char bits, unsigned char len);


#endif /* __BUSPIRATE_RAW_H__ */
