// ==================================================================
// @(#)demo.c
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

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <buspirate.h>
#include <spi.h>

#include <24c02.h>
#include <24fc1025.h>
#include <cc2500.h>
#include <ds1324.h>
#include <enc28j60.h>
#include <ht1632.h>
#include <pcd8544.h>

// ------------------------------------------------------------------
/**
 * Signal handler that catches Ctrl-C and resets the 'loop'
 * variable to 0. This variable can be used by demos that need to run
 * until the user sends a break.
 */
volatile int loop= 1;
static void signal_handler(int signum) {
  fprintf(stdout, "\nCtrl-C received\n");
  fflush(stdout);
  loop= 0;
}

// ------------------------------------------------------------------
/**
 *
 */
void show_pins_state(unsigned char value, int power, int pullup)
{
  printf("  Pin CS  : %s\n", (value & BP_PIN_CS)?"ON":"OFF");
  printf("  Pin MISO: %s\n", (value & BP_PIN_MISO)?"ON":"OFF");
  printf("  Pin CLK : %s\n", (value & BP_PIN_CLK)?"ON":"OFF");
  printf("  Pin MOSI: %s\n", (value & BP_PIN_MOSI)?"ON":"OFF");
  printf("  Pin AUX : %s\n", (value & BP_PIN_AUX)?"ON":"OFF");
  if (pullup)
    printf("  Pullup  : %s\n", (value & BP_PIN_PULLUP)?"ON":"OFF");
  if (power)
    printf("  Power   : %s\n", (value & BP_PIN_POWER)?"ON":"OFF");
}

// ------------------------------------------------------------------
/**
 *
 */
void show_spi_config(unsigned char config)
{
  assert(config <= 15);
  printf("  Output level   : %s\n",
	 (config & BP_BIN_SPI_LV_3V3?"3V3":"HiZ"));
  printf("  Clock idle     : %s\n",
	 (config & BP_BIN_SPI_CLK_IDLE_HIGH?"high":"low"));
  printf("  Clock edge     : %s\n",
	 (config & BP_BIN_SPI_CLK_EDGE_HIGH?"idle->active":"active->idle"));
  printf("  Sample position: %s\n",
	 (config & BP_BIN_SPI_SMP_END?"end":"middle"));
}

// ------------------------------------------------------------------
/**
 *
 */
void show_data(const char * msg, const unsigned char * data,
	       unsigned char nbytes)
{
  int i;
  printf("%s", msg);
  for (i= 0; i < nbytes; i++)
    printf(" 0x%.2X", data[i]);
  printf("\n");
}

// ------------------------------------------------------------------
/**
 * This demo setups the CS, MISO, CLK, MOSI and AUX pins as outputs
 * and continuously changes their output state.
 *
 * You can connect LEDs to those pins for example and observe some
 * "chenillard" pattern.
 */
int demo_bitbang(BP * bp, int argc, char * argv[])
{
  // 0=CS, 1=MISO, 2=CLK, 3=MOSI, 4=AUX
#define SET_PIN(PIN, VALUE) \
  bp_bin_pins_set(bp, 0, 0, (PIN)==4?(VALUE):0,		    \
		  (PIN)==3?(VALUE):0, (PIN)==2?(VALUE):0,   \
		  (PIN)==1?(VALUE):0, (PIN)==0?(VALUE):0,   \
		  &value)
#define ON(PIN, VALUE) \
  ((PIN)==(VALUE)?"ON ":"OFF")

  unsigned char value;
  int i, j;

  printf("*** bitbang demo ***\n");
  printf("This demo changes the state of the CS/MISO/CLK/MISO/AUX pins.\n");

  printf("Setup pins as output\n");
  if (bp_bin_pins_setup(bp, 0, 0, 0, 0, 0, &value) != BP_SUCCESS)
    return -1;
  //show_pins_state(value, 0, 0);

  printf("Change state of pins...\n");
  printf("CS   | MISO | CLK  | MOSI | AUX\n");
  printf("--------------------------------\n");
  while (loop) {
    for (i= 0; (i < 5) && loop; i++) {
      for (j= 0; (j < 5) && loop; j++) {
	printf("\r%s  | %s  | %s  | %s  | %s ",
	       ON(0,j), ON(1,j), ON(2,j), ON(3,j), ON(4,j));
	fflush(stdout);
	if (SET_PIN(j, 1) != BP_SUCCESS)
	  return -1;
	usleep(500000);
	if (SET_PIN(j, 0) != BP_SUCCESS)
	  return -1;
      }
    }
  }

  return 0;
}

// ------------------------------------------------------------------
/**
 * This demo continuously reads the ADC pin of the Bus Pirate.
 * It displays the obtained voltage in millivolts.
 *
 * The ADC pin is connected to the BP microcontroller through a
 * resistance network that divides the voltage by two. The
 * microcontroller's ADC uses a 3.3V reference voltage.
 */
int demo_adc(BP * bp, int argc, char * argv[])
{
  printf("*** ADC demo ***\n");
  printf("This demo reads the ADC input voltage.\n");

  int old_mV= -1, mV;
  int count= 0;
  int old_mean= -1, mean;

  while (loop) {
    if (bp_bin_read_voltage(bp, &mV) != BP_SUCCESS) {
      printf("Error: could not read voltage\n");
      loop= 0;
    }
    count++;
    mean= count;
    if ((mV != old_mV) || (mean != old_mean)) {
      old_mV= mV;
      old_mean= mean;
      printf("\r                   ");
      printf("\r%d mV  (%d reads)", mV, count);
      fflush(stdout);
    }
    usleep(1);
  }
  printf("\n");
  return 0;
}

// ------------------------------------------------------------------
/**
 * A simple SPI demo.
 *
 * Need to explain what it does.
 */
int demo_spi(BP * bp, int argc, char * argv[])
{
  unsigned char version;

  printf("*** SPI demo ***\n");
  printf("This demo sends and reads SPI data.\n");

  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) != BP_SUCCESS)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  unsigned char speed= BP_BIN_SPI_SPEED_30K;
  /* Note: SPI speed >= 2.6MHz do not work due to issues
     within the BP firmware (checked with version 5.10) */
  printf("Select SPI speed : %s\n", BP_BIN_SPI_SPEEDS[speed]);
  if (bp_bin_spi_set_speed(bp, speed) != BP_SUCCESS)
    return -1;

  unsigned char config=
    BP_BIN_SPI_LV_HIZ |
    BP_BIN_SPI_CLK_IDLE_LOW |
    BP_BIN_SPI_CLK_EDGE_HIGH |
    BP_BIN_SPI_SMP_MIDDLE;

  printf("Binary I/O SPI config (%u):\n", config);
  show_spi_config(config);

  printf("Setting SPI config\n");
  if (bp_bin_spi_set_config(bp, config) != BP_SUCCESS)
    return -1;

  printf("Configuring power and pull-ups.\n");
  if (bp_bin_spi_set_periph(bp, BP_BIN_SPI_PERIPH_POWER |
			    BP_BIN_SPI_PERIPH_PULLUPS) < 0)
    return -1;

  printf("Sending SPI data...\n");
  while (loop) {
    unsigned char data[2]= { 0x55, 0xAA };
    if (bp_bin_spi_cs(bp, 1) < 0)
      return -1;
    if (bp_bin_spi_bulk(bp, data, sizeof(data)) != BP_SUCCESS)
      return -1;
    if (bp_bin_spi_cs(bp, 0) < 0)
      return -1;
    usleep(100000);
  }

  return 0;
}

// ------------------------------------------------------------------
/**
 * This is an example empty demo.
 * It just waits for user termination with Ctrl-C.
 */
int xp(BP * bp, int argc, char * argv[])
{
  int i= 0;
  while (loop) {
    printf("\rWaiting for termination... %c", "|/-\\|/-\\"[(i++) % 8]);
    usleep(1);
  }

  return 0;
}

// ------------------------------------------------------------------
/**
 * Definition of demo functions.
 * A function that returns < 0 in case of error, >= 0 otherwise.
 */
typedef int (* demo_f)(BP * bp, int argc, char * argv[]);

typedef struct demo_t {
  char * name;
  demo_f fct;
} demo_t;

// ------------------------------------------------------------------
/**
 * All demos must be registered here
 */
demo_t demos[]=
  {
    { "version",           NULL },
    //    { "xp",                xp },
    { "bitbang",           demo_bitbang },
    { "adc",               demo_adc },
    { "spi",               demo_spi },
    { "spi-cc2500",        demo_cc2500},
    { "spi-enc28j60",      demo_spi_enc28j60},
    { "i2c-ds1624",        ds1624_demo},
    { "i2c-24c02",         _24c02_demo},
    { "i2c-24fc1025",      _24fc1025_demo},
    { "spi-pcd8544",       demo_pcd8544},
    { "raw-ht1632",        demo_ht1632},
  };
#define NUM_DEMOS sizeof(demos)/sizeof(demo_t)

// ------------------------------------------------------------------
/**
 * Display program usage.
 *
 * Optionally displays an error message (if 'msg' is not NULL). In
 * this case, the program is terminated with an error status.
 */
void help(const char * msg, ...)
{
  va_list ap;
  int i;

  if (msg != NULL) {
    va_start(ap, msg);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, msg, ap);
    va_end(ap);
  }

  printf("\n\n");
  printf("usage: buspirate-demo DEVICE DEMO_ID\n");
  printf("\n");
  printf("with DEMO_ID in\n");
  for (i= 0; i < NUM_DEMOS; i++)
    printf("  %.2u  %s\n", i+1, demos[i].name);

  printf("\n");
  printf("See src/dev/README file for instructions on how to wire the Bus Pirate\n");
  printf("for a specific demo.\n");

  if (msg != NULL)
    exit(EXIT_FAILURE);
}

// ------------------------------------------------------------------
/**
 * Display program name / copyright information.
 */
void welcome()
{
  printf("Bus Pirate library demonstration\n");
  printf("(C) 2010-2014, Bruno Quoitin\n");
}

// ------------------------------------------------------------------
/**
 * Main program
 */
int main(int argc, char * argv[])
{
  BP * bp;
  unsigned char version;
  int demonum= 0;

  welcome();

  if (argc < 3)
    help("not enough arguments");

  demonum= atoi(argv[2]);
  if ((demonum < 1) || (demonum > NUM_DEMOS+1))
    help("invalid demo-id");

  bp= bp_open(argv[1]);
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

  if (demos[demonum-1].fct != NULL)
    if (demos[demonum-1].fct(bp, argc, argv) < 0)
      fprintf(stderr, "Error: demo returned with error.\n");

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
