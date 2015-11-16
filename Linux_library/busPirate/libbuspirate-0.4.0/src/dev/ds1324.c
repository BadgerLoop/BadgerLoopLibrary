// ==================================================================
// @(#)ds1324.c
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
#include <config.h>
#endif

#include <ds1324.h>
#include <i2c.h>

#ifdef __BUSPIRATE__
#include <stdio.h>
#include <unistd.h>
#endif /* __BUSPIRATE__ */

#include <assert.h>

static int _i2c_init(BP * bp)
{
  unsigned char version;

  if (bp_bin_mode_i2c(bp, &version) != BP_SUCCESS)
    return -1;

  if (bp_bin_i2c_set_speed(bp, BP_BIN_I2C_SPEED_5K) != BP_SUCCESS)
    return -1;

  if (bp_bin_i2c_set_periph(bp, (BP_BIN_I2C_PERIPH_POWER |
				 BP_BIN_I2C_PERIPH_PULLUPS)) != BP_SUCCESS)
    return -1;

  return 0;
}

static int _i2c_cmd(BP * bp, unsigned char addr, unsigned char cmd)
{
  unsigned char ack;

  if (bp_bin_i2c_start(bp) != BP_SUCCESS)
    return -1;
  if (bp_bin_i2c_write(bp, addr, &ack) != BP_SUCCESS)
    return -1;
  if (ack != BP_BIN_I2C_ACK) /* ACK expected */
    return -1;
  if (bp_bin_i2c_write(bp, cmd, &ack) != BP_SUCCESS)
    return -1;
  if (ack != BP_BIN_I2C_ACK) /* ACK expected */
    return -1;
  if (bp_bin_i2c_stop(bp) != BP_SUCCESS)
    return -1;
  return 0;
}

static int _i2c_read(BP * bp, unsigned char addr, unsigned char cmd,
		     unsigned char * rbuf, size_t rbuf_len)
{
  unsigned char ack;
  int i;
  if (bp_bin_i2c_start(bp) != BP_SUCCESS)
    return -1;
  if (bp_bin_i2c_write(bp, addr, &ack) != BP_SUCCESS)
    return -1;
  if (ack != BP_BIN_I2C_ACK) /* ACK expected */
    return -1;
  if (bp_bin_i2c_write(bp, cmd, 0) != BP_SUCCESS)
    return -1;
  if (ack != BP_BIN_I2C_ACK) /* ACK expected */
    return -1;
  if (bp_bin_i2c_start(bp) != BP_SUCCESS)
    return -1;
  if (bp_bin_i2c_write(bp, addr | 0x01, &ack) != BP_SUCCESS)
    return -1;
  if (ack != BP_BIN_I2C_ACK) /* ACK expected */
    return -1;
  for (i= 0; i < rbuf_len; i++) {
    if (bp_bin_i2c_read(bp, &rbuf[i]) != BP_SUCCESS)
      return -1;
    if (i+1 == rbuf_len) {
      if (bp_bin_i2c_nack(bp) != BP_SUCCESS)
	return -1;
    } else {
      if (bp_bin_i2c_ack(bp) != BP_SUCCESS)
	return -1;
    }
  }
  if (bp_bin_i2c_stop(bp) != BP_SUCCESS)
    return -1;
  return 0;
}

int ds1624_start_convert(BP * bp, unsigned char addr)
{
  assert(addr <= 7);
  return _i2c_cmd(bp, DS1624_ADDR_PREFIX | (addr << 1),
		  DS1624_CMD_START_CONVERT);
}

int ds1624_stop_convert(BP * bp, unsigned char addr)
{
  assert(addr <= 7);
  return _i2c_cmd(bp, DS1624_ADDR_PREFIX | (addr << 1),
		  DS1624_CMD_STOP_CONVERT);
}

int ds1624_read_temp(BP * bp, unsigned char addr, unsigned short * temp)
{
  unsigned char rbuf[2];
  assert(addr <= 7);
  if (_i2c_read(bp, DS1624_ADDR_PREFIX | (addr << 1),
		DS1624_CMD_READ_TEMP, rbuf, 2) < 0)
    return -1;
  if (temp != NULL)
    *temp= (rbuf[0] << 8) | rbuf[1];
  return 0;
}

int ds1631_start_convert(BP * bp, unsigned char addr)
{
  assert(addr <= 7);
  return _i2c_cmd(bp, DS1631_ADDR_PREFIX | (addr << 1),
		  DS1631_CMD_START_CONVERT);
}

int ds1631_stop_convert(BP * bp, unsigned char addr)
{
  assert(addr <= 7);
  return _i2c_cmd(bp, DS1631_ADDR_PREFIX | (addr << 1),
		  DS1631_CMD_STOP_CONVERT);
}

int ds1631_read_temp(BP * bp, unsigned char addr, unsigned short * temp)
{
  unsigned char rbuf[2];
  assert(addr <= 7);
  if (_i2c_read(bp, DS1631_ADDR_PREFIX | (addr << 1),
		DS1631_CMD_READ_TEMP, rbuf, 2) < 0)
    return -1;
  if (temp != NULL)
    *temp= (rbuf[0] << 8) | rbuf[1];
  return 0;
}

#ifdef __BUSPIRATE__
int ds1624_demo(BP * bp, int argc, char * argv[])
{
  unsigned char ds1624_addr= 0x00;
  unsigned char ds1631_addr= 0x01;
  double max_temp= 0, min_temp=200, cur_temp;

  printf("Talking to DS1624/31 over I2C\n");

  if (_i2c_init(bp) < 0)
    return -1;

  if (ds1624_start_convert(bp, ds1624_addr) < 0)
    return -1;
  if (ds1631_start_convert(bp, ds1631_addr) < 0)
    return -1;

  while (loop) {
    
    usleep(1000000);

    unsigned short ds1624_temp, ds1631_temp;
    if (ds1624_read_temp(bp, ds1624_addr, &ds1624_temp) < 0)
      return -1;
    if (ds1631_read_temp(bp, ds1631_addr, &ds1631_temp) < 0)
      return -1;
    cur_temp= ((double) (ds1624_temp >> 3))/32;
    if (cur_temp < min_temp)
      min_temp= cur_temp;
    if (cur_temp > max_temp)
      max_temp= cur_temp;
    printf("\rTemperature = %.4f (max=%.4f, min=%.4f) ; %.4X ; %.4X ; %.5f",
	   cur_temp, max_temp, min_temp, ds1624_temp, ds1631_temp,
	   ((double) (ds1631_temp >> 4))/16);
    fflush(stdout);

  }
  printf("\n");

  return 0;
}
#endif /* __BUSPIRATE__ */

