/*	These are all the includes you should need
 *	all you'll need to do is import this C file and you'll be good ;)
 */

//#include <stdint.h>        /* For uint8_t definition */
//#include <stdbool.h>       /* For true/false definition */
//#include <stdint.h>
#include <stdio.h>
#include <usart.h>
#include <adc.h>
#include <i2c.h>
#include "MPU6050.c" /* MPU6050 drivers */
#include "vl6180.c"
#include "i2c.c"
//#include "MS5803-01BA.c" /* MS5803-01BA drivers */
//#include "I2Cdev.c" /* I2Cdev library */
//#include "SFE_BMP180.c"
//#include "vl6180-driver.c"
//#include "mpu9250.c" /* MPU6050 drivers */


//Delay function for y'all
void delayzz(void) {
	int i, j;
	for(i=0;i<5000;i++)
	{
		for(j=0;j<2;j++) 
		{
			/* Well its Just a Timer */
		}    
	}   
}

//Function to Initialise the ADC Module
void ADCInit(void)
{
   OpenADC(ADC_FOSC_2 & ADC_RIGHT_JUST & ADC_8ANA_0REF, ADC_CH0 & ADC_INT_ON);
   //OpenADC(ADC_FOSC_RC & ADC_RIGHT_JUST & ADC_1ANA_0REF, ADC_CH0 & ADC_INT_OFF);
}

/* uint16_t convertFrom8To16(uint8_t dataFirst, uint8_t dataSecond) {
    uint16_t dataBoth = 0x0000;

    dataBoth = dataFirst;
    dataBoth = dataBoth << 8;
    dataBoth |= dataSecond;
    return dataBoth;
}

uint8_t *convertFrom16To8(uint16_t dataAll) {
    static uint8_t arrayData[2] = { 0x00, 0x00 };

    *(arrayData) = (dataAll >> 8) & 0x00FF;
    arrayData[1] = dataAll & 0x00FF;
    return arrayData;
} */

//#define MCUCLK 80000000 // 80MHz | HSPLL == 20MHz * 4
#define MCUCLK 20000000 // 20MHz | HS == 20MHz
#define I2C_SPEED	400000 // 400kHz
#define I2C_SSPADD	( ( MCUCLK / ( 4 * I2C_SPEED ) ) - 1 )

void initI2C_USART(void) {  
    //For USART
    TRISCbits.RC6 = 0; //TX pin set as output
    TRISCbits.RC7 = 1; //RX pin set as input

    //Open USART (needed for I2C to function)
    // Serial (115200 kbps)
    /* OpenUSART(USART_TX_INT_OFF | USART_RX_INT_OFF | USART_ASYNCH_MODE |
        USART_EIGHT_BIT | USART_CONT_RX | USART_BRGH_HIGH, 520); //was 12 */
    OpenUSART (USART_TX_INT_OFF &
             USART_RX_INT_OFF &
             USART_ASYNCH_MODE &
             USART_EIGHT_BIT &
             USART_CONT_RX &
             USART_BRGH_HIGH, 129); //Baud Rate = 19.2 kbps => 64 | Baud Rate = 9.6 kbps => 129
    
    // I2C (400 kHz)
    OpenI2C(MASTER, SLEW_OFF);
    SSPADD = I2C_SSPADD;

    //Slight delay
    //Delay10KTCYx(50);
}

void initMPU6050(void) {
	// MPU6050
    //MPU6050(MPU6050_ADDRESS_AD0_LOW);
    // Initialize MPU6050
    //MPU6050_initialize();

    //Slight delay
    //Delay10KTCYx(50);
}
