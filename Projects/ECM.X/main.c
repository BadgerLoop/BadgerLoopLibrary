/*
 * File:   main.c
 * Author: njaunich
 *
 * Created on October 30, 2015, 1:14 PM
 */

#define _XTAL_FREQ 20000000
#define ALL /* CAN ID = 1 */

#include "pic18f458_badgerloop.c"

volatile uint8_t receivedData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    //receivedData[0-7] = data
volatile uint8_t sendData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    //sendData[0-7] = data

void CAN_Transmit(void)
{
    uint16_t canId = CAN_IDENTIFIER;
    canId = (canId<<5);

    //  wait till transmission
    while(TXB0CON & (1<<3));
    //  load CAN ID
    TXB0SIDL = (uint8_t)(canId & 0x00E0); // standard 11bit Identifier
    TXB0SIDH = (uint8_t)((canId>>8)& 0x00FF);

    //  Data length = 1byte == 8 bits or a unsigned 8 bit integer (uint8_t) ;)
    //TXB0DLC = 0x01;
    TXB0DLC = 0x08;

    //  load data
    TXB0D0 = sendData[0];
    TXB0D1 = sendData[1];
    TXB0D2 = sendData[2];
    TXB0D3 = sendData[3];
    TXB0D4 = sendData[4];
    TXB0D5 = sendData[5];
    TXB0D6 = sendData[6];
    TXB0D7 = sendData[7];

    //  start transmission
    TXB0CON = (1<<3);

}

uint16_t CAN_Receive(void)
{
    //receivedData[0-7] = data

    uint8_t canIdH=0;
    uint8_t canIdL=0;
    uint16_t canId=0;

    //  wait till reception
    while(!(RXB0CON & (1<<7)));

    // copy CAN ID
    canIdL = RXB0SIDL;
    canIdH = RXB0SIDH;

    //  copy data
    receivedData[0] = RXB0D0;
    receivedData[1] = RXB0D1;
    receivedData[2] = RXB0D2;
    receivedData[3] = RXB0D3;
    receivedData[4] = RXB0D4;
    receivedData[5] = RXB0D5;
    receivedData[6] = RXB0D6;
    receivedData[7] = RXB0D7;
    
    // clear receive flag
    RXB0CON &= ~(1<<7);

    canId = canIdH<<3;
    canId |= canIdL>>5;
	
    return canId;
}

#define LED_PIN LATBbits.LATB0

void flashLED(void) {
    int count;
    count = 10;
    while(1) {
        LED_PIN = 1;   // RB-4 to High  
        delayzz();
        LED_PIN = 0;    // RB-4 to LOW
        delayzz();
        count--;
        if (count == 0)
        {
            break;
        }
    }
}

union doubleAndChar {
  double d[1];
  char b[sizeof(double)];
};

void main(void) {
    //int16_t ax, ay, az, gx, gy, gz; //MPU6050 values
    //uint8_t a_unsigned_array[2];
    initI2C_USART();
    setupCANTxRx();
    LED_PIN = 1;
    
    //Set RB0 to output
    TRISBbits.TRISB0 = 0;
    flashLED();
    if (BMP180_init() != 1) {
        LED_PIN = 1;
    }
    while(true)
    {
        union doubleAndChar temp;
        sendData[0] = 1;
        sendData[1] = 1;
        if (BMP180_startTemperature() != 5)
        {
            LED_PIN = 1;
        }
        __delay_ms(5);
        
        if (BMP180_getTemperature((double)temp.d) == 1)
        {
            sendData[1] = (char)temp.b[0];
            //putsUSART((char)temp.b[0]);
            //putsUSART("Hello");
            //while(!PIR1bits.TXIF);

            //TXSTAbits.TX9D = 0;  // 9th bit, just in case
            //TXREG = 0x00;  // transmit 'U'
            //putcUSART('0');
            LED_PIN ^= 1;
            __delay_ms(25);
        } 
        /* sendData[0] = 1 //Sensor 1 -> MPU6050 (ax,ay)
         * sendData[1] = AFS_SEL
         * sendData[2] = ax_msbyte
         * sendData[3] = ax_lsbyte
         * sendData[4] = ay_msbyte
         * sendData[5] = ay_lsbyte
         * sendData[6] = az_msbyte
         * sendData[7] = az_lsbyte
         */
        /*sendData[0] = 1;
        sendData[1] = AFS_SEL;
        //Sending ax, ay, az over CAN
        //Convert ax for sending
        a_unsigned_array[0] = convertFrom16To8( (uint16_t) ax )[0];
        a_unsigned_array[1] = convertFrom16To8( (uint16_t) ax )[1];
        sendData[2] = a_unsigned_array[0];
        sendData[3] = a_unsigned_array[1];
        //Convert ay for sending
        a_unsigned_array[0] = convertFrom16To8( (uint16_t) ay )[0];
        a_unsigned_array[1] = convertFrom16To8( (uint16_t) ay )[1];
        sendData[4] = a_unsigned_array[0];
        sendData[5] = a_unsigned_array[1];
        
        //Convert ax for sending
        a_unsigned_array[0] = convertFrom16To8( (uint16_t) az )[0];
        a_unsigned_array[1] = convertFrom16To8( (uint16_t) az )[1];
        sendData[6] = a_unsigned_array[0];
        sendData[7] = a_unsigned_array[1]; */
        //Actually send message
        CAN_Transmit();
        
        __delay_ms(25);
    }
}
