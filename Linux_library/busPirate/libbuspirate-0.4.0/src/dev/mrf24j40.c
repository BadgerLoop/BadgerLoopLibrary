// ==================================================================
// @(#)mrf24j40.c
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

#include <mrf24j40.h>

#ifdef __BUSPIRATE__
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <spi.h>
#define DEBUG_ID     "MRF24J40"
#define DEBUG_STREAM stderr
#include <debug.h>
#include <buspirate.h>
//#define DEVICE BP
#define DEVICE_SPI_SS_ACTIVE(DEV)				\
  if (bp_bin_spi_cs((BP *) DEV, 0) != BP_SUCCESS)		\
    return -1;
#define DEVICE_SPI_SS_INACTIVE(DEV)		  \
  if (bp_bin_spi_cs((BP *) DEV, 1) != BP_SUCCESS) \
    return -1;
#define DEVICE_SPI_TXRX(DEV, IDATA)				\
  if (bp_bin_spi_bulk((BP *) DEV, IDATA, 1) != BP_SUCCESS)	\
    return -1;
#define USLEEP(us) usleep(us)
#else
#include <defs.h>
#include <printf-stdarg.h>
#include <string.h>
#include <lcd.h>
#include <spi.h>
#define DEVICE void
#define DEVICE_SPI_SS_ACTIVE(DEV)   spi0_ss(SPI_SS_ACTIVE)
#define DEVICE_SPI_SS_INACTIVE(DEV) spi0_ss(SPI_SS_INACTIVE)
#define DEVICE_SPI_TXRX(DEV, IDATA) \
  *IDATA= spi0_txrx(*IDATA)
#define USLEEP(us)					\
  int SLEEP_i;						\
  for (SLEEP_i= 0; SLEEP_i < 60*(us); SLEEP_i++)	\
    NOP;
#endif /* __BUSPIRATE__ */

// -----[ utility functions ]----------------------------------------
/**
 * Short read/write command are formatted as follows:
 * bit  7   = 0
 * bits 6-1 = address
 * bit  0   = 0 (read) / 1 (write)
 */
inline unsigned char SHORT_READ_CMD(unsigned char addr)
{
  assert(addr < 64);
  return (addr << 1);
}

inline unsigned char SHORT_WRITE_CMD(unsigned char addr)
{
  assert(addr < 64);
  return (addr << 1) | 0x01;
}

// -----[ mrf24j40_short_read ] -------------------------------------
/**
 * Read one byte from a short address memory register of the
 * MRF24J40 transceiver.
 *
 * A short address is a 6 bits number.
 */
int mrf24j40_short_read(DEVICE * dev, unsigned char addr,
			unsigned char * value)
{
  unsigned char data[]= { addr << 1, 0xFF };
  DEVICE_SPI_SS_ACTIVE(dev);
#ifdef __BUSPIRATE__
  if (bp_bin_spi_bulk((BP *) dev, data, 2) != BP_SUCCESS)
    return -1;
# else
  int i;
  for (i= 0; i < sizeof(data)/sizeof(unsigned char); i++)
    DEVICE_SPI_TXRX(dev, &data[i]);
#endif /* __BUSPIRATE__ */
  DEVICE_SPI_SS_INACTIVE(dev);
  if (value != NULL)
    *value= data[1];
  return 0;
}

// -----[ mrf24j40_short_write ] ------------------------------------
/**
 * Write one byte to a short address memory register of the
 * MRF24J40 transceiver.
 *
 * A short address is a 6 bits number.
 */
int mrf24j40_short_write(DEVICE * dev, unsigned char addr,
			 unsigned char value)
{
  int i;
  unsigned char data[]= { (addr << 1) | 0x01, value };
  DEVICE_SPI_SS_ACTIVE(dev);
  for (i= 0; i < sizeof(data)/sizeof(unsigned char); i++)
    DEVICE_SPI_TXRX(dev, &data[i]);
  DEVICE_SPI_SS_INACTIVE(dev);
  return 0;
}

// -----[ mrf24j40_long_read ]---------------------------------------
/**
 * Read one byte from a long address memory register of the
 * MRF24J40 transceiver.
 *
 * A long address is a 10 bits number.
 */
int mrf24j40_long_read(DEVICE * dev, unsigned short addr,
		       unsigned char * value)
{
  assert(addr < 0x3FF);
  unsigned char data[]= { 0x80 | (addr >> 3), (addr << 5) & 0xF0, 0xFF };
  DEVICE_SPI_SS_ACTIVE(dev);
#ifdef __BUSPIRATE__
  if (bp_bin_spi_bulk((BP *) dev, data, 3) != BP_SUCCESS)
    return -1;
#else
  int i;
  for (i= 0; i < sizeof(data)/sizeof(unsigned char); i++)
    DEVICE_SPI_TXRX(dev, &data[i]);  
#endif
  DEVICE_SPI_SS_INACTIVE(dev);
  if (value != NULL)
    *value= data[2];
  return 0;
}

// -----[ mrf24j40_long_write ]---------------------------------------
/**
 * Write one byte to a long address memory register of the
 * MRF24J40 transceiver.
 *
 * A long address is a 10 bits number.
 */
int mrf24j40_long_write(DEVICE * dev, unsigned short addr,
			unsigned char value)
{
  int i;
  assert(addr < 0x3FF);
  unsigned char data[]= { 0x80 | (addr >> 3), ((addr << 5) & 0xF0) | 0x10, value };
  DEVICE_SPI_SS_ACTIVE(dev);
  for (i= 0; i < sizeof(data)/sizeof(unsigned char); i++)
    DEVICE_SPI_TXRX(dev, &data[i]);
  DEVICE_SPI_SS_INACTIVE(dev);
  return 0;
}

#define SHORT_WRITE(ADDRESS, VALUE) \
  if (mrf24j40_short_write(dev, MRF24J40_SREG_##ADDRESS, VALUE) < 0) \
    return -1;
#define SHORT_READ(ADDRESS, VALUE_PTR) \
  if (mrf24j40_short_read(dev, MRF24J40_SREG_##ADDRESS, VALUE_PTR) < 0) \
    return -1;
#define LONG_WRITE(ADDRESS, VALUE) \
  if (mrf24j40_long_write(dev, MRF24J40_LREG_##ADDRESS, VALUE) < 0) \
    return -1;
#define LONG_READ(ADDRESS, VALUE_PTR) \
  if (mrf24j40_long_read(dev, MRF24J40_LREG_##ADDRESS, VALUE_PTR) < 0) \
    return -1;

// -----[ mrf24j40_init ]--------------------------------------------
/**
 * Initialize MRF24J40 transceiver.
 *
 * Note: see RXMCR register to allow reception of packets with errors
 *       (bad CRC, bad MAC address, illegal frame type, dPAN/sPAN or
 *       MAC short address mismatch)
 */
int mrf24j40_init(DEVICE * dev, unsigned short panid, unsigned short saddr,
		  const unsigned char * laddr, unsigned char channel,
		  int coordinator) {
  unsigned char value;
  
  // Perform a software reset (3 bits cleared by hardware)
  SHORT_WRITE(SOFTRST, 0x07);

  // Initialize FIFOEN=1 and TXONTS=0x6 
  SHORT_WRITE(PACON2, 0x98);

  // Initialize RFSTBL=0x9
  SHORT_WRITE(TXSTBL, 0x95);

  // Initialize VCOOPT
  LONG_WRITE(RFCON1, 0X01);

  // Enable PLL
  LONG_WRITE(RFCON2, 0x80);

  // Initialize TXFIL=1 and 20MRECVR=1
  LONG_WRITE(RFCON6, 0x90);

  // Initialize SLPCLKSEL=0x2
  LONG_WRITE(RFCON7, 0x80);

  // Initialize RFVCO=1
  LONG_WRITE(RFCON8, 0x10);

  // Initialize /CLKOUTEN=1 and SLPCLKDIV=0x01
  LONG_WRITE(SLPCON1, 0x21);

  // !!! CONFIGURATION FOR NONBEACON-ENABLED DEVICES !!!
  if (coordinator) {
    SHORT_READ(RXMCR, &value);
    SHORT_WRITE(RXMCR, value | 0x08); // PANCOORD <3> = 1
    SHORT_WRITE(ORDER, 0xFF); // BO <7:4>= 0xF, SO <3:0> = 0xF
  } else {
    SHORT_READ(RXMCR, &value);
    SHORT_WRITE(RXMCR, value & 0xF7); // PANCOORD <3> = 0
  }
  SHORT_READ(TXMCR, &value);
  SHORT_WRITE(TXMCR, value & 0xDF); // SLOTTED <5> = 0
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  SHORT_WRITE(BBREG2, 0x80); // Set CCA mode to ED (mode 1)
  SHORT_WRITE(CCAEDTH, 0x60); // Set CCA ED threshold
  SHORT_WRITE(BBREG6, 0x40); // Set appended RSSI value to RXFIFO

  // Enable interrupts HERE
  SHORT_READ(INTCON, &value);
  SHORT_WRITE(INTCON, value & 0xF6); // Enable RXIE <3> and TXNIE <0>
  
  LONG_READ(SLPCON0, &value);
  LONG_WRITE(SLPCON0, value | 0x02); // INTEDGE <1> 0=falling, 1=rising

  // Set channel
  // - bits 7:4 = channel (11 <= channel <= 26)
  // - bits 3:1 = RF optimize control (default=0; recommended=2)
  assert((channel >= 11) && (channel <= 26)); 
  LONG_WRITE(RFCON0, ((channel-11) << 4) | 0x02);

  SHORT_WRITE(RFCTL, 0x04); // Reset RF state machine
  SHORT_WRITE(RFCTL, 0x00);

  USLEEP(192); // Delay at least 192us

  // Flush the RX FIFO
  // - bit 0=1 flush RX FIFO (cleared by hardware)
  SHORT_WRITE(RXFLUSH, 0x01);

  // Configure PANID
  SHORT_WRITE(PANIDL, panid & 0xFF);
  SHORT_WRITE(PANIDH, panid >> 8);

  // Configure short MAC address
  SHORT_WRITE(SADRL, saddr & 0xFF);
  SHORT_WRITE(SADRH, saddr >> 8);

  // Configure long MAC address
  char i;
  for (i= 0; i < 8; i++)
    SHORT_WRITE(EADR0+i, *(laddr+i));

  // Enable auto-acknowledgment
  SHORT_READ(RXMCR, &value);
  SHORT_WRITE(RXMCR, value & MRF24J40_RXMCR_NOACKRSP);

  return 0;
}

// -----[ mrf24j40_promiscuous ]-------------------------------------
/**
 * Enable promiscuous mode. Optionally, the device can accept frames
 * with a bad CRC.
 *
 * RXMCR register:
 *   promiscuous bit 0
 *   error       bit 1
 */
int mrf24j40_promiscuous(DEVICE * dev, int error) {
  unsigned char value;

  SHORT_READ(RXMCR, &value);
  value|= MRF24J40_RXMCR_PROMI;
  if (error)
    value|= MRF24J40_RXMCR_ERRPKT;
  SHORT_WRITE(RXMCR, value); 
  return 0;
}

// -----[ mrf24j40_set_tx_power ]------------------------------------
/**
 * Set the TX power.
 *
 * Parameters: lsc is the large scale control (10dB granularity)
 *             ssc is the small scale control
 */
int mrf24j40_set_tx_power(DEVICE * dev, unsigned char lsc, unsigned char ssc)
{
  assert(lsc <= 3);
  assert(ssc <= 7);
  unsigned char rfcon3;
  rfcon3= (lsc << 6) + (ssc << 1);
  LONG_WRITE(RFCON3, rfcon3);
  return 0;
}

#define TXN_FIFO_WRITE(OFFS, VALUE) \
  if (mrf24j40_long_write(dev, MRF24J40_MEM_OFFSET_TXN_FIFO+OFFS, VALUE) < 0) \
    return -1;
#define RX_FIFO_READ(OFFS, VALUE_PTR) \
  if (mrf24j40_long_read(dev, MRF24J40_MEM_OFFSET_RX_FIFO+OFFS, VALUE_PTR) < 0) \
    return -1;

// -----[ mrf24j40_tx ]----------------------------------------------
/**
 * Note: if ack_request is asserted, the MRF24J40 will expect an ACK
 *       and retry transmission will be done automatically by the
 *       hardware. The maximum number of retransmissions is fixed by
 *       the IEEE standard to aMaxFrameRetries (3). If no ACK is
 *       received after the last retransmission, the hardware will
 *       notify the failure.
 *
 * Return value:
 *   -1 SPI communication failure
 *    0 frame transmission failed
 *    1 frame transmission succeeded
 */
int mrf24j40_tx(DEVICE * dev, unsigned short panid,
		unsigned short saddr, unsigned short daddr,
		unsigned char seqnum, int ack_request,
		const char * payload, unsigned char payload_len)
{
  assert(payload_len < 128);

  // Prepare packet in TX FIFO. Layout is as follows:
  //  0  : header length
  //  1  : packet length (m+3)
  //  2-3: frame control
  //  4  : sequence number
  //  5  : data[0]
  //  ...
  //  5+m-1: data[m-1]
  //  6+m: FCS[0]
  //  7+m: FCS[1]

  // Header length (ignored if no security used)
  TXN_FIFO_WRITE(0, 0);

  // Frame length (m)
  TXN_FIFO_WRITE(1, 11+payload_len);

  // Frame control 0:
  //  bits 0-2   frame type       : 001 = data frame
  //  bit  3     security enabled : 0 disabled
  //  bit  4     frame pending    : 0 no frame pending
  //  bit  5     ACK request      : 0 no ACK request
  //  bit  6     PAN ID compress.
  //  bits 7-9   reserved
  //  bits 10-11 DA mode
  //  bits 12-13 frame version
  //  bits 14-15 SA mode
  TXN_FIFO_WRITE(2, 0x01 | (ack_request?0x20:0));

  // Frame control 1
  unsigned char da_mode= 0x02; // PAN + short
  unsigned char sa_mode= 0x02; // PAN + short
  TXN_FIFO_WRITE(3, ((sa_mode << 6) | (da_mode << 2)));

  // Sequence number
  TXN_FIFO_WRITE(4, seqnum);

  // Destination PANID
  TXN_FIFO_WRITE(5, panid & 0xFF);
  TXN_FIFO_WRITE(6, panid >> 8);

  // Destination Short Address
  TXN_FIFO_WRITE(7, daddr & 0xFF);
  TXN_FIFO_WRITE(8, daddr >> 8);

  // Source PANID
  TXN_FIFO_WRITE(9, panid & 0xFF);
  TXN_FIFO_WRITE(10, panid >> 8);

  // Source Short Address
  TXN_FIFO_WRITE(11, saddr & 0xFF);
  TXN_FIFO_WRITE(12, saddr >> 8);

  // Data
  int i= 0;
  while (i < payload_len) {
    TXN_FIFO_WRITE(13+i, payload[i]);
    i++;
  }
  
  // Trigger transmission
  //  bit 4 = 1 data pending bit was set ; 0 not set
  //  bit 3 = 1 indirect transmission enabled ; 0 disabled
  //  bit 2 = 1 ACK requested ; 0 no ACK requested
  //  bit 1 = 1 secure packet ; 0 no security
  //  bit 0 = 1 trigger packet transmission (cleared by hardware)
  unsigned char txncon= MRF24J40_TXNCON_TXNTRIG;
  if (ack_request)
    txncon|= MRF24J40_TXNCON_TXNACKREQ;
  SHORT_WRITE(TXNCON, txncon); // Initiate normal FIFO transmission
 
  return 0;
}

// -----[ mrf24j40_intstat_read ]------------------------------------
/**
 *
 */
int mrf24j40_intstat_read(DEVICE * dev, int * intstat)
{
  unsigned char _intstat;
  SHORT_READ(INTSTAT, &_intstat);
  if (intstat != NULL)
    *intstat= _intstat;
  return 0;
}

// -----[ mrf24j40_intstat_select ]----------------------------------
/**
 * This function is provided to allow a client to perform an active
 * wait (polling) for a change in the INTSTAT register instead of
 * relying on interrupts.
 *
 * The caller can provide a mask of INTSTAT bits it is interested in.
 * The function will only return if the INTSTAT register has one of
 * the masked bits equal to 1.
 */
int mrf24j40_intstat_select(DEVICE * dev, int mask, int * intstat)
{
  int _intstat;
  do {
    if (mrf24j40_intstat_read(dev, &_intstat) < 0)
      return -1;
  } while ((_intstat & mask) == 0);
  if (intstat != NULL)
    *intstat= _intstat;
  return 0;
}

// -----[ mrf24j40_tx_status ]---------------------------------------
/**
 * Check transmission status. This function should only be used after
 * the INTSTAT register has bit TXNIF equal to 1 (i.e. after a call to
 * mrf24j40_intstat_read or _select).
 *
 * Register TXSTAT contains receive status
 *   bits 7-6 = retry times
 *   bit  5 = 1 CCA failed ; 0 CCA passed
 */
int mrf24j40_tx_status(DEVICE * dev, int * status)
{
  unsigned char txstat;
  *status= 0;

  SHORT_READ(TXSTAT, &txstat);
  if (txstat & MRF24J40_TXSTAT_CCAFAIL)
    *status|= MRF24J40_TX_ERROR_CCA_FAILED;

  if (!(txstat & MRF24J40_TXSTAT_TXNSTAT))
    *status|= MRF24J40_TX_SUCCESS;
  else
    *status|= MRF24J40_TX_ERROR;

  return 0;
}

// -----[ mrf24j40_rx ]----------------------------------------------
/**
 */
int mrf24j40_rx(DEVICE * dev, mac_frame * frame, unsigned char * rssi,
		unsigned char * lqi)
{
  unsigned char rx_fifo[144]; // 0x300 - 0x38F
  int i;

  // Set RXDECINV bit (disable reception)
  SHORT_WRITE(BBREG1, MRF24J40_BBREG1_RXDECINV);

  // Read whole RX buffer
  RX_FIFO_READ(0, &rx_fifo[0]);
  unsigned char len= rx_fifo[0];
  assert(len <= 127);
  for (i= 1; i < len+3; i++)
    RX_FIFO_READ(i, &rx_fifo[i]);

  // Clear RXDECINV bit (enable reception)
  SHORT_WRITE(BBREG1, 0);

  // Frame length includes packet header, payload and FCS

  if (frame != NULL) {
    int index= 1;

    frame->type= rx_fifo[index] & 0x07;
    index++;

    // Frame control
    frame->src_addr.mode= (rx_fifo[index] >> 6) & 0x03;
    frame->dst_addr.mode= (rx_fifo[index] >> 2) & 0x03;
    index++;
    frame->seqnum= rx_fifo[index++];

    // Destination address
    switch (frame->dst_addr.mode) {
    case 0x02:
      frame->dst_addr.m2.panid= (rx_fifo[index+1] << 8) | rx_fifo[index];
      frame->dst_addr.m2.sadr= (rx_fifo[index+3] << 8) | rx_fifo[index+2];
      index+= 4;
      break;
    case 0x03:
      memcpy(frame->dst_addr.m3.ladr, &rx_fifo[index], MAC_ADDR_M3_LEN);
      index+= 8;
      break;
    }

    // Source address
    switch (frame->src_addr.mode) {
    case 0x02:
      frame->src_addr.m2.panid= (rx_fifo[index+1] << 8) | rx_fifo[index];
      frame->src_addr.m2.sadr= (rx_fifo[index+3] << 8) | rx_fifo[index+2];
      index+= 4;
      break;
    case 0x03:
      memcpy(frame->src_addr.m3.ladr, &rx_fifo[index], MAC_ADDR_M3_LEN);
      index+= 8;
      break;
    }

    // Security fields ?

    // Payload
    memcpy(frame->payload, &rx_fifo[index], len-2-(index-1));

    // CRC (FCS)
    frame->crc= (rx_fifo[len] << 8) | rx_fifo[len-1];

    frame->len= len-2-(index-1);

  }

  if (rssi != NULL)
    *rssi= rx_fifo[len+1];
  if (lqi != NULL)
    *lqi= rx_fifo[len+2];

  return 1;
}

inline void byte2binstr(unsigned char b, char * buf, size_t len) {
  int i;
  assert(len >= 9);
  for (i= 0; i < 8; i++)
    buf[i]= ((b & (1 << (7-i))) != 0)?'1':'0';
  buf[8]= '\0';
}

// -----[ mrf24j40_xtalk_test ]--------------------------------------
/**
 * Simple test to measure SPI error rate. Useful e.g. when
 * experimenting with a breadboard.
 * Test is based on writing/reading a value several times to
 * the PANIDL register.
 *
 * If the number of measurements (num_meas) is less than 0,
 * then the test will run forever.
 *
 * In my experience, I have had a lot of crosstalk in the form
 * of spurious clock edges which led to shifted values. Crosstalks
 * were due to long, parallel cables on a breadboard.
 */
#ifdef __BUSPIRATE__
int mrf24j40_xtalk_test(DEVICE * bp, int num_meas) {
  //const unsigned char short_reg_addr= MRF24J40_SREG_PANIDL;
  const unsigned short long_reg_addr= MRF24J40_MEM_OFFSET_SEC_BUF;
  int count= 0, old_count= 0;
  int total= 0;

  unsigned char w_value= 0;
  unsigned char r_value;
  
  while ((num_meas > 0) || (num_meas < 0)) {
    //if (mrf24j40_short_write(bp, reg_addr, w_value) < 0)
    //  return -1;
    //if (mrf24j40_short_read(bp, reg_addr, &r_value) < 0)
    //  return -1;
    if (mrf24j40_long_write(bp, long_reg_addr, w_value) < 0)
      return -1;
    if (mrf24j40_long_read(bp, long_reg_addr, &r_value) < 0)
      return -1;
    if (r_value != w_value) {
      char r_buf[9], w_buf[9];
      byte2binstr(w_value, w_buf, sizeof(w_buf));
      byte2binstr(r_value, r_buf, sizeof(r_buf));
      printf("Mismatch: %X (%s) -?-> %X (%s)\n",
	     w_value, w_buf, r_value, r_buf);
      count++;
    }
    total++;
    if ((old_count != count) || (total % 100 == 0)) {
      printf("\rNumber of mismatches: %d/%d (%.1f%%)", count, total,
	     ((double) count)/total*100);
      fflush(stdout);
      old_count= count;
    }
    w_value++;
    if (num_meas > 0)
      num_meas--;
  }
  printf("\rNumber of mismatches: %d/%d (%.1f%%)\n", count, total,
	 ((double) count)/total*100);
  return count;
}
#endif /* __BUSPIRATE__ */
