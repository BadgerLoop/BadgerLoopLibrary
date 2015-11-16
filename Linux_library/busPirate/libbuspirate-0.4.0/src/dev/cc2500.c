// ==================================================================
// @(#)cc2500.c
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

#include <stdio.h>
#include <unistd.h>

#include <cc2500.h>
#include <spi.h>

#define DEBUG_ID     "CC2500"
#define DEBUG_STREAM stderr
#include <debug.h>

#define FXOSC 26000000

int cc2500_read_register(BP * bp, unsigned char reg, unsigned char * value)
{
  assert(value != NULL);

  __debug__("Asserting CS\n");
  if (bp_bin_spi_cs(bp, 0) != BP_SUCCESS)
    return CC2500_FAILURE;

  unsigned char data[]= { reg, 0xFF };
  if (bp_bin_spi_bulk(bp, data, 2) != BP_SUCCESS)
    return CC2500_FAILURE;

  __debug__("De-asserting CS\n");
  if (bp_bin_spi_cs(bp, 1) != BP_SUCCESS)
    return CC2500_FAILURE;

  *value= data[1];
  return CC2500_SUCCESS;
}

int cc2500_get_version(BP * bp, unsigned char * version,
		       unsigned char * partnum)
{
  __debug__("Asserting CS\n");
  if (bp_bin_spi_cs(bp, 0) != BP_SUCCESS)
    return CC2500_FAILURE;

  unsigned char data[]= { CC2500_REG_VERSION, 0xFF,
			  CC2500_REG_PARTNUM, 0xFF };
  if (bp_bin_spi_bulk(bp, data, 4) != BP_SUCCESS)
    return CC2500_FAILURE;
  //show_data("  read data:", data, 4);

  __debug__("De-asserting CS\n");
  if (bp_bin_spi_cs(bp, 1) != BP_SUCCESS)
    return CC2500_FAILURE;

  if (version != NULL)
    *version= data[1];
  if (partnum != NULL)
    *partnum= data[3];
  return CC2500_SUCCESS;
}

int cc2500_get_channel(BP * bp, unsigned char * channel)
{
  __debug__("Asserting CS\n");
  if (bp_bin_spi_cs(bp, 0) != BP_SUCCESS)
    return CC2500_FAILURE;

  unsigned char data[]= { 0x80 + CC2500_REG_CHANNR, 0xFF };
  if (bp_bin_spi_bulk(bp, data, 2) != BP_SUCCESS)
    return CC2500_FAILURE;
  //show_data("  read data:", data, 2);

  __debug__("De-asserting CS\n");
  if (bp_bin_spi_cs(bp, 1) != BP_SUCCESS)
    return CC2500_FAILURE;
  return CC2500_SUCCESS;
}

int cc2500_get_freq(BP * bp, unsigned long * freq)
{
  __debug__("Asserting CS\n");
  if (bp_bin_spi_cs(bp, 0) != BP_SUCCESS)
    return CC2500_FAILURE;

  unsigned char data[]= { 0x80 + CC2500_REG_FREQ2, 0xFF,
			  0x80 + CC2500_REG_FREQ1, 0xFF,
			  0x80 + CC2500_REG_FREQ0, 0xFF };
  if (bp_bin_spi_bulk(bp, data, 6) != BP_SUCCESS)
    return CC2500_FAILURE;
  //show_data("  read data:", data, 6);

  __debug__("De-asserting CS\n");
  if (bp_bin_spi_cs(bp, 1) != BP_SUCCESS)
    return CC2500_FAILURE;

  if (freq != NULL) {
    unsigned long the_freq= (data[1] << 16) + (data[3] << 8) + data[5];
    the_freq*= FXOSC;
    the_freq/= (1 << 16);
    *freq= the_freq;
  }
  return CC2500_SUCCESS;
}

int cc2500_get_marcstate(BP * bp, unsigned char * marcstate)
{
  unsigned char value;
  if (cc2500_read_register(bp, CC2500_REG_MARCSTATE, &value)
      != CC2500_SUCCESS)
    return CC2500_FAILURE;

  if (marcstate != NULL)
    *marcstate= value;
  return CC2500_SUCCESS;
}

int cc2500_strobe(BP * bp, unsigned char strobe, unsigned char * status)
{
  __debug__("Asserting CS\n");
  if (bp_bin_spi_cs(bp, 0) != BP_SUCCESS)
    return CC2500_FAILURE;

  unsigned char data[]= { strobe };
  if (bp_bin_spi_bulk(bp, data, 1) != BP_SUCCESS)
    return CC2500_FAILURE;

  __debug__("De-asserting CS\n");
  if (bp_bin_spi_cs(bp, 1) != BP_SUCCESS)
    return CC2500_FAILURE;

  if (status != NULL)
    *status= data[0];
  return CC2500_SUCCESS;
}

// ------------------------------------------------------------------
/**
 * bit 7: chip RDY (low=ready)
 * bits 6:4: status
 * bits 3:0: RX/TX FIFO space
 */
void cc2500_show_status(unsigned char status)
{
  static const char * state[]=
    { "IDLE", "RX", "TX", "FSTXON", "CALIBRATE",
      "SETTLING", "RXFIFO_OVERFLOW", "TXFIFO_UNDERFLOW" };
  printf("CC2500 status (%u):\n", status);
  printf("  chip ready: %s\n", (status & CC2500_STATUS_CHIP_RDY?"NO":"YES"));
  printf("  state     : %s\n", state[(status & 0x70) >> 4]);
  printf("  bytes avlb: %u\n", status & 0x0F);
}

// ------------------------------------------------------------------
/**
 * bits 7:5: reserved
 * bits 4:0: MARC_STATE
 */
void cc2500_show_marcstate(unsigned char marcstate)
{
  static const char * state[]=
    { "SLEEP", "IDLE", "XOFF", "VCOON_MC", "REGON_MC",
      "MANCAL", "VCOON", "REGON", "STARTCAL", "BWBOOST",
      "FS_LOCK", "IFADCON", "ENDCAL", "RX", "RX_END",
      "RX_RST", "TXRX_SWITCH", "RXFIFO_OVERFLOW",
      "FSTXON", "TX", "TX_END", "RXTX_SWITCH",
      "TXFIFO_UNDERFLOW" };
  marcstate&= 0x1F;
  //assert(marcstate <= 22);
  //assert(marcstate != 0x00); /* impossible to read SLEEP state */
  //assert(marcstate != 0x02); /* impossible to read XOFF state */
  if ((marcstate == 0x00) || (marcstate == 0x02) || (marcstate > 22))
    printf("  marc_state: impossible value (%u)!\n", marcstate);
  else 
    printf("  marc_state: %s\n", state[marcstate & 0x1F]);
}

// ------------------------------------------------------------------
/**
 *
 */
void cc2500_show_mdmcfg4_3(unsigned char cfg4, unsigned char cfg3)
{
  unsigned char chanbw_e= (cfg4 & 0xC0) >> 6;
  unsigned char chanbw_m= (cfg4 & 0x30) >> 4;
  unsigned char drate_e= cfg4 & 0x0F;
  unsigned char drate_m= cfg3;
  printf("MDMCFG4\n");
  printf("  CHANBW_E: %u\n", chanbw_e);
  printf("  CHANBW_M: %u\n", chanbw_m);
  unsigned long bw_channel= FXOSC;
  bw_channel/= 8*(4+chanbw_m)*(1 << chanbw_e);
  printf("    -> BWchannel = %lu Hz\n", bw_channel);
  printf("  DRATE_E : %u\n", drate_e);
  printf("MDMCFG3\n");
  printf("  DRATE_M : %u\n", drate_m);
  unsigned long r_data= FXOSC;
  r_data*= (256+drate_m)*(1 << drate_e);
  r_data/= (1L << 28);
  printf("    -> Rdata = %lu baud\n", r_data);
}

// ------------------------------------------------------------------
/**
 *
 */
void cc2500_show_mdmcfg2(unsigned char cfg2)
{
  static const char * mod_format_table[8]=
    {"2-FSK", "GFSK", "-", "OOK", "-", "-", "-", "MSK"};
  static const char * sync_mode_table[8]=
    { "No preamble/sync",
      "15/16 sync word bits detected",
      "16/16 sync word bits detected",
      "30/32 sync word bits detected",
      "No preamble/sync, carries-sense > threshold",
      "15/16 + carrier-sense > threshold",
      "16/16 + carrier-sense > threshold",
      "30/32 + carrier-sense > threshold" };
  unsigned char dem_dcfilt_off= (cfg2 & 0x80) >> 7;
  unsigned char mod_format= (cfg2 & 0x70) >> 4;
  unsigned char manchester_en= (cfg2 & 0x08) >> 3;
  unsigned char sync_mode= (cfg2 & 0x07);
  printf("MDMCFG2\n");
  printf("  DEM_DCFILT_OFF: %s\n", dem_dcfilt_off?"disable":"enable");
  printf("  MOD_FORMAT    : %s\n", mod_format_table[mod_format]);
  printf("  MANCHESTER_EN : %s\n", manchester_en?"enable":"disable");
  printf("  SYNC_MODE     : %s\n", sync_mode_table[sync_mode]);
}

// ------------------------------------------------------------------
/**
 *
 */
void cc2500_show_mdmcfg1_0(unsigned char cfg1, unsigned char cfg0)
{
  static unsigned char num_preamble_table[8]=
    { 2, 3, 4, 6, 8, 12, 16, 24 };
  unsigned char fec_en= (cfg1 & 0x80) >> 7;
  unsigned char num_preamble= (cfg1 & 0x70) >> 4;
  unsigned char chanspc_e= cfg1 & 0x03;
  unsigned char chanspc_m= cfg0;
  printf("MDMCFG1\n");
  printf("  FEC_EN      : %s\n", fec_en?"enable":"disable");
  printf("  NUM_PREAMBLE: %u\n", num_preamble_table[num_preamble]);
  printf("  CHANSPC_E   : %u\n", chanspc_e);
  printf("MDMCFG0\n");
  printf("  CHANSPC_M: %u\n", chanspc_m);
  unsigned long delta_f_channel= FXOSC;
  delta_f_channel*= (256+chanspc_m)*(1 << chanspc_e);
  delta_f_channel/= (1 << 18);
  printf("    -> Delta f channel = %lu Hz\n", delta_f_channel);
}

// ------------------------------------------------------------------
/**
 * \retval
 *  \li < 0 in case of failure
 *  \li >= 0 in case of success
 */
int demo_cc2500(BP * bp, int argc, char * argv[])
{
  unsigned char version, config;

  printf("CC2500 SPI demo ***\n");

  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) != BP_SUCCESS)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  printf("Setting SPI speed to 1MHz\n");
  if (bp_bin_spi_set_speed(bp, BP_BIN_SPI_SPEED_1M) != BP_SUCCESS)
    return -1;

  printf("Setting SPI config (open-drain,clk-idle=LOW,clk-edge=i->a,smp=middle)\n");
  if (bp_bin_spi_set_config(bp, BP_BIN_SPI_LV_HIZ |
			    BP_BIN_SPI_CLK_IDLE_LOW |
			    BP_BIN_SPI_CLK_EDGE_LOW |
			    BP_BIN_SPI_SMP_MIDDLE) != BP_SUCCESS)
    return -1;

  printf("Reading SPI config\n");
  /*if (bp_bin_spi_get_config(bp, &config) != BP_SUCCESS)
    return -1;*/
  //printf("Binary I/O SPI config (%u):\n", config);
  //show_spi_config(config);

  usleep(1000);

  printf("Setting SPI periph config (power=ON,pullups=ON)\n");
  if (bp_bin_spi_set_periph(bp, 0x08 | 0x04) != BP_SUCCESS)
    return -1;

  usleep(1000000);

  /*printf("Reading SPI periph config\n");
  if (bp_bin_spi_get_periph(bp, &config) != BP_SUCCESS)
    return -1;
    printf("  config=%u\n", config);*/

  /*unsigned long freq;
  if (cc2500_get_freq(bp, &freq) < 0)
    return -1;
    printf("Frequency: %lu\n", freq);*/

  unsigned char channel;
  if (cc2500_get_channel(bp, &channel) != CC2500_SUCCESS)
    return -1;
  printf("Channel: %u\n", channel);

  unsigned char cc2500_version, cc2500_partnum;
  if (cc2500_get_version(bp, &cc2500_version, &cc2500_partnum)
      != CC2500_SUCCESS)
    return -1;
  if ((cc2500_version != 0x03) || (cc2500_partnum != 0x80)) {
    printf("Error: unexpected CC2500 version/partnum\n");
    return -1;
  }
  printf("Version: %u\n", cc2500_version);
  printf("Partnum: %u\n", cc2500_partnum);

  unsigned char marcstate;
  if (cc2500_get_marcstate(bp, &marcstate) != CC2500_SUCCESS)
    return -1;
  cc2500_show_marcstate(marcstate);

  unsigned char status;
  if (cc2500_strobe(bp, CC2500_SRX, &status) != CC2500_SUCCESS)
    return -1;
  cc2500_show_status(status);

  if (cc2500_get_marcstate(bp, &marcstate) != CC2500_SUCCESS)
    return -1;
  cc2500_show_marcstate(marcstate);

  unsigned char mdmcfg4, mdmcfg3;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG4, &mdmcfg4)
      != CC2500_SUCCESS)
    return -1;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG3, &mdmcfg3)
      != CC2500_SUCCESS)
    return -1;
  cc2500_show_mdmcfg4_3(mdmcfg4, mdmcfg3);
  unsigned char value;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG2, &value)
      != CC2500_SUCCESS)
    return -1;
  cc2500_show_mdmcfg2(value);
  unsigned char mdmcfg1, mdmcfg0;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG1, &mdmcfg1)
      != CC2500_SUCCESS)
    return -1;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG0, &mdmcfg0)
      != CC2500_SUCCESS)
    return -1;
  cc2500_show_mdmcfg1_0(mdmcfg1, mdmcfg0);

  return 0;
}
