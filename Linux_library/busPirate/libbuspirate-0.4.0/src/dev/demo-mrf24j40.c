// ==================================================================
// @(#)main.c
//
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 12/12/2010
// $Id$
//
// MRF24J40 SPI demo
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
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <buspirate.h>
#include <spi.h>
#include <mrf24j40.h>

unsigned char  CHANNEL        = 11;
unsigned short PAN_ID         = 0x0001;
unsigned short SRC_SHORT_ADDR = 0x0002;
unsigned char  SRC_LONG_ADDR[]=
  {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89};
unsigned short DST_SHORT_ADDR = 0x0001;
unsigned char  TX_INTERVAL    = 5;
int            ACK_REQUEST    = 1;
int            PROMISCUOUS    = 0;

//#define __ENABLE_XTALK_TEST__  1


// -----[ signal_handler ]-------------------------------------------
/**
 * Signal handler used to catch ctrl-C (SIGINT) and change the value
 * of the _loop variable.
 */
volatile int loop= 1;
static void signal_handler(int signum) {
  fprintf(stdout, "Ctrl-C received\n");
  fflush(stdout);
  loop= 0;
}

// -----[ configure_bus_pirate ]-------------------------------------
/**
 * Configure the bus pirate to talk with MRF24J40 transceiver
 *
 * Protocol    : SPI (mode 0)
 * Speed       : 8MHz (maximum, MRF24J40 supports up to 20MHz)
 * Supply power: yes
 * Output mode : open-drain + pull-ups (Vpu = 3.3V)
 */
static int configure_bus_pirate(BP * bp)
{
  unsigned char version;
  unsigned char config;

  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) != BP_SUCCESS)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  printf("Setting SPI speed to 8MHz\n");
  if (bp_bin_spi_set_speed(bp, BP_BIN_SPI_SPEED_8M) != BP_SUCCESS)
    return -1;

  config= BP_BIN_SPI_LV_HIZ;
  printf("Setting binary I/O SPI config (%u):\n", config);
  if (bp_bin_spi_set_config(bp, config) != BP_SUCCESS)
    return -1;

  usleep(1000);

  printf("Setting SPI periph config (power=ON,pullups=ON)\n");
  if (bp_bin_spi_set_periph(bp, 0x08 | 0x04) != BP_SUCCESS)
    return -1;

  usleep(1000);

  return 0;
}

static double timer_future;

void timer_start(double seconds)
{
  struct timeval tv;
  assert(seconds > 0);
  assert(gettimeofday(&tv, NULL) >= 0);
  timer_future= (tv.tv_sec + 0.000001 * tv.tv_usec) + seconds;
}

int timer_expired()
{
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) >= 0);
  double now= tv.tv_sec + 0.000001 * tv.tv_usec;
  return (now >= timer_future);
}


//#define CONSTANT_FRAME "HELLO BUSPIRATE"
#define MAX_PAYLOAD_LEN 100
int next_frame_data(char * payload)
{
#ifdef CONSTANT_FRAME
  
  strcpy(payload, CONSTANT_FRAME);
  return strlen(CONSTANT_FRAME);
   
#else /* CONSTANT_FRAME */
  
  static unsigned char len= 1;
  if (len > MAX_PAYLOAD_LEN)
    len= 1;
  unsigned char i;
  for (i= 0; i < len; i++)
    *(payload+i)= (char) i;
  len++;
  return len-1;
  
#endif /* CONSTANT_FRAME */
}

// -----[ demo_mrf24j40_rxtx ]---------------------------------------
/**
 * The purpose of this demo is to show how to send/receive
 * IEEE 802.15.4 frames with the MRF24J40 transceiver and the bus
 * pirate as an USB/SPI adapter.
 */
int demo_mrf24j40_rxtx(BP * bp, int argc, char * argv[]) {
  mac_frame frame;
  unsigned char rssi, lqi;
  int ncount= 0;
  unsigned char seq_num= 0;
  time_t start_time= time(NULL);
  char MSG_DATA[MAX_PAYLOAD_LEN + 1];
  int len;
  struct {
    int tx_ok;
    int tx_err;
    int rx_ok;
  } stats= { 0, 0, 0 };

  printf("*** MRF24J40 rx/tx SPI demo ***\n");

  // First configure bus pirate for SPI with MRF24J40
  if (configure_bus_pirate(bp) < 0)
    return -1;

  printf("Hit enter to continue...");
  getchar();

  // If required perform crosstalk test to ensure the SPI
  // bus is reliable.
#ifdef __ENABLE_XTALK_TEST__
  if (mrf24j40_xtalk_test(bp, 500) != 0)
    return -1;
#endif /* __ENABLE_XTALK_TEST__ */

  // Initialize MRF24J40 transceiver. This is required to setup the
  // transceiver's radio, channel and addresses
  if (mrf24j40_init(bp, PAN_ID, SRC_SHORT_ADDR, SRC_LONG_ADDR, CHANNEL, 1) < 0)
    return -1;

  if (PROMISCUOUS)
    if (mrf24j40_promiscuous(bp, 0) < 0)
      return -1;

  if (TX_INTERVAL != 0)
    timer_start(TX_INTERVAL);
  
  // The demo loops forever (until an error occurs or ctrl-C is
  // pressed). It regularly send frames, then wait for the TX
  // status. It will also handle and dissect any received frame.
  while (loop) {

    
    if ((TX_INTERVAL > 0) && timer_expired()) {
      timer_start(TX_INTERVAL);
      len= next_frame_data(MSG_DATA);
      
      printf("\rSending packet %s(seq_num:%x)...       \n",
	     (ACK_REQUEST?"with ACK ":""), seq_num);
      if (mrf24j40_tx(bp, PAN_ID, SRC_SHORT_ADDR, DST_SHORT_ADDR, seq_num,
		      ACK_REQUEST?1:0,
		      MSG_DATA, len) < 0)
	return -1;
	seq_num++;
    }

    printf("\rWaiting for status %c", "|/-\\|/-\\"[(ncount++) % 8]);
    fflush(stdout);

    // To the contrary of the operation with an MCU, the bus pirate
    // cannot report external interrupts, therefore polling is used
    // (see mrf24j40_instat_read). Two sources of interrupt are
    // handled: (a) TX status is available and (b) a frame has been
    // received.
    int intstat;
    if (mrf24j40_intstat_read(bp, &intstat) < 0)
      return -1;

    // Process TX status
    if (intstat & MRF24J40_INTSTAT_TXNIF) {

      int status;
      if (mrf24j40_tx_status(bp, &status) < 0)
	return -1;
      
      if (status != MRF24J40_TX_SUCCESS) {
	stats.tx_err++;
	printf("\rTransmission failed, retry count exceeded :-(\n");
      } else {
	stats.tx_ok++;
	printf("\rTransmission successful :-)\n");
      }

    }

    // Process incoming frame
    if (intstat & MRF24J40_INTSTAT_RXIF) {

      printf("\r                    ");
      printf("\rReceiving");
      fflush(stdout);
      
      if (mrf24j40_rx(bp, &frame, &rssi, &lqi) < 0)
	return -1;
      
      stats.rx_ok++;
      printf("\rFrame #%u received at %.2f [RSSI=%u,LQI=%u] (%d/%d)\n",
	     stats.rx_ok, difftime(time(NULL), start_time), rssi, lqi,
	     stats.tx_ok, stats.tx_err+stats.tx_ok);
      mac_frame_dump(stdout, &frame);
      printf("\n");

    }

    // If there is no interesting interrupt, we do nothing except
    // relinquish control to system (this is to avoid using 100%
    // CPU). Note that we need to mask interrupts of interest as
    // other sources of interrupt are possible and we will not
    // process them (and might not clear the related interrupt
    // flag).
    if (!(intstat & (MRF24J40_INTSTAT_RXIF | MRF24J40_INTSTAT_TXNIF)))
      usleep(0);

  }

  return 0;
}

// ------------------------------------------------------------------
/**
 *
 */
void help(const char * msg, ...)
{
  va_list ap;

  if (msg != NULL) {
    va_start(ap, msg);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, msg, ap);
    va_end(ap);
  }

  printf("\n\n");
  printf("usage: demo_mrf24j40 <device>\n");

  printf("\n");
  printf("--no-ack-req        disables ACK requests\n");
  printf("--channel=<ch>      specifies RF channel\n");
  printf("--mac=<addr>        specifies long address\n");
  printf("--src=<addr>        specifies source short address\n");
  printf("--dst=<addr>        specifies destination short address\n");
  printf("--pan-id            specifies PAN id\n");
  printf("--interval=<int>    specifies interval between transmissions\n");

  if (msg != NULL)
    exit(EXIT_FAILURE);
}

static struct option long_options[]= {
  {"no-ack-request", no_argument, 0, 'a'},
  {"channel", required_argument, 0, 'c'},
  {"mac", required_argument, 0, 'm'},
  {"src", required_argument, 0, 's'},
  {"dst", required_argument, 0, 'd'},
  {"pan-id", required_argument, 0, 'p'},
  {"interval", required_argument, 0, 'i'},
  {0, 0, 0, 0}
};

int hexc2i(char c, int * i) {
  if (('0' <= c) && (c <= '9'))
    *i= c-'0';
  else if (('a' <= c) && (c <= 'f'))
    *i= c-'a'+10;
  else if (('A' <= c) && (c <= 'F'))
    *i= c-'A'+10;
  else
    return -1;
  return 0;
}

int hexc2uc(const char * hex, unsigned char * uc) {
  int i1, i2;
  if (hexc2i(hex[0], &i1) < 0)
    return -1;
  if (hexc2i(hex[1], &i2) < 0)
    return -1;
  *uc= (i1 << 4) + i2;
  return 0;
}

int parse_long_addr(const char * mac, unsigned char pmac[]) {
  int i;
  for (i= 0; i < 8; i++) {
    if (hexc2uc(mac, &pmac[i]) < 0)
      return -1;
    mac+= 2;

    if (i < 7) {
      if (*mac != ':')
	return -1;
    } else {
      if (*mac != '\0')
	return -1;
    }
    mac++;
  }
  return 0;
}

int parse_short_addr(const char * mac, unsigned short * saddr) {
  unsigned char uc1, uc2;
  if (hexc2uc(mac, &uc1) < 0)
    return -1;
  mac+= 2;
  if (hexc2uc(mac, &uc2) < 0)
    return -1;
  mac+= 2;
  *saddr= (((unsigned short) uc1) << 8) + uc2;
  return 0;
}

int parse_byte(const char * str, unsigned char * byte,
	       unsigned char min, unsigned char max) {
  char * endptr= NULL;
  long val= strtol(str, &endptr, 0);
    if (((val == 0) && (errno == EINVAL)) ||
	(errno == ERANGE))
    return -1;
  if ((val < min) || (val > max))
    return -1;
  *byte= (unsigned char) val;
  return 0;
}

int parse_options(int argc, char * argv[]) {
  while (1) {
    int option_index= 0;

    int c= getopt_long(argc, argv, "", long_options, &option_index);

    /* End of options */
    if (c == -1)
      break;

    switch (c) {
    case 'a':
      ACK_REQUEST= 0;
      break;
    case 'c':
      if (parse_byte(optarg, &CHANNEL, 11, 26) < 0)
	help("invalid value for option --channel\n");
      break;
    case 'm':
      if (parse_long_addr(optarg, SRC_LONG_ADDR) < 0)
	help("invalid value for option --mac\n");
      break;
    case 's':
      if (parse_short_addr(optarg, &SRC_SHORT_ADDR) < 0)
	help("invalid short address for option --src\n");
      break;
    case 'd':
      if (parse_short_addr(optarg, &DST_SHORT_ADDR) < 0)
	help("invalid short address for option --dst\n");
      break;
    case 'p':
      if (parse_short_addr(optarg, &PAN_ID) < 0)
	help("invalid PAN ID for option --pan-id\n");
      break;
    case 'i':
      if (parse_byte(optarg, &TX_INTERVAL, 0, 255) < 0)
	help("invalid value for option --interval\n");
      break;
    }
  }
  return 0;
}

void show_params() {
  printf("Parameters:\n");
  printf("  ack-requests  : %s\n", (ACK_REQUEST?"yes":"no"));
  printf("  channel       : %d\n", CHANNEL);
  printf("  pan_id        : %.4X\n", PAN_ID);
  printf("  src_long_addr : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
	 SRC_LONG_ADDR[0], SRC_LONG_ADDR[1], SRC_LONG_ADDR[2],
	 SRC_LONG_ADDR[3], SRC_LONG_ADDR[4], SRC_LONG_ADDR[5],
	 SRC_LONG_ADDR[6], SRC_LONG_ADDR[7]);
  printf("  src_short_addr: %.4X\n", SRC_SHORT_ADDR);
  printf("  dst_short_addr: %.4X\n", DST_SHORT_ADDR);
  printf("  promiscuous   : %s\n", (PROMISCUOUS?"yes":"no"));
}

// ------------------------------------------------------------------
/**
 * Main program
 */
int main(int argc, char * argv[])
{
  BP * bp;
  unsigned char version;

  parse_options(argc, argv);

  /* Check number of non-option arguments */
  if (argc-optind < 1)
    help("not enough arguments");

  show_params();

  bp= bp_open(argv[optind]);
  if (bp == NULL) {
    fprintf(stderr, "Could not open bus pirate\n");
    goto fail;
  }

  if (bp_bin_init(bp, &version) != BP_SUCCESS) {
    fprintf(stderr, "Could not enter binary mode\n");
    goto fail;
  }
  printf("Binary I/O mode version: %u\n", version);

  assert(signal(SIGINT, signal_handler) != SIG_ERR);

  demo_mrf24j40_rxtx(bp, argc, argv);

  printf("Reset device to user terminal\n");
  if (bp_reset(bp) != BP_SUCCESS)
    goto fail;

  printf("Closing.\n");
  bp_close(bp);
  exit(EXIT_SUCCESS);

 fail:
  fprintf(stderr, "Exit with error\n");
  if (bp != NULL)
    bp_close(bp);
  exit(EXIT_FAILURE);
}
