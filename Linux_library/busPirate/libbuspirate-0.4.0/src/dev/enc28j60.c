// ==================================================================
// @(#)enc28j60.c
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <enc28j60.h>
#include <spi.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//#define DEBUG
#define DEBUG_STREAM stderr
#define DEBUG_ID     "ENC28J60:"
#include <debug.h>

#define ENC28J60_SPI_RCR 0x00
#define ENC28J60_SPI_WCR 0x40
#define ENC28J60_SPI_RBM 0x3A
#define ENC28J60_SPI_WBM 0x7A
#define ENC28J60_SPI_BFS 0x80
#define ENC28J60_SPI_BFC 0xA0

#define ENC28J60_MEM_RX_START 0x0000
#define ENC28J60_MEM_RX_END   0x0FFF

#define ENC28J60_MAX_FRAME_LEN 1518

#define STATUS_BIT_BROADCAST (1 << 25)

const char * STATUS_VECTOR_BIT_NAMES[15]= {
  "Long Event/Drop Event",
  "Reserved",
  "Carrier Event Previously Seen",
  "Reserved",
  "CRC Error",
  "Length Check Error",
  "Length Out of Range",
  "Received Ok",
  "Receive Multicast Packet",
  "Receive Broadcast Packet",
  "Dribble Nibble",
  "Receive Control Frame",
  "Receive Pause Control Frame",
  "Receive Unknown Opcode",
  "Receive VLAN Type Detected",
};

#define ETHER_TYPE_IP4      0x0800
#define ETHER_TYPE_ARP      0x0806
#define ETHER_TYPE_IP6      0x86DD
#define ETHER_TYPE_HOMEPLUG 0x88E1
#define ETHER_TYPE_BP       0x4000

static eth_addr_t ETHER_ADDR_BP=
  {0x12, 0x34, 0x56, 0x89, 0xAB, 0xCD};
static eth_addr_t ETHER_ADDR_BCAST=
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static char ANIMATE_CARS[8]= "-\\|/-\\|/";


// -----[ enc28j60_cr_write ]----------------------------------------
/**
 * Write Control Register (WCR)
 */
int enc28j60_cr_write(BP * bp, unsigned char addr, unsigned char value) {
  __debug__("cr_write addr=0x%.2x value=0x%.2x\n", addr, value);
  assert(addr <= 0x1F);
  unsigned char data[2]= { ENC28J60_SPI_WCR | (addr & 0x1F), value };
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  if (bp_bin_spi_bulk(bp, data, 2) < 0)
    return -1;
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  return 0;
}

// -----[ enc28j60_cr_read ]-----------------------------------------
/**
 * Read Control Register (RCR)
 */
int enc28j60_cr_read(BP * bp, unsigned char addr, unsigned char * value) {
  __debug__("cr_read addr=0x%.2x\n", addr);
  assert(addr <= 0x1F);
  unsigned char data[2]= { ENC28J60_SPI_RCR | (addr & 0x1F), 0xFF };
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  if (bp_bin_spi_bulk(bp, data, 2) < 0)
    return -1;
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  __debug__("  value=0x%.2x\n", data[1]);
  if (value != NULL)
    *value= data[1];
  return 0;
}

// -----[ enc28j60_mem_read ]----------------------------------------
/**
 * Read buffer memory (RBM)
 */
int enc28j60_mem_read(BP * bp, unsigned int size, unsigned char * buf) {
  __debug__("mem_read size=%d\n", size);
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  unsigned char cmd= ENC28J60_SPI_RBM;
  if (bp_bin_spi_bulk(bp, &cmd, 1) < 0)
    return -1;
  __debug__("  ");
  while (size > 0) {
    int s= size;
    if (s > 16)
      s= 16;
    memset(buf, 0xff, s);
    if (bp_bin_spi_bulk(bp, buf, s) < 0)
      return -1;
    __debug_more_hex_buf__(buf, s);
    size-= s;
    buf+= s;
  }
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  __debug__("\n");
  return 0;
}

// -----[ enc28j60_mem_write ]---------------------------------------
/**
 * Write buffer memory (WBM)
 */
int enc28j60_mem_write(BP * bp, unsigned int size, unsigned char * buf) {
  __debug__("mem_write size=%d\n", size);
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  unsigned char cmd= ENC28J60_SPI_WBM;
  if (bp_bin_spi_bulk(bp, &cmd, 1) < 0)
    return -1;
  while (size > 0) {
    int s= size;
    if (s > 16)
      s= 16;
    if (bp_bin_spi_bulk(bp, buf, s) < 0)
      return -1;
    size-= s;
    buf+= s;
  }
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  return 0;
}

// -----[ enc28j60_bit_field_set ]-----------------------------------
int enc28j60_bit_field_set(BP * bp, unsigned char addr, unsigned char bits) {
  assert(addr <= 0x1F);
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  unsigned char data[2]= { ENC28J60_SPI_BFS + addr, bits };
  if (bp_bin_spi_bulk(bp, data, 2) < 0)
    return -1;  
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  return 0;
}
  
// -----[ enc28j60_bit_field_clear ]---------------------------------
int enc28j60_bit_field_clear(BP * bp, unsigned char addr, unsigned char bits) {
  assert(addr <= 0x1F);
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  unsigned char data[2]= { ENC28J60_SPI_BFC + addr, bits };
  if (bp_bin_spi_bulk(bp, data, 2) < 0)
    return -1;  
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  return 0;
}

// -----[ enc28j60_bank_select ]-------------------------------------
/**
 * Select register bank
 */
int enc28j60_bank_select(BP * bp, unsigned char bank) {
  __debug__("bank_select bank=%d\n", bank);
  unsigned char value;
  assert(bank <= 3);
  if (enc28j60_cr_read(bp, ENC28J60_REG_ECON1, &value) < 0)
    return -1;
  value= (value & 0xFC) | (bank & 0x03);
  if (enc28j60_cr_write(bp, ENC28J60_REG_ECON1, value) < 0)
    return -1;
  return 0;
}


#define CR_READ(DEV, REG, VALPTR) \
  if (enc28j60_cr_read((DEV), ENC28J60_REG_##REG, (VALPTR)) < 0) \
    return -1.
#define CR_WRITE(DEV, REG, VAL) \
  if (enc28j60_cr_write((DEV), ENC28J60_REG_##REG, (VAL)) < 0) \
    return -1.
#define BANK_SELECT(DEV, BANK) \
  if (enc28j60_bank_select((DEV), (BANK)) < 0) \
    return -1;


// -----[ enc28j60_init ]--------------------------------------------
/**
 * Ethernet controller initialization
 */
int enc28j60_init(BP * bp, eth_addr_t addr) {
  __debug__("init\n");
  unsigned char value;

  // Initialization
  // 1. Receive / transmit buffers
  //   ERXST, ERXND pointers
  //   Note: receive buffer should start at 0000h (see errata)
  BANK_SELECT(bp, 0);
  CR_WRITE(bp, B0_ERXSTL, 0x00);
  CR_WRITE(bp, B0_ERXSTH, 0x00);

  CR_WRITE(bp, B0_ERXNDL, 0xFF);
  CR_WRITE(bp, B0_ERXNDH, 0x0F);

  // Initializes ERDPT used for reading memory buffer
  // (don't know why, but it is initialized to 0x05FA)
  CR_WRITE(bp, B0_ERDPTL, 0x00);
  CR_WRITE(bp, B0_ERDPTH, 0x00);

  // 2. Receive filters
  //   ERXFCON
  value= 0x00;
  value|= 0x80; // UCEN = 1 (Unicast Filter Enable bit)
  value&= 0xBF; // ANDOR = 0 (AND/OR Filter Select bit)
  value|= 0x20; // CRCEN = 1 (Post-Filter CRC Check Enable bit)
  value|= 0x01; // BCEN = 1 (Broadcast Filter Enable bit)
  BANK_SELECT(bp, 1);
  CR_WRITE(bp, B1_ERXFCON, value);

  // 3. Wait for oscillator start-up (OST)
  //   ESTAT.CLKRDY (bit 0)
  /* Errata (2010) says that waiting for CLKRDY does not always work
     => better to wait for 1ms (here we wait 10ms)
    do {
    CR_READ(bp, ESTAT, &value);
    } while (!(value & 0x01));*/

  usleep(10000);

  // 4. MAC initialization
  // 4.1. Enable frame reception + full-duplex
  //     MACON1.MARXEN,.TXPAUS,.RXPAUS
  BANK_SELECT(bp, 2);
  value= 0x00;
  value|= 0x01; // MARXEN = 1 (MAC Receive Enable bit)
  //value|= 0x02; // PASSALL = 1 (Pass All Received Frames Enable bit)
  value|= 0x04; // RXPAUS = 1 (Pause Control Frame Reception Enable bit)
  value|= 0x08; // TXPAUS = 1 (Pause Control Frame Transmission Enable bit)
  CR_WRITE(bp, B2_MACON1, value);

  // 4.2. auto-padding, auto-CRC, frame length status, full-duplex
  value= 0x00;
  value|= 0xA0; // PADCFG2:0 = 101 (Automatic Pac and CRC Configuration bits)
  value|= 0x10; // TXCRCEN = 1 (Transmit CRC Enable bit)
  value|= 0x02; // FRMLNEN = 1 (Frame Length Checking Enable bit)
  value|= 0x01; // FULDPX = 1 (MAC Full-Duplex Enable bit)
  CR_WRITE(bp, B2_MACON3, value);

  // 4.3. defer transmission until media idle
  value= 0x00;
  value|= 0x40; // DEFER = 1 (Defer Transmission Enable bit)
  CR_WRITE(bp, B2_MACON4, value);

  // 4.4. maximum frame length
  CR_WRITE(bp, B2_MAMXFLL, ENC28J60_MAX_FRAME_LEN & 0xFF);
  CR_WRITE(bp, B2_MAMXFLH, ENC28J60_MAX_FRAME_LEN >> 8);

  // 4.5. back-to-back inter-packet gap
  CR_WRITE(bp, B2_MABBIPG, 0x15); // when full-duplex
  //CR_WRITE(bp, B2_MABBIPG, 0x12); // when half-duplex

  // 4.6. non-back-to-back inter-packet gap
  CR_WRITE(bp, B2_MAIPGL, 0x12);
  CR_WRITE(bp, B2_MAIPGH, 0x0C);

  // 4.7. number of retransmissions, collision window
  // default values should be OK

  // 4.8. unicast MAC address
  BANK_SELECT(bp, 3);
  CR_WRITE(bp, B3_MAADR1, addr[0]);
  CR_WRITE(bp, B3_MAADR2, addr[1]);
  CR_WRITE(bp, B3_MAADR3, addr[2]);
  CR_WRITE(bp, B3_MAADR4, addr[3]);
  CR_WRITE(bp, B3_MAADR5, addr[4]);
  CR_WRITE(bp, B3_MAADR6, addr[5]);
  
  // 5. PHY initialization
  // nothing to do ?

  return 0;
}

// -----[ enc28j60_rx_enable ]---------------------------------------
/**
 * Enable/disable frame reception
 */
int enc28j60_rx_enable(BP * bp, int enable) {
  __debug__("rx_enable enable=%d\n", enable);
  unsigned char value;
  CR_READ(bp, ECON1, &value);
  if (enable)
    value|= 0x04; // RXEN = 1
  else
    value&= 0xFB; // RXEN = 0
  CR_WRITE(bp, ECON1, value);
  return 0;
}

// -----[ enc28j60_read_pktcnt ]-------------------------------------
/**
 * Read packet count register
 */
int enc28j60_read_pktcnt(BP * bp, unsigned char * pktcnt) {
  __debug__("read_pktcnt\n");
  BANK_SELECT(bp, 1);
  CR_READ(bp, B1_EPKTCNT, pktcnt);
  return 0;
}

// -----[ enc28j60_frame_available ]---------------------------------
/**
 * Return 1 if a frame is available, 0 if no frame is available,
 *        < 0 if an error occured
 */
int enc28j60_frame_available(BP * bp) {
  __debug__("frame_available\n");
  unsigned char value;
    CR_READ(bp, EIR, &value);
    if (value & 0x40)
      return 1;
    return 0;
}


// -----[ enc28j60_read_frame ]--------------------------------------
/**
 * Read next frame from buffer memory
 */
int enc28j60_read_frame(BP * bp, unsigned char * buf, unsigned int size,
			unsigned long * status) {
  __debug__("read_frame size=%d\n");
  unsigned int erdpt;
  unsigned int next_pkt_addr;
  unsigned char pkt_hdr[6];
  unsigned char value;

  BANK_SELECT(bp, 0);
  CR_READ(bp, B0_ERDPTL, &value);
  erdpt= value;
  CR_READ(bp, B0_ERDPTH, &value);
  erdpt+= (value << 8);

  // Read packet header
  if (enc28j60_mem_read(bp, sizeof(pkt_hdr), pkt_hdr) < 0)
    return -1;

  next_pkt_addr= ((pkt_hdr[1] << 8) + pkt_hdr[0]);
  *status= ((pkt_hdr[5] << 24) + (pkt_hdr[4] << 16) +
	    (pkt_hdr[3] << 8) + pkt_hdr[2]);

  // Determine packet size
  unsigned int pkt_size;
  if (next_pkt_addr > erdpt)
    pkt_size= next_pkt_addr-erdpt-6;
  else
    pkt_size= (0x1000-erdpt) + next_pkt_addr - 6;

  assert(size >= pkt_size);
  // Read packet
  if (enc28j60_mem_read(bp, pkt_size, buf) < 0)
    return -1;

  // Update receive read pointer
  BANK_SELECT(bp, 0);
  CR_WRITE(bp, B0_ERXRDPTL, buf[0]);
  CR_WRITE(bp, B0_ERXRDPTH, buf[1]);

  // Decrease packet count
  // ECON2.PKTDEC = 1 (Packet Decrement bit)
  if (enc28j60_bit_field_set(bp, ENC28J60_REG_ECON2, 0x40) < 0)
    return -1;

  return pkt_size;
}

// -----[ enc28j60_send_frame ]--------------------------------------
/**
 * Write frame into memory and request to send.
 */
int enc28j60_send_frame(BP * bp, eth_addr_t da, eth_addr_t sa,
			int type_len, unsigned char * payload, int len)
{
  unsigned char buf[4096];
  unsigned int frame_size= 14 + len;

  /* Memory layout
     ETXST --> [ control byte ]
               [ data 0       ]
	       [ data 1       ]
               [   ...        ]
     ETXND --> [ data N-1     ]
  */     

  /* MACON3 is used to determine how the packet is transmitted.
     However, a per packet control can be used to override it.
     7-4 : unused
     3-1 : override content of MACON3 ; if 0, no override */
  buf[0]= 0x00; 

  /* Destination Address */
  memcpy(&buf[1], da, 6);
  /* Source Address */
  memcpy(&buf[7], sa, 6);
  /* Type / Length */
  buf[13]= (type_len & 0xFF00) >> 8;
  buf[14]= (type_len & 0xFF);
  /* Payload */
  memcpy(&buf[15], payload, len);

  BANK_SELECT(bp, 0);

  CR_WRITE(bp, B0_EWRPTL, 0x00);
  CR_WRITE(bp, B0_EWRPTH, 0x10);

  CR_WRITE(bp, B0_ETXSTL, 0x00);
  CR_WRITE(bp, B0_ETXSTH, 0x10);

  if (enc28j60_mem_write(bp, frame_size + 1 + len, buf) < 0)
    return -1;

  CR_WRITE(bp, B0_ETXNDL, 0x00 + frame_size + 1 + len);
  CR_WRITE(bp, B0_ETXNDH, 0x10);

  unsigned char tmp;
  CR_READ(bp, EIR, &tmp);
  tmp&= 0xF7; // Clear EIR.TXIF
  CR_WRITE(bp, EIR, tmp);

  /* Set TXRTS bit -- Transmit Request To Send */
  if (enc28j60_bit_field_set(bp, ENC28J60_REG_ECON1, 0x08) < 0)
    return -1;
	
  return 0;
}

static void _wait_animate()
{
  static int i;
  usleep(10000);
  printf("\r%c", ANIMATE_CARS[i % 8]);
  fflush(stdout);
  i++;
}

#ifndef _WIN32
static void _dump_frame_to_file(const char * prefix,
				unsigned int pktcnt,
				unsigned char * buf,
				unsigned int pkt_size) {
  int j;
  char filename[128];
  
  snprintf(filename, sizeof(filename), "%s-%d", prefix, pktcnt);
  FILE * out= fopen(filename, "w");
  assert(out != NULL);

  for (j= 0; j < pkt_size; j++) {
    fprintf(out, " %.2X", buf[j]);
    if ((j % 16) == 15)
      fprintf(out, "\n");
  }
  fprintf(out, "\n");
  fclose(out);
}
#endif /* _WIN32 */

static void _show_eth_addr(eth_addr_t addr)
{
  printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
	 addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static void _show_eth_type(int type)
{
  switch (type) {
  case ETHER_TYPE_IP4: printf("IPv4"); break;
  case ETHER_TYPE_ARP: printf("ARP"); break;
  case ETHER_TYPE_IP6: printf("IPv6"); break;
  case ETHER_TYPE_HOMEPLUG: printf("HomePlug"); break;
  default: printf("?");
  }
}

static void _show_frame_info(unsigned int pktcnt,
			     unsigned char * buf,
			     int pkt_size,
			     unsigned long status) {
  int i;
  printf("Frame %d received\n", pktcnt);
  printf("  status:%.8x\n", (unsigned int) status);
  printf("    length:%lu\n", status & 0x0000FFFF);
  for (i= 0; i < 15; i++) {
    if ((status >> 16) & (1 << i)) {
      printf("    bit %s\n", STATUS_VECTOR_BIT_NAMES[i]);
    }
  }
  printf("  dst :");
  _show_eth_addr(*((eth_addr_t *) buf));
  printf("\n");
  printf("  src :");
  _show_eth_addr(*((eth_addr_t *) (buf+6)));
  printf("\n");
  unsigned int type= ((buf[12] << 8) + buf[13]);
  printf("  type:%.4x (", type);
  _show_eth_type(type);
  printf(")\n");
}

// -----[ demo_spi_enc28j60 ]----------------------------------------
/**
 * Simple demo - captures any received frame
 */
int demo_spi_enc28j60(BP * bp)
{
  unsigned char version;
  unsigned char config;

  printf("*** ENC28J60 send SPI demo ***\n");

  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) < 0)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  printf("Setting SPI speed to 1Mbps\n");
  if (bp_bin_spi_set_speed(bp, BP_BIN_SPI_SPEED_250K) < 0)
    return -1;

  config= BP_BIN_SPI_LV_HIZ /*| BP_BIN_SPI_CLK_EDGE_HIGH*/;
  show_spi_config(config);
  printf("Binary I/O SPI config (%u):\n", config);
  if (bp_bin_spi_set_config(bp, config) < 0)
    return -1;

  usleep(1000);

  printf("Setting SPI periph config (power=ON,pullups=ON)\n");
  if (bp_bin_spi_set_periph(bp, 0x08 | 0x04) < 0)
    return -1;

  usleep(1000);

  /*printf("Reading SPI periph config\n");
  if (bp_bin_spi_get_periph(bp, &config) < 0)
    return -1;
  printf("  config =%u\n", config);
  printf("  power  =%s\n", (config & 0x08)?"ON":"OFF");
  printf("  pullups=%s\n", (config & 0x04)?"ON":"OFF");*/

  printf("Hit enter to continue...");
  getchar();

  BANK_SELECT(bp, 3);
  unsigned char value;
  CR_READ(bp, B3_EREVID, &value);
  printf("EREVID: %.2x\n", value);

  printf("Ethernet address: ");
  _show_eth_addr(ETHER_ADDR_BP);
  printf("\n");
  if (enc28j60_init(bp, ETHER_ADDR_BP) < 0)
    return -1;

  if (enc28j60_rx_enable(bp, 1) < 0)
    return -1;

  int pktcnt= 0;
  do {
    int available= enc28j60_frame_available(bp);
    if (available < 0)
      return -1;
    if (available) {
      printf("\r");
      fflush(stdout);

      unsigned long status;
      unsigned char buf[4096];
      int pkt_size= enc28j60_read_frame(bp, buf, sizeof(buf), &status);
      if (pkt_size < 0)
	return -1;
      unsigned short type= (buf[12] << 8) + buf[13];
      _show_frame_info(pktcnt, buf, pkt_size, status);
#ifndef _WIN32
      _dump_frame_to_file("/tmp/enc28j60.memory", pktcnt, buf, pkt_size);
#endif /* _WIN32 */
      pktcnt++;

      if ((type == ETHER_TYPE_BP) &&
	  !(status & STATUS_BIT_BROADCAST)) {

	/* Ping frame received, send response */
	if (enc28j60_send_frame(bp, ETHER_ADDR_BCAST,
				ETHER_ADDR_BP, ETHER_TYPE_BP,
				(unsigned char *) "BUS PIRATE",
				10) < 0)
	  return -1;

	/* Wait for end of transmission */
	while (1) {
	  unsigned char tmp;
	  CR_READ(bp, ECON1, &tmp);
	  if (!(tmp & 0x08)) {
	    printf("Transmission completed :-)\n");
	    
	    CR_READ(bp, ECON1, &tmp);
	    printf("ECON1=%.2x\n", tmp);
	    CR_READ(bp, EIR, &tmp);
	    printf("EIR  =%.2x\n", tmp);
	    
	    break;
	  } else
	    _wait_animate();
	}
      }

    } else
      _wait_animate();
  } while (loop);

  return 0;
}
