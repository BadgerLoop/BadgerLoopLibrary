// ==================================================================
// @(#)ht1632.c
//
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 28/07/2014
// $Id$
//
// libbuspirate
// Copyright (C) 2014 Bruno Quoitin
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

#include <stdio.h>
#include <string.h>
#include <ht1632.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <raw.h>

#define HT1632_SYS_DIS 0
#define HT1632_SYS_EN  1
#define HT1632_LED_OFF 2
#define HT1632_LED_ON  3
#define HT1632_BLINK_OFF 8
#define HT1632_BLINK_ON  9
#define HT1632_RC_CLK  24
#define HT1632_COMMONS_OPTIONS_PMOS_16 44
#define HT1632_PWM       160

#define HT1632_SPEED BP_BIN_RAW_SPEED_400K
#define HT1632_FRAME_SIZE (16 * 24 / 4)

int ht1632_init(BP * bp)
{
  unsigned char version;

  if (bp_bin_mode_raw(bp, &version) != BP_SUCCESS)
    return -1;
  printf("Version : %d\n", version);

  /*if (bp_bin_raw_version(bp, &version) != BP_SUCCESS)
    return -1;
    printf("Version : %d\n", version);*/
  
  if (bp_bin_raw_set_speed(bp, HT1632_SPEED) != BP_SUCCESS)
    return -1;

  unsigned char config= 0x00; /* HiZ + 2wire + MSB */
  if (bp_bin_raw_set_config(bp, config) != BP_SUCCESS)
    return -1;

  unsigned char periph= 0x0C; /* power + pullups */
  if (bp_bin_raw_set_periph(bp, periph) != BP_SUCCESS)
    return -1;

  return 0;
}

int ht1632_command(BP * bp, unsigned char cmd)
{
  if (bp_bin_raw_cs(bp, 0) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_bulk_bits(bp, 0x80, 3) != BP_SUCCESS)
    return -1;
  
  if (bp_bin_raw_bulk_bytes(bp, &cmd, 1) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_bulk_bits(bp, 0x0, 1) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_cs(bp, 1) != BP_SUCCESS)
    return -1;

  return 0;
}

int ht1632_write(BP * bp, unsigned char addr, unsigned char data)
{
  if (bp_bin_raw_cs(bp, 0) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_bulk_bits(bp, 0xA0, 3) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_bulk_bits(bp, addr << 1, 7) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_bulk_bits(bp, data << 4, 4) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_cs(bp, 1) != BP_SUCCESS)
    return -1;

  return 0;
}

int ht1632_mwrite(BP * bp, unsigned char start_addr,
		  unsigned char * data, unsigned char len)
{
  if (bp_bin_raw_cs(bp, 0) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_bulk_bits(bp, 0xA0, 3) != BP_SUCCESS)
    return -1;

  if (bp_bin_raw_bulk_bits(bp, start_addr << 1, 7) != BP_SUCCESS)
    return -1;

  /* Try to group data in as few writes as possible */
  unsigned char bytes[16];
  int pos= 0;
  while (len >= 2) {
    bytes[pos++]= ((*data) << 4) | *(data+1);
    data+= 2;
    len-= 2;
    if ((pos == 16) || (len == 0)) {
      if (bp_bin_raw_bulk_bytes(bp, bytes, pos) != BP_SUCCESS)
	return -1;
      pos= 0;
    }
  }
  /* Final 4-bits nibble required ? */
  if (len > 0)
    if (bp_bin_raw_bulk_bits(bp, (*data) << 4, 4) != BP_SUCCESS)
      return -1;

  if (bp_bin_raw_cs(bp, 1) != BP_SUCCESS)
    return -1;

  return 0;
}

int demo_ht1632(BP * bp, int argc, char * argv[])
{
  printf("*** HT1632 demo ***\n");
  printf("This demo interfaces with an Holtek HT1632 based LED display.\n");

  if (ht1632_init(bp) < 0)
    return -1;

  if (ht1632_command(bp, HT1632_SYS_DIS) < 0)
    return 0;
  if (ht1632_command(bp, HT1632_RC_CLK) < 0)
    return 0;
  if (ht1632_command(bp, HT1632_COMMONS_OPTIONS_PMOS_16) < 0)
    return 0;
  if (ht1632_command(bp, HT1632_BLINK_OFF) < 0)
    return 0;
  if (ht1632_command(bp, HT1632_SYS_EN) < 0)
    return 0;
  if (ht1632_command(bp, HT1632_LED_ON) < 0)
    return 0;
  if (ht1632_command(bp, HT1632_PWM | 0xF) < 0)
    return 0;
  
  /* Full frame size = 16 * 24 LEDs in groups of 4 -> 96 nibbles */
  unsigned char frame_buffer[HT1632_FRAME_SIZE];
  int toggle= 0;
  while (loop) {
    
    memset(frame_buffer, toggle?0x0A:0x05, HT1632_FRAME_SIZE);
    toggle= !toggle;
    if (ht1632_mwrite(bp, 0, frame_buffer, HT1632_FRAME_SIZE) < 0)
      return -1;
#ifdef _WIN32
    Sleep(1000);
#else
    usleep(1000000);
#endif
  }

  return 0;
}
