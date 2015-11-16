// ==================================================================
// @(#)24c02.c
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

#include <24c02.h>
#include <i2c.h>

#include <assert.h>

#ifdef __BUSPIRATE__
#include <stdio.h>
#include <unistd.h>
#endif /* __BUSPIRATE__ */

int _24c02_byte_write(BP * bp, unsigned char dev_addr,
		      unsigned char addr, unsigned char value)
{
  unsigned char ack;
  assert(dev_addr <= 7);

  if (bp_bin_i2c_start(bp) < 0)
    return -1;
  if (bp_bin_i2c_write(bp, _24C02_ADDR_PREFIX | (dev_addr << 1), &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;
  if (bp_bin_i2c_write(bp, addr, &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;  
  if (bp_bin_i2c_write(bp, value, &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;  
  if (bp_bin_i2c_stop(bp) < 0)
    return -1;

  return 0;
}

/* Note: according to some datasheets, the 24C01/02 only support
 *       8-byte page writes while 24C04, 24C08 and 24C16 support
 *       16-byte page writes.
 *       According to other datasheets 24C01 and 24C02 also support
 *       16-byte page writes. Experiments with Catalyst Semiconductor,
 *       Inc. (CSI) 24C02 EEPROM failed with 16-byte page writes. */
int _24c02_page_write(BP * bp, unsigned char dev_addr,
		      unsigned char addr, unsigned char * values,
		      size_t size)
{
  unsigned char ack;
  assert(dev_addr <= 7);
  assert((size > 0) && (size <= 8));

  if (bp_bin_i2c_start(bp) < 0)
    return -1;
  if (bp_bin_i2c_write(bp, _24C02_ADDR_PREFIX | (dev_addr << 1), &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;
  if (bp_bin_i2c_write(bp, addr, &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;
  while (size > 0) {
    if (bp_bin_i2c_write(bp, *values, &ack) < 0)
      return -1;
    if (ack != BP_BIN_I2C_ACK)
      return -1;
    values++;
    size--;
  }
  if (bp_bin_i2c_stop(bp) < 0)
    return -1;
  return 0;
}

int _24c02_random_read(BP * bp, unsigned char dev_addr,
		       unsigned char addr, unsigned char * value)
{
  unsigned char ack;
  assert(dev_addr <= 7);

  if (bp_bin_i2c_start(bp) < 0)
    return -1;
  if (bp_bin_i2c_write(bp, _24C02_ADDR_PREFIX | (dev_addr << 1), &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;
  if (bp_bin_i2c_write(bp, addr, &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;
  if (bp_bin_i2c_start(bp) < 0)
    return -1;
  if (bp_bin_i2c_write(bp, _24C02_ADDR_PREFIX | (dev_addr << 1)| 0x01, &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;  
  if (bp_bin_i2c_read(bp, value) < 0)
    return -1;
  if (bp_bin_i2c_nack(bp) < 0)
    return -1;
  if (bp_bin_i2c_stop(bp) < 0)
    return -1;

  return 0;
}

int _24c02_sequential_read(BP * bp, unsigned char dev_addr,
			   unsigned char addr, unsigned char * values,
			   size_t size)
{
  unsigned char ack;
  assert(dev_addr <= 7);
  assert((size > 1) && (size <= 256));

  if (bp_bin_i2c_start(bp) < 0)
    return -1;
  if (bp_bin_i2c_write(bp, _24C02_ADDR_PREFIX | (dev_addr << 1), &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;
  if (bp_bin_i2c_write(bp, addr, &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;
  if (bp_bin_i2c_start(bp) < 0)
    return -1;
  if (bp_bin_i2c_write(bp, _24C02_ADDR_PREFIX | (dev_addr << 1)| 0x01, &ack) < 0)
    return -1;
  if (ack != BP_BIN_I2C_ACK)
    return -1;  
  while (size > 0) {
    if (bp_bin_i2c_read(bp, values) < 0)
      return -1;
    if (size == 1) {
      if (bp_bin_i2c_nack(bp) < 0)
	return -1;
    } else {
      if (bp_bin_i2c_ack(bp) < 0)
	return -1;
    }
    values++;
    size--;
  }
  if (bp_bin_i2c_stop(bp) < 0)
    return -1;
  return 0;
}

/* Note: datasheet mentions that acknowledge polling can be performed
 *       by initiationg a write operation. If the device does not
 *       answer with an ACK, it means it is still busy. Otherwise, the
 *       write operation can go on. In our general case, we do not
 *       necessarily want to perform another write operation. Therefore,
 *       the acknowledge polling is performed with read operations. In
 *       case the device answers with an ACK, we complete the read but
 *       discard the value. */
int _24c02_ack_polling(BP * bp, unsigned char dev_addr)
{
  unsigned char ack, value;
  do {
    if (bp_bin_i2c_start(bp) < 0)
      return -1;
    if (bp_bin_i2c_write(bp, _24C02_ADDR_PREFIX | (dev_addr << 1) | 0x01, &ack) < 0)
      return -1;
    if (ack == BP_BIN_I2C_ACK) {
      if (bp_bin_i2c_read(bp, &value) < 0)
	return -1;
      if (bp_bin_i2c_nack(bp) < 0)
	return -1;
    }
    if (bp_bin_i2c_stop(bp) < 0)
      return -1;
  } while (ack == BP_BIN_I2C_NACK);
  return 0;
}

#ifdef __BUSPIRATE__
/* Setup
   
 */
int _24c02_demo(BP * bp, int argc, char * argv[])
{
  unsigned char version;
  unsigned char dev_addr= 0;
  int addr;
  //unsigned char value;

  if (bp_bin_mode_i2c(bp, &version) < 0)
    return -1;

  if (bp_bin_i2c_set_speed(bp, BP_BIN_I2C_SPEED_400K) < 0)
    return -1;

  if (bp_bin_i2c_set_periph(bp, (BP_BIN_I2C_PERIPH_POWER |
				 BP_BIN_I2C_PERIPH_PULLUPS)) < 0)
    return -1;

  //printf("256 byte writes...\n");
  //for (addr= 0; addr < 256; addr++) {
  //  if (_24c02_byte_write(bp, dev_addr, addr, addr) < 0)
  //    return -1;
  //  usleep(10000); /* Typical write time of 6ms ! */
  //}
  //
  //printf("256 byte reads...\n");
  //for (addr= 0; addr < 256; addr++) {
  //  if (_24c02_random_read(bp, dev_addr, addr, &value) < 0)
  //    return -1;
  //  if (value != addr)
  //    printf("Mismatch at addr=%.2X (%.2X != %.2X)\n",
  //           addr, addr, value);
  //}

  printf("Using default address 0xA0\n");
  printf("  (A0/A1/A2 tied to ground)\n");
  printf("32 8-byte page writes...\n");
  unsigned char mem[256];
  int p;
  for (addr= 0; addr < 256; addr++)
    mem[addr]= 0;
  for (p= 0; p < 32; p++) {
    if (_24c02_page_write(bp, dev_addr, p*8, &mem[p*8], 8) < 0)
      return -1;
    if (_24c02_ack_polling(bp, dev_addr) < 0)
      return -1;
  }

  printf("full-read...\n");
  if (_24c02_sequential_read(bp, dev_addr, 0, mem, 256) < 0)
    return -1;
  for (addr= 0; addr < 256; addr++)
    if (mem[addr] != 0)
      printf("Mismatch at addr=%.2X (%.2X != %.2X)\n",
	     addr, 0, mem[addr]);

  printf("32 8-byte page writes...\n");
  for (addr= 0; addr < 256; addr++)
    mem[addr]= 255-addr;
  for (p= 0; p < 32; p++) {
    if (_24c02_page_write(bp, dev_addr, p*8, &mem[p*8], 8) < 0)
      return -1;
    //usleep(10000); /* 6ms write time */
    if (_24c02_ack_polling(bp, dev_addr) < 0)
      return -1;
  }

  printf("full-read...\n");
  if (_24c02_sequential_read(bp, dev_addr, 0, mem, 256) < 0)
    return -1;
  for (addr= 0; addr < 256; addr++)
    if (mem[addr] != 255-addr)
      printf("Mismatch at addr=%.2X (%.2X != %.2X)\n",
	     addr, 255-addr, mem[addr]);

  return 0;
}
#endif /* __BUSPIRATE__ */

