// ==================================================================
// @(#)enc28j60.h
//
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 19/01/2012
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

#ifndef __ENC28J60_H__
#define __ENC28J60_H__

#include <buspirate.h>

/* Any bank registers */
// Reserved 0x1A
#define ENC28J60_REG_EIE         0x1B
#define ENC28J60_REG_EIR         0x1C
#define ENC28J60_REG_ESTAT       0x1D
#define ENC28J60_REG_ECON2       0x1E
#define ENC28J60_REG_ECON1       0x1F

/* Bank 0 registers */
#define ENC28J60_REG_B0_ERDPTL   0x00
#define ENC28J60_REG_B0_ERDPTH   0x01
#define ENC28J60_REG_B0_EWRPTL   0x02
#define ENC28J60_REG_B0_EWRPTH   0x03
#define ENC28J60_REG_B0_ETXSTL   0x04
#define ENC28J60_REG_B0_ETXSTH   0x05
#define ENC28J60_REG_B0_ETXNDL   0x06
#define ENC28J60_REG_B0_ETXNDH   0x07
#define ENC28J60_REG_B0_ERXSTL   0x08
#define ENC28J60_REG_B0_ERXSTH   0x09
#define ENC28J60_REG_B0_ERXNDL   0x0A
#define ENC28J60_REG_B0_ERXNDH   0x0B
#define ENC28J60_REG_B0_ERXRDPTL 0x0C
#define ENC28J60_REG_B0_ERXRDPTH 0x0D
#define ENC28J60_REG_B0_ERXWRPTL 0x0E
#define ENC28J60_REG_B0_ERXWRPTH 0x0F
#define ENC28J60_REG_B0_EDMASTL  0x10
#define ENC28J60_REG_B0_EDMASTH  0x11
#define ENC28J60_REG_B0_EDMANDL  0x12
#define ENC28J60_REG_B0_EDMANDH  0x13
#define ENC28J60_REG_B0_EDMADSTL 0x14
#define ENC28J60_REG_B0_EDMADSTH 0x15
#define ENC28J60_REG_B0_EDMACSL  0x16
#define ENC28J60_REG_B0_EDMACSH  0x17
// Reserved 0x18
// Reserved 0x19

/* Bank 1 registers */
#define ENC28J60_REG_B1_EHT0     0x00
#define ENC28J60_REG_B1_EHT1     0x01
#define ENC28J60_REG_B1_EHT2     0x02
#define ENC28J60_REG_B1_EHT3     0x03
#define ENC28J60_REG_B1_EHT4     0x04
#define ENC28J60_REG_B1_EHT5     0x05
#define ENC28J60_REG_B1_EHT6     0x06
#define ENC28J60_REG_B1_EHT7     0x07
#define ENC28J60_REG_B1_EPMM0    0x08
#define ENC28J60_REG_B1_EPMM1    0x09
#define ENC28J60_REG_B1_EPMM2    0x0A
#define ENC28J60_REG_B1_EPMM3    0x0B
#define ENC28J60_REG_B1_EPMM4    0x0C
#define ENC28J60_REG_B1_EPMM5    0x0D
#define ENC28J60_REG_B1_EPMM6    0x0E
#define ENC28J60_REG_B1_EPMM7    0x0F
#define ENC28J60_REG_B1_EPMCSL   0x10
#define ENC28J60_REG_B1_EPMCSH   0x11
// Reserved 0x12
// Reserved 0x13
#define ENC28J60_REG_B1_EPMOL    0x14
#define ENC28J60_REG_B1_EPMOH    0x15
// Reserved 0x16
// Reserved 0x17
#define ENC28J60_REG_B1_ERXFCON  0x18
#define ENC28J60_REG_B1_EPKTCNT  0x19

/* Bank 2 registers */
#define ENC28J60_REG_B2_MACON1   0x00
// Reserved 0x01
#define ENC28J60_REG_B2_MACON3   0x02
#define ENC28J60_REG_B2_MACON4   0x03
#define ENC28J60_REG_B2_MABBIPG  0x04
// Reserved 0x05
#define ENC28J60_REG_B2_MAIPGL   0x06
#define ENC28J60_REG_B2_MAIPGH   0x07
#define ENC28J60_REG_B2_MACLCON1 0x08
#define ENC28J60_REG_B2_MACLCON2 0x09
#define ENC28J60_REG_B2_MAMXFLL  0x0A
#define ENC28J60_REG_B2_MAMXFLH  0x0B
// Reserved 0x0C-0x11
#define ENC28J60_REG_B2_MICMD    0x12
// Reserved 0x13
#define ENC28J60_REG_B2_MIREGADR 0x14
// Reserved 0x15
#define ENC28J60_REG_B2_MIWRL    0x16
#define ENC28J60_REG_B2_MIWRH    0x17
#define ENC28J60_REG_B2_MIRDL    0x18
#define ENC28J60_REG_B2_MIRDH    0x19

/* Bank 3 registers */
#define ENC28J60_REG_B3_MAADR5   0x00
#define ENC28J60_REG_B3_MAADR6   0x01
#define ENC28J60_REG_B3_MAADR3   0x02
#define ENC28J60_REG_B3_MAADR4   0x03
#define ENC28J60_REG_B3_MAADR1   0x04
#define ENC28J60_REG_B3_MAADR2   0x05
#define ENC28J60_REG_B3_EBSTSD   0x06
#define ENC28J60_REG_B3_EBSTCON  0x07
#define ENC28J60_REG_B3_EBSTCSL  0x08
#define ENC28J60_REG_B3_EBSTCSH  0x09
#define ENC28J60_REG_B3_MISTAT   0x0A
// Reserved 0x0B-0x11
#define ENC28J60_REG_B3_EREVID   0x12
// Reserved 0x13-0x14
#define ENC28J60_REG_B3_ECOCON   0x15
// Reserved 0x16
#define ENC28J60_REG_B3_EFLOCON  0x17
#define ENC28J60_REG_B3_EPAUSL   0x18
#define ENC28J60_REG_B3_EPAUSH   0x19

#define ETH_ADDR_LEN 6
typedef unsigned char eth_addr_t[ETH_ADDR_LEN];

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ enc28j60_init ]------------------------------------------
  int enc28j60_init(BP * bp, eth_addr_t addr);
  // -----[ enc28j60_rx_enable ]-------------------------------------
  int enc28j60_rx_enable(BP * bp, int enable);
  // -----[ enc28j60_read_pktcnt ]-----------------------------------
  int enc28j60_read_pktcnt(BP * bp, unsigned char * pktcnt);
  // -----[ enc28j60_frame_available ]-------------------------------
  int enc28j60_frame_available(BP * bp);
  // -----[ enc28j60_read_frame ]------------------------------------
  int enc28j60_read_frame(BP * bp, unsigned char * buf, unsigned int size,
			  unsigned long * status);


  int demo_spi_enc28j60(BP * bp);

#ifdef __cplusplus
}
#endif

#endif /* __ENC28J60_H__ */
