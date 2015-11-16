// ==================================================================
// @(#)spi.h
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

#ifndef __BUSPIRATE_SPI_H__
#define __BUSPIRATE_SPI_H__

#include <buspirate.h>

#define BP_BIN_SPI_SPEED_30K  0
#define BP_BIN_SPI_SPEED_125K 1
#define BP_BIN_SPI_SPEED_250K 2
#define BP_BIN_SPI_SPEED_1M   3
#define BP_BIN_SPI_SPEED_2M   4
/* Note : as of BP firmware 5.10, the speeds equal and above 2.6MHz
   are not correctly setup by the BP firmware. The SPI1CON1 primary
   prescaler is incorrect. A bug has been issued on the BP firmware
   but this has not been resolved yet. */
#define BP_BIN_SPI_SPEED_2_6M 5
#define BP_BIN_SPI_SPEED_4M   6
#define BP_BIN_SPI_SPEED_8M   7

extern const char * BP_BIN_SPI_SPEEDS[];

#define BP_BIN_SPI_LV_3V3        0x08
#define BP_BIN_SPI_LV_HIZ        0x00

#define BP_BIN_SPI_CLK_IDLE_HIGH 0x04
#define BP_BIN_SPI_CLK_IDLE_LOW  0x00

#define BP_BIN_SPI_CLK_EDGE_HIGH 0x02
#define BP_BIN_SPI_CLK_EDGE_LOW  0x00

#define BP_BIN_SPI_SMP_END       0x01
#define BP_BIN_SPI_SMP_MIDDLE    0x00

#define BP_BIN_SPI_VERSION     0x01
#define BP_BIN_SPI_CS_LOW      0x02
#define BP_BIN_SPI_CS_HIGH     0x03
#define BP_BIN_SPI_BULK        0x10
#define BP_BIN_SPI_SET_PERIPH  0x40
#define BP_BIN_SPI_SET_SPEED   0x60
#define BP_BIN_SPI_SET_CONFIG  0x80
/* --- DEPRECATED ---
   Those commands have been removed brom the BP firmware

   #define BP_BIN_SPI_GET_PERIPH  0x50
   #define BP_BIN_SPI_GET_SPEED   0x70
   #define BP_BIN_SPI_GET_CONFIG  0x90
*/

#define BP_BIN_SPI_PERIPH_POWER   0x08
#define BP_BIN_SPI_PERIPH_PULLUPS 0x04
#define BP_BIN_SPI_PERIPH_AUX     0x02
#define BP_BIN_SPI_PERIPH_CS      0x01

#ifdef __cplusplus
extern "C" {
#endif

  int bp_bin_spi_version(BP * bp, unsigned char * version);
  int bp_bin_spi_set_config(BP * bp, unsigned char config);
  int bp_bin_spi_set_speed(BP * bp, unsigned char speed);
  int bp_bin_spi_set_periph(BP * bp, unsigned char config);
  int bp_bin_spi_cs(BP * bp, int enabled);

  int bp_bin_spi_bulk(BP * bp, unsigned char * data, unsigned char nlen);

  /* --- DEPRECATED ---
     The commands required to implement those functions
     have been removed from the BP firmware.

     int bp_bin_spi_get_speed(BP * bp, unsigned char * speed);
     int bp_bin_spi_get_config(BP * bp, unsigned char * config);
     int bp_bin_spi_get_periph(BP * bp, unsigned char * config);
  */


#ifdef __cplusplus
}
#endif

#endif /* __BUSPIRATE_SPI_H__ */
