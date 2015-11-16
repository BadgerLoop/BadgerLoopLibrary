#include <spi.h>
#include <stdio.h>
#include <unistd.h>

#include "pcd8544.h"

int _pcd8544_cmd(BP *bp, unsigned char cmd)
{
  if (bp_bin_spi_set_periph(bp, (BP_BIN_SPI_PERIPH_POWER |
				 BP_BIN_SPI_PERIPH_PULLUPS)) < 0)
    return -1;
  if (bp_bin_spi_bulk(bp, &cmd, 1) < 0)
    return -1;
  return 0;
}

int _pcd8544_data(BP *bp, unsigned char data)
{
  if (bp_bin_spi_set_periph(bp, (BP_BIN_SPI_PERIPH_POWER |
				 BP_BIN_SPI_PERIPH_PULLUPS |
				 BP_BIN_SPI_PERIPH_AUX)) < 0)
    return -1;
  if (bp_bin_spi_bulk(bp, &data, 1) < 0)
    return -1;
  return 0;
}

int demo_pcd8544(BP * bp, int argc, char * argv[])
{
  unsigned char version;
  int i;

  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) != BP_SUCCESS)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);
  
  printf("Set SPI configuration\n");
  unsigned char config= (BP_BIN_SPI_LV_HIZ |
			 BP_BIN_SPI_CLK_IDLE_LOW |
			 BP_BIN_SPI_CLK_EDGE_LOW |
			 BP_BIN_SPI_SMP_MIDDLE);
  if (bp_bin_spi_set_config(bp, config) != BP_SUCCESS)
    return -1;

  printf("Set SPI speed\n");
  if (bp_bin_spi_set_speed(bp, BP_BIN_SPI_SPEED_1M) != BP_SUCCESS)
    return -1;

  printf("Configuring power and pull-ups.\n");
  if (bp_bin_spi_set_periph(bp, (BP_BIN_SPI_PERIPH_POWER |
				 BP_BIN_SPI_PERIPH_PULLUPS)) < 0)
    return -1;
  
  usleep(100000);

  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;

  /* Extended instruction mode (bit H=1) */
  _pcd8544_cmd(bp, 0x21);
  _pcd8544_cmd(bp, 0xC8); /* Set VOP */
  _pcd8544_cmd(bp, 0x06); /* Set Temperature Coefficient */
  _pcd8544_cmd(bp, 0x13); /* Set Bias System (1:48) */

  /* Normal instruction mode (bit H=0) */
  _pcd8544_cmd(bp, 0x20);
  /* Display control set normal mode */
  _pcd8544_cmd(bp, 0x0C);

  /* Data write (clear screen) */
  for (i= 0; i < 48*84/8; i++)
    _pcd8544_data(bp, 0x00);
  /* Data write (line pattern) */
  for (i= 0; i < 48*84/8; i++)
    _pcd8544_data(bp, 0x55);

  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;

  usleep(2000000);


  return 0;
}
