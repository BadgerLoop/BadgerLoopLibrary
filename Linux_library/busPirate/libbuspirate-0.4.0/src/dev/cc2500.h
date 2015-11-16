// ==================================================================
// @(#)cc2500.h
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

#ifndef __CC2500_H__
#define __CC2500_H__

#include <buspirate.h>
#include <spi.h>

/* R/W configuration registers */
#define CC2500_REG_CHANNR    0x0A
#define CC2500_REG_FREQ2     0x0D
#define CC2500_REG_FREQ1     0x0E
#define CC2500_REG_FREQ0     0x0F
#define CC2500_REG_MDMCFG4   0x10
#define CC2500_REG_MDMCFG3   0x11
#define CC2500_REG_MDMCFG2   0x12
#define CC2500_REG_MDMCFG1   0x13
#define CC2500_REG_MDMCFG0   0x14

/* Command strobes */
#define CC2500_SRES          0x30
#define CC2500_SFSTXON       0x31
#define CC2500_SXOFF         0x32
#define CC2500_SCAL          0x33
#define CC2500_SRX           0x34
#define CC2500_STX           0x35
#define CC2500_SIDLE         0x36
#define CC2500_SWOR          0x38
#define CC2500_SPWD          0x39
#define CC2500_SFRX          0x3A
#define CC2500_SFTX          0x3B
#define CC2500_SWORRST       0x3C
#define CC2500_SNOP          0x3D

/* Status registers (read-only) */
#define CC2500_REG_PARTNUM        0xF0
#define CC2500_REG_VERSION        0xF1
#define CC2500_REG_FREQEST        0xF2
#define CC2500_REG_LQI            0xF3
#define CC2500_REG_RSSI           0xF4
#define CC2500_REG_MARCSTATE      0xF5
#define CC2500_REG_WORTIME1       0xF6
#define CC2500_REG_WORTIME0       0xF7
#define CC2500_REG_PKTSTATUS      0xF8
#define CC2500_REG_VCO_VC_DAC     0xF9
#define CC2500_REG_TXBYTES        0xFA
#define CC2500_REG_RXBYTES        0xFB
#define CC2500_REG_RCCTRL1_STATUS 0xFC
#define CC2500_REG_RCCTRL0_STATUS 0xFD

#define CC2500_STATUS_CHIP_RDY        0x80
/* status bits 6:4 */
#define CC2500_STATUS_STATE_IDLE      0x00
#define CC2500_STATUS_STATE_RX        0x01
#define CC2500_STATUS_STATE_TX        0x02
#define CC2500_STATUS_STATE_FSTXON    0x03
#define CC2500_STATUS_STATE_CALIBRATE 0x04
#define CC2500_STATUS_STATE_SETTLING  0x05
#define CC2500_STATUS_STATE_RXFIFO_OV 0X06
#define CC2500_STATUS_STATE_TXFIFO_UN 0x07

#define CC2500_SUCCESS 1
#define CC2500_FAILURE 0

#ifdef __cplusplus
extern "C" {
#endif

  int cc2500_read_register(BP * bp, unsigned char reg, unsigned char * value);

  int cc2500_get_version(BP * bp, unsigned char * version,
			 unsigned char * partnum);
  int cc2500_get_channel(BP * bp, unsigned char * channel);
  int cc2500_get_freq(BP * bp, unsigned long * freq);
  int cc2500_get_marcstate(BP * bp, unsigned char * marcstate);
  int cc2500_strobe(BP * bp, unsigned char strobe, unsigned char * status);

  void cc2500_show_status(unsigned char status);
  void cc2500_show_marcstate(unsigned char marcstate);
  void cc2500_show_mdmcfg4_3(unsigned char cfg4, unsigned char cfg3);
  void cc2500_show_mdmcfg2(unsigned char value);
  void cc2500_show_mdmcfg1_0(unsigned char cfg1, unsigned char cfg0);

  int demo_cc2500(BP * bp, int argc, char * argv[]);
  
#ifdef __cplusplus
}
#endif

#endif
