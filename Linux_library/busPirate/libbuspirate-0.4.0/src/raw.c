// ==================================================================
// @(#)raw.c
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <buspirate.h>
//#define DEBUG
#define DEBUG_STREAM stderr
#define DEBUG_ID     "BP:RAW:"
#include <debug.h>
#include <raw.h>

// ------------------------------------------------------------------
/**
 * RAW WIRE: read version.
 */
int bp_bin_raw_version(BP * bp, unsigned char * version)
{
  __debug__("BP_BIN_RAW_VERSION\n");
  _bp_check_state(bp, BP_STATE_BIN_RAW);

  unsigned char rbuf[4];
  if (_bp_bin_write_read(bp, BP_BIN_RAW_VERSION, rbuf, 4) != BP_SUCCESS)
    return BP_FAILURE;
  if (memcmp(rbuf, "RAW", 3) != 0)
    return BP_FAILURE;
  if ((rbuf[3] < '0') || (rbuf[3] > '9'))
    return BP_FAILURE;
  if (version != NULL)
    *version= rbuf[3]-'0';
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * RAW WIRE: set peripheral configuration.
 *
 *   bit 3 : power
 *   bit 2 : pullups
 *   bit 1 : AUX
 *   bit 0 : CS
 */
int bp_bin_raw_set_periph(BP * bp, unsigned char config)
{
  __debug__("BP_BIN_RAW_SET_PERIPH(0x%.2X)\n", config);
  _bp_check_state(bp, BP_STATE_BIN_RAW);
  assert(config <= 15);

  unsigned char rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_RAW_SET_PERIPH | config, rbuf, 1)
      != BP_SUCCESS)
    return BP_FAILURE;
  if (rbuf[0] != 0x01)
    return BP_FAILURE;
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * RAW WIRE: set speed.
 *
 *   0 : ~5kHz
 *   1 : ~10kHz
 *   2 : ~100kHz
 *   3 : ~400kHz
 */
int bp_bin_raw_set_speed(BP * bp, unsigned char speed)
{
  __debug__("BP_BIN_RAW_SET_SPEED\n");
  _bp_check_state(bp, BP_STATE_BIN_RAW);
  assert(speed <= 3);

  unsigned char  rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_RAW_SET_SPEED | speed, rbuf, 1)
      != BP_SUCCESS)
    return BP_FAILURE;
  __debug__("SET SPEED answer=%u\n", rbuf[0]);
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * RAW WIRE: set config.
 *
 *   bit 3 : HiZ (0) / 3.3V (1)
 *   bit 2 : 2 (0) /3 (1) wire
 *   bit 1 : MSB (0) / LSB (1)
 *   bit 0 : not used
 */
int bp_bin_raw_set_config(BP * bp, unsigned char config)
{
  __debug__("BP_BIN_RAW_SET_CONFIG(0x%.2X)\n", config);
  _bp_check_state(bp, BP_STATE_BIN_RAW);
  assert(config <= 15);

  unsigned char rbuf[1];
  if (_bp_bin_write_read(bp, BP_BIN_RAW_SET_CONFIG | config, rbuf, 1)
      != BP_SUCCESS)
    return BP_FAILURE;
  return BP_SUCCESS;
}

static int _bp_bin_raw_check_response(BP * bp)
{
  unsigned char rbuf;
  if (bp_read(bp, &rbuf, 1) != 1)
    return BP_FAILURE;
  if (rbuf != 0x01)
    return BP_FAILURE;
  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * RAW WIRE: CS
 */
int bp_bin_raw_cs(BP * bp, int enabled)
{
  __debug__("BP_BIN_RAW_CS(%d)\n", enabled?1:0);
  _bp_check_state(bp, BP_STATE_BIN_RAW);

  unsigned char data= BP_BIN_RAW_CS_LOW_HIGH;
  if (enabled)
    data|= 0x01;
  if (bp_write(bp, &data, 1) != BP_SUCCESS)
    return BP_FAILURE;

  return _bp_bin_raw_check_response(bp);
}

// ------------------------------------------------------------------
/**
 * RAW WIRE: bulk transfer of 1-16 bytes.
 */
int bp_bin_raw_bulk_bytes(BP * bp, unsigned char * data, unsigned char len)
{
  __debug__("BP_BIN_RAW_BULK_BYTES(0x%.2X)\n", len);
  _bp_check_state(bp, BP_STATE_BIN_RAW);
  assert((len >= 1) && (len <= 16));

  unsigned char wbuf[17];
  wbuf[0]= BP_BIN_RAW_BULK_BYTES | (len-1);
  memcpy(&wbuf[1], data, len);

  int i;
  __debug__("  sent data:");
  for (i= 0; i < len+1; i++)
    __debug_more__(" 0x%.2X", wbuf[i]);
  __debug_more__("\n");

  if (bp_write(bp, wbuf, len+1) != BP_SUCCESS)
    return BP_FAILURE;
  if (bp_read(bp, wbuf, len+1) != BP_SUCCESS)
    return BP_FAILURE;

  __debug__("  read data:");
  for (i= 0; i < len+1; i++)
    __debug_more__(" 0x%.2X", wbuf[i]);
  __debug_more__("\n");

  /*if (wbuf[0] != 0x01)
    return BP_FAILURE;
    memcpy(data, &wbuf[1], nlen);*/

  return BP_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * RAW WIRE: bulk transfer of 1-8 bits.
 */
int bp_bin_raw_bulk_bits(BP * bp, unsigned char bits, unsigned char len)
{
  __debug__("BP_BIN_RAW_BULK_BITS(0x%.2X)\n", len);
  _bp_check_state(bp, BP_STATE_BIN_RAW);
  assert((len >= 1) && (len <= 8));

  unsigned char wbuf[17];
  wbuf[0]= BP_BIN_RAW_BULK_BITS | (len-1);
  wbuf[1]= bits;

  if (bp_write(bp, wbuf, 2) != BP_SUCCESS)
    return BP_FAILURE;
  if (bp_read(bp, wbuf, 2) != BP_SUCCESS)
    return BP_FAILURE;

  if ((wbuf[0] != 0x01) || (wbuf[1] != 0x01))
    return BP_FAILURE;

  return BP_SUCCESS;
}
