/*	These are all the includes you should need
 *	all you'll need to do is import this C file and you'll be good ;)
 */

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <plib/usart.h>

#include "I2Cdev.c" /* I2Cdev library */
#include "MPU6050.c" /* MPU6050 drivers */
#include "MS5803-01BA.c" /* MS5803-01BA drivers */

/******************************************************************************/
/*	These are the configuration bits for the PIC18F458
 *	Don't mess with these without consulting Nick Jaunich first!
 */
/******************************************************************************/
#pragma config OSC = HSPLL         // Oscillator Selection bits (HS oscillator + PLL = 20mHz*4 = 80mHz)
#pragma config OSCS = OFF       // Oscillator System Clock Switch Enable bit (Oscillator system clock switch option is disabled (main oscillator is source))

// CONFIG2L
#pragma config PWRT = ON       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOR = ON         // Brown-out Reset Enable bit (Brown-out Reset enabled)
#pragma config BORV = 25        // Brown-out Reset Voltage bits (VBOR set to 2.5V)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 128      // Watchdog Timer Postscale Select bits (1:128)

// CONFIG4L
#pragma config STVR = OFF        // Stack Full/Underflow Reset Enable bit (Stack Full/Underflow will cause Reset)
#pragma config LVP = OFF         // Low-Voltage ICSP Enable bit (Low-Voltage ICSP enabled)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000200-001FFFh) not code protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot Block (000000-0001FFh) not code protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000200-001FFFh) not write protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0001FFh) not write protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000200-001FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from Table Reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0001FFh) not protected from Table Reads executed in other blocks)
/******************************************************************************/

/*
 *	Module ifdefs are listed below
 *		
 *		WirelessCommunicationModule = WCM
 *			CAN ID = 2
 *  	VehicleControlModule = VCM
 *			CAN ID = 3
 *		PressureControlModule = PCM
 *			CAN ID = 4
 *		EnvironmentalControlModule = ECM
 *			CAN ID = 5
 *		BrakingPropulsionModule = BPM
 *			CAN ID = 6
 */

#define CAN_TRANSMIT    (1)

#define CAN_RX  (1<<3)
#define CAN_TX  (1<<2)

#define CAN_MODE_NORMAL     (0x00)
#define CAN_MODE_DISABLE    (0x20)
#define CAN_MODE_LOOPBACK   (0x40)
#define CAN_MODE_LISTEN     (0x60)
#define CAN_MODE_CONFIG     (0x80)

/* #define WCM */
#ifdef ALL
    #define CAN_IDENTIFIER  (1)
#endif

#ifdef WCM
	#define CAN_IDENTIFIER  (2)
 	//more
#endif

#ifdef VCM
	#define CAN_IDENTIFIER  (3)
 	//more
#endif

#ifdef PCM
	#define CAN_IDENTIFIER  (4)
 	//more
#endif

#ifdef ECM
	#define CAN_IDENTIFIER  (5)
 	//more
#endif

#ifdef BPM
	#define CAN_IDENTIFIER  (6)
 	//more
#endif

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

void CAN_SetMode(uint8_t canMode)
{
	// put CAN in normal mode
    CANCON = canMode;

    // wait till mode changes
    while((CANSTAT & 0xE0) != canMode);
}

void CAN_Init(void)
{
    TRISB = (1<<3);

    // put CAN in configure mode
    CAN_SetMode(CAN_MODE_CONFIG);

    //  Configuration generated by mbtime.exe
    //  Baud = 100kbps
    //  1Tq = 400nsec
    //  Propagation = 1Tq
    //  Phase 1 = 3Tq
    //  Phase 2 = 3Tq
    //  SJW = 1Tq
    
    //BRGCON1 = 0x0F;
        BRGCON1 = 0x43; //SJW = 2 Tq, BRP = 3
    //BRGCON2 = 0xBF
        BRGCON2 = 0xA1; // Seg2 freely programmable,
                        // 1 samples/bit, Ph1 = 5 Tq, Prop seg = 2 Tq
    //BRGCON3 = 0x07;
        //BRGCON3 = 0x01; // Seg2 = 2 Tq  ---> Sample at 80%
        BRGCON2 = 0x01;
    // configure CAN reception,
    // accept all message
    RXM0SIDH = 0; RXM0SIDL = 0; //Set buffer 0 mask
    RXM1SIDH = 0; RXM1SIDL = 0; //Set buffer 1 mask
    RXF0SIDL = 0;
    RXF1SIDL = 0;
    RXF2SIDL = 0;
    RXF3SIDL = 0;
    RXF4SIDL = 0;
    RXF5SIDL = 0;

    RXB0CON = 0;
    RXB1CON = 0;

    // drive CAN_TX to VDD
    //CIOCON = 0x20;

    //CANCON |= (1<<4);   // abort pending TX
    
    // put CAN in normal mode
    CAN_SetMode(CAN_MODE_NORMAL);
}

void setupCANTxRx(void) {
    //Make sure CANRX/TX is functioning properly
    //Set RB2 (CANTX) to output
    TRISBbits.TRISB2 = 0; 
    //Set RB3 (CANRX) to input
    TRISBbits.TRISB3 = 1;  
    
    //Run CAN_Init function
    CAN_Init();
    
    //Slight delay
	Delay10KTCYx(50);
} 

#define MCUCLK 80000000 // 80MHz | HSPLL == 20MHz * 4
#define I2C_SPEED	400000 // 400kHz
#define I2C_SSPADD	( ( MCUCLK / ( 4 * I2C_SPEED ) ) - 1 )

void initI2C(void) {    
    //Open USART (needed for I2C to function)
    // Serial (115200 kbps)
    OpenUSART(USART_TX_INT_OFF | USART_RX_INT_OFF | USART_ASYNCH_MODE |
        USART_EIGHT_BIT | USART_CONT_RX | USART_BRGH_HIGH, 12); 
    
    // I2C (400 kHz)
    OpenI2C(MASTER, SLEW_OFF);
    SSPADD = I2C_SSPADD;

    //Slight delay
    Delay10KTCYx(50);
}

void initMPU6050(void) {
	// MPU6050
    MPU6050(MPU6050_ADDRESS_AD0_LOW);
    // Initialize MPU6050
    MPU6050_initialize();

    //Slight delay
    Delay10KTCYx(50);
}
