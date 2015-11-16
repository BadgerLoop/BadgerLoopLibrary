// ==================================================================
// @(#)spi.c
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <buspirate.h>
//#define DEBUG
#define DEBUG_STREAM stderr
#define DEBUG_ID     "BP:SPI:"
#include <debug.h>
#include <spi.h>

const char * BP_BIN_SPI_SPEEDS[8]=
  { "31.5kHz", "125kHz", "250kHz", "1MHz", "2MHz", "2.6MHz", "4MHz", "8MHz" };


// ------------------------------------------------------------------
/**
 * SPI: read version.
 */
int bp_bin_spi_version(BP * bp, unsigned char * version)
{
  __debug__("BP_BIN_SPI_VERSION\n");
  _bp_check_state(bp, BP_STATE_BIN_SPI);

  unsigned char rbuf[4];
  if (_bp_bin_write_read(bp, BP_BIN_SPI_VERSION, rbuf, 4) != BP_SUCCESS)
    return BP_FAILURE;
  if (memcmp(rbuf, "SPI", 3) != 0)
    return BP_FAILURE;
  if ((rbuf[3] < '0') || (rbuf[3] > '9'))
    return BP_FAILURE;
  if (version != NULL)
    *version= rbuf[3]-'0';
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * SPI: get speed.
 * --- DEPRECATED ---
 */
/*int bp_bin_spi_get_speed(BP * bp, unsigned char * speed)
{
  __debug__("BP_BIN_SPI_GET_SPEED\n");
  _bp_check_state(bp, BP_STATE_BIN_SPI);

  unsigned char  rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_SPI_GET_SPEED, rbuf, 1) != BP_SUCCESS)
    return BP_FAILURE;
  if (speed != NULL)
    *speed= rbuf[0] & 0x07;
  return BP_SUCCESS;
}
*/

// ------------------------------------------------------------------
/**
 * SPI: set speed.
 */
int bp_bin_spi_set_speed(BP * bp, unsigned char speed)
{
  __debug__("BP_BIN_SPI_SET_SPEED\n");
  _bp_check_state(bp, BP_STATE_BIN_SPI);
  assert(speed <= 7);

  unsigned char  rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_SPI_SET_SPEED | speed, rbuf, 1)
      != BP_SUCCESS)
    return BP_FAILURE;
  __debug__("SET SPEED answer=%u\n", rbuf[0]);
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * SPI: get config.
 * --- DEPRECATED ---
 */
/*
int bp_bin_spi_get_config(BP * bp, unsigned char * config)
{
  __debug__("BP_BIN_SPI_GET_CONFIG()\n", config);
  _bp_check_state(bp, BP_STATE_BIN_SPI);

  unsigned char  rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_SPI_GET_CONFIG, rbuf, 1) != BP_SUCCESS)
    return BP_FAILURE;
  if (config != NULL)
    *config= rbuf[0] & 0x0F;
  return BP_SUCCESS;
}
*/

// ------------------------------------------------------------------
/**
 * SPI: set config.
 */
int bp_bin_spi_set_config(BP * bp, unsigned char config)
{
  __debug__("BP_BIN_SPI_SET_CONFIG(0x%.2X)\n", config);
  _bp_check_state(bp, BP_STATE_BIN_SPI);
  assert(config <= 15);

  unsigned char rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_SPI_SET_CONFIG | config, rbuf, 1)
      != BP_SUCCESS)
    return BP_FAILURE;
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * SPI: read peripheral configuration.
 * --- DEPRECATED ---
 */
/*
int bp_bin_spi_get_periph(BP * bp, unsigned char * config)
{
  __debug__("BP_BIN_SPI_GET_PERIPH()\n", config);
  _bp_check_state(bp, BP_STATE_BIN_SPI);

  unsigned char rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_SPI_GET_PERIPH, rbuf, 1) != BP_SUCCESS)
    return BP_FAILURE;


  __debug__("*** THE VALUE = 0x%.2X ***\n", rbuf[0]);
  
  // It looks like this command does not work properly anymore with
  // newer versions of the Bus Pirate firmware. It used to work with
  // version 2.4 -> returned (0x50 | config). Seems to be broken with
  // version 4.2 -> returns 0x00

  //   Following test (temporarily?) disabled
  //if ((rbuf[0] & 0xF0) != BP_BIN_SPI_GET_PERIPH)
  //  return BP_FAILURE;
  if (config != NULL)
    *config= rbuf[0] & 0x0F;
  return BP_SUCCESS;
}
*/

// ------------------------------------------------------------------
/**
 * SPI: write peripheral configuration.
 *
 * config is a bitwise OR of the following values
 *   BP_BIN_SPI_PERIPH_POWER   : to enable power supply
 *   BP_BIN_SPI_PERIPH_PULLUPS : to enable pullups (need to power Vpu pin)
 *   BP_BIN_SPI_PERIPH_AUX     : AUX pin
 *   BP_BIN_SPI_PERIPH_CS      : CS pin
 */
int bp_bin_spi_set_periph(BP * bp, unsigned char config)
{
  __debug__("BP_BIN_SPI_SET_PERIPH(0x%.2X)\n", config);
  _bp_check_state(bp, BP_STATE_BIN_SPI);
  assert(config <= 15);

  unsigned char rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_SPI_SET_PERIPH | config, rbuf, 1)
      != BP_SUCCESS)
    return BP_FAILURE;
  if (rbuf[0] != 0x01)
    return BP_FAILURE;
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * SPI: bulk read/write.
 */
int bp_bin_spi_bulk(BP * bp, unsigned char * data, unsigned char nlen)
{
  __debug__("BP_BIN_SPI_BULK\n");
  _bp_check_state(bp, BP_STATE_BIN_SPI);
  assert((nlen > 0) && (nlen <= 16));
  
  unsigned char wbuf[17];
  wbuf[0]= BP_BIN_SPI_BULK | (nlen-1);
  memcpy(&wbuf[1], data, nlen);

  __debug__("  sent data:");
  __debug_more_hex_buf__(wbuf, nlen+1);
  __debug_more__("\n");

  if (bp_write(bp, wbuf, nlen+1) != BP_SUCCESS)
    return BP_FAILURE;
  if (bp_read(bp, wbuf, nlen+1) != BP_SUCCESS)
    return BP_FAILURE;

  __debug__("  read data:");
  __debug_more_hex_buf__(wbuf, nlen+1);
  __debug_more__("\n");

  if (wbuf[0] != 0x01)
    return BP_FAILURE;
  memcpy(data, &wbuf[1], nlen);
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * SPI: enable/disable CS.
 */
int bp_bin_spi_cs(BP * bp, int enabled)
{
  __debug__("BP_BIN_SPI_CS\n");
  _bp_check_state(bp, BP_STATE_BIN_SPI);

  unsigned char data;
  if (enabled)
    data= BP_BIN_SPI_CS_HIGH;
  else
    data= BP_BIN_SPI_CS_LOW;
  if (bp_write(bp, &data, 1) != BP_SUCCESS)
    return BP_FAILURE;

  if ((bp_firmware_version_high(bp) == 2) &&
      (bp_firmware_version_low(bp) == 4)) {
    /*BUG: version 2.4 does not respond with 0x01 */
  } else {
    unsigned char rbuf;
    if (bp_read(bp, &rbuf, 1) != 1)
      return -1;
    if (rbuf != 0x01)
      return -1;
  }
  
  return BP_SUCCESS;
}
