// ==================================================================
// @(#)mrf24j40.h
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

#ifndef __MRF24J40_H__
#define __MRF24J40_H__

#include <802_15_4.h>
#include <stddef.h>

// -----[ Short address registers ]-----
#define MRF24J40_SREG_RXMCR     0x00
#define MRF24J40_SREG_PANIDL    0x01
#define MRF24J40_SREG_PANIDH    0x02
#define MRF24J40_SREG_SADRL     0x03
#define MRF24J40_SREG_SADRH     0x04
#define MRF24J40_SREG_EADR0     0x05
#define MRF24J40_SREG_EADR1     0x06
#define MRF24J40_SREG_EADR2     0x07
#define MRF24J40_SREG_EADR3     0x08
#define MRF24J40_SREG_EADR4     0x09
#define MRF24J40_SREG_EADR5     0x0A
#define MRF24J40_SREG_EADR6     0x0B
#define MRF24J40_SREG_EADR7     0x0C
#define MRF24J40_SREG_RXFLUSH   0x0D
/* 0x0E, 0x0F reserved */
#define MRF24J40_SREG_ORDER     0x10
#define MRF24J40_SREG_TXMCR     0x11
#define MRF24J40_SREG_ACKTMOUT  0x12
#define MRF24J40_SREG_ESLOTG1   0x13
#define MRF24J40_SREG_SYMTICKL  0x14
#define MRF24J40_SREG_SYMTICKH  0x15
#define MRF24J40_SREG_PACON0    0x16 /* Power amplifier control 0 */
#define MRF24J40_SREG_PACON1    0x17 /* Power amplifier control 1 */
#define MRF24J40_SREG_PACON2    0x18 /* Power amplifier control 2 */
/* 0x19 reserved */
#define MRF24J40_SREG_TXBCON0   0x1A
#define MRF24J40_SREG_TXNCON    0x1B
#define MRF24J40_SREG_TXG1CON   0x1C
#define MRF24J40_SREG_TXG2CON   0x1D
#define MRF24J40_SREG_ESLOTG23  0x1E
#define MRF24J40_SREG_ESLOTG45  0x1F
#define MRF24J40_SREG_ESLOTG67  0x20
#define MRF24J40_SREG_TXPEND    0x21
#define MRF24J40_SREG_WAKECON   0x22
#define MRF24J40_SREG_FRMOFFSET 0x23
#define MRF24J40_SREG_TXSTAT    0x24
#define MRF24J40_SREG_TXBCON1   0x25
#define MRF24J40_SREG_GATECLK   0x26
#define MRF24J40_SREG_TXTIME    0x27
#define MRF24J40_SREG_HSYMTMRL  0x28
#define MRF24J40_SREG_HSYMTMRH  0x29
#define MRF24J40_SREG_SOFTRST   0x2A /* Software reset */
/* 0x2B reserved */
#define MRF24J40_SREG_SECCON0   0x2C
#define MRF24J40_SREG_SECCON1   0x2D
#define MRF24J40_SREG_TXSTBL    0x2E /* TX stabilization */
/* 0x2F reserved */
#define MRF24J40_SREG_RXSR      0x30
#define MRF24J40_SREG_INTSTAT   0x31 /* Interrupt status */
#define MRF24J40_SREG_INTCON    0x32
#define MRF24J40_SREG_GPIO      0x33
#define MRF24J40_SREG_TRISGPIO  0x34
#define MRF24J40_SREG_SLPACK    0x35
#define MRF24J40_SREG_RFCTL     0x36 /* RF mode control */
#define MRF24J40_SREG_SECCR2    0x37
#define MRF24J40_SREG_BBREG0    0x38
#define MRF24J40_SREG_BBREG1    0x39
#define MRF24J40_SREG_BBREG2    0x3A
#define MRF24J40_SREG_BBREG3    0x3B
#define MRF24J40_SREG_BBREG4    0x3C
/* 0x3D reserved */
#define MRF24J40_SREG_BBREG6    0x3E
#define MRF24J40_SREG_CCAEDTH   0x3F

// -----[ Long address registers ]-----
#define MRF24J40_LREG_RFCON0    0x200
#define MRF24J40_LREG_RFCON1    0x201
#define MRF24J40_LREG_RFCON2    0x202
#define MRF24J40_LREG_RFCON3    0x203
/* 0x204 reserved */
#define MRF24J40_LREG_RFCON5    0x205
#define MRF24J40_LREG_RFCON6    0x206
#define MRF24J40_LREG_RFCON7    0x207
#define MRF24J40_LREG_RFCON8    0x208
#define MRF24J40_LREG_SLPCAL0   0x209
#define MRF24J40_LREG_SLPCAL1   0x20A
#define MRF24J40_LREG_SLPCAL2   0x20B
/* 0x20C-0x20E reserved */
#define MRF24J40_LREG_RSSI      0x210
#define MRF24J40_LREG_SLPCON0   0x211
/* 0x212-0x21F reserved */
#define MRF24J40_LREG_SLPCON1   0x220
/* 0x221 reserved */
#define MRF24J40_LREG_WAKETIMEL 0x222
#define MRF24J40_LREG_WAKETIMEH 0x223
#define MRF24J40_LREG_REMCNTL   0x224
#define MRF24J40_LREG_REMCNTH   0x225
#define MRF24J40_LREG_MAINCNT0  0x226
#define MRF24J40_LREG_MAINCNT1  0x227
#define MRF24J40_LREG_MAINCNT2  0x228
#define MRF24J40_LREG_MAINCNT3  0x229
/* 0x22A-0x22E reserved */
#define MRF24J40_LREG_TESTMODE  0x22F
#define MRF24J40_LREG_ASSOEADR0 0x230
#define MRF24J40_LREG_ASSOEADR1 0x231
#define MRF24J40_LREG_ASSOEADR2 0x232
#define MRF24J40_LREG_ASSOEADR3 0x233
#define MRF24J40_LREG_ASSOEADR4 0x234
#define MRF24J40_LREG_ASSOEADR5 0x235
#define MRF24J40_LREG_ASSOEADR6 0x236
#define MRF24J40_LREG_ASSOEADR7 0x237
#define MRF24J40_LREG_ASS0SADR0 0x238
#define MRF24J40_LREG_ASSOSADR1 0x239
/* 0x23A-0x23B reserved */
/* 0x23C-0x23E unimplemented */
#define MRF24J40_LREG_UPNONCE0  0x240
#define MRF24J40_LREG_UPNONCE1  0x241
#define MRF24J40_LREG_UPNONCE2  0x242
#define MRF24J40_LREG_UPNONCE3  0x243
#define MRF24J40_LREG_UPNONCE4  0x244
#define MRF24J40_LREG_UPNONCE5  0x245
#define MRF24J40_LREG_UPNONCE6  0x246
#define MRF24J40_LREG_UPNONCE7  0x247
#define MRF24J40_LREG_UPNONCE8  0x248
#define MRF24J40_LREG_UPNONCE9  0x249
#define MRF24J40_LREG_UPNONCE10 0x24A
#define MRF24J40_LREG_UPNONCE11 0x24B
#define MRF24J40_LREG_UPNONCE12 0x24C

#define MRF24J40_TXSTAT_TXNSTAT   0x01
#define MRF24J40_TXSTAT_CCAFAIL   0x20
#define MRF24J40_TXSTAT_TXNRETRY  0xC0

#define MRF24J40_INTSTAT_TXNIF    0x01
#define MRF24J40_INTSTAT_RXIF     0x08

#define MRF24J40_TXNCON_TXNTRIG   0x01
#define MRF24J40_TXNCON_TXNACKREQ 0x04

#define MRF24J40_BBREG1_RXDECINV  0x04

#define MRF24J40_RXMCR_PROMI      0x01
#define MRF24J40_RXMCR_ERRPKT     0x02
#define MRF24J40_RXMCR_NOACKRSP   0x20

// -----[ Memory area offsets (long addresses) ]-----
#define MRF24J40_MEM_OFFSET_TXN_FIFO  0x000
#define MRF24J40_MEM_OFFSET_TXB_FIFO  0x080
#define MRF24J40_MEM_OFFSET_GTS1_FIFO 0x100
#define MRF24J40_MEM_OFFSET_GTS2_FIFO 0x180
#define MRF24J40_MEM_OFFSET_CTRL_REGS 0x200
#define MRF24J40_MEM_OFFSET_SEC_BUF   0x280
#define MRF24J40_MEM_OFFSET_RX_FIFO   0x300

#define MRF24J40_TX_SUCCESS          0x01
#define MRF24J40_TX_ERROR            0x02
#define MRF24J40_TX_ERROR_CCA_FAILED 0x04

#ifndef DEVICE
#define DEVICE void
#endif /* DEVICE */

#ifdef __cplusplus
extern "C" {
#endif

  int mrf24j40_short_read(DEVICE * dev, unsigned char addr, unsigned char * value);
  int mrf24j40_short_write(DEVICE * dev, unsigned char addr, unsigned char value);
  
  int mrf24j40_long_read(DEVICE * dev, unsigned short addr, unsigned char * value);
  int mrf24j40_long_write(DEVICE * dev, unsigned short addr, unsigned char value);
  int mrf24j40_init(DEVICE * dev, unsigned short panid, unsigned short saddr,
		    const unsigned char * laddr, unsigned char channel,
		    int coordinator);
  int mrf24j40_promiscuous(DEVICE * dev, int error);
  int mrf24j40_set_tx_power(DEVICE * dev, unsigned char lsc,
			    unsigned char ssc);
  int mrf24j40_tx(DEVICE * dev, unsigned short panid,
		  unsigned short saddr, unsigned short daddr,
		  unsigned char seqnum, int ack_request,
		  const char * payload, unsigned char payload_len);
  int mrf24j40_tx_status(DEVICE * dev, int * status);
  int mrf24j40_intstat_read(DEVICE * dev, int * intstat);
  int mrf24j40_intstat_select(DEVICE * dev, int mask, int * intstat);

  int mrf24j40_rx(DEVICE * dev, mac_frame * frame, unsigned char * rssi,
		  unsigned char * lqi);

#ifdef __BUSPIRATE__
  int mrf24j40_xtalk_test(DEVICE * dev, int num_meas);
#endif /* __BUSPIRATE__ */

#ifdef __cplusplus
}
#endif

#endif /* __MRF24J40_H__ */
