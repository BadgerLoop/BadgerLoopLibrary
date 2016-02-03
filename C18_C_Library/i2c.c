#include <i2c.h>
#include <stdio.h>
#include <usart.h>
#include <delays.h>

#if defined(__18CXX)
    #include <p18cxxx.h>   /* C18 General Include File */
#endif

unsigned int ACKStatus(void)
{
  return (!SSPCON2bits.ACKSTAT);                //Return Ack Status
}

unsigned int LDByteReadI2C(unsigned char ControlByte, unsigned char Address, unsigned char *Data, unsigned char Length)
 {
     unsigned int error = 0;
     IdleI2C();  
     StartI2C();                                 //Generate Start Condition
     WriteI2C(ControlByte | 0x00);               //Write Control Byte
     IdleI2C();                                  //wait for bus Idle
     WriteI2C(Address);                          //Write start address
     IdleI2C();                                  //wait for bus Idle
     RestartI2C();                               //Generate restart condition
     WriteI2C(ControlByte | 0x01);               //Write control byte for read
     IdleI2C();                                  //wait for bus Idle
     error = getsI2C(Data, Length);              //read Length number of bytes
     NotAckI2C();                                //Send Not Ack
     StopI2C();                                  //Generate Stop
     return error;
 }

 unsigned int LDByteWriteI2C(unsigned char ControlByte, unsigned char LowAdd, unsigned char data)
 {
     unsigned int ErrorCode1;
     unsigned int ErrorCode2;
     IdleI2C();                               //Ensure Module is Idle
     StartI2C();                              //Generate Start COndition
     WriteI2C(ControlByte);                   //Write Control byte
     IdleI2C();
     ErrorCode1 = ACKStatus();                //Return ACK Status
     WriteI2C(LowAdd);                        //Write Low Address
     IdleI2C();
     ErrorCode2 = ACKStatus();                //Return ACK Status
     WriteI2C(data);                          //Write Data
     IdleI2C();
     StopI2C();                               //Initiate Stop Condition
     //EEAckPolling(ControlByte);             //Perform ACK polling
 //    if(ErrorCode1 == 0) { printf("ACK 1 not recieved"); }
 //    if(ErrorCode2 == 0) { printf("ACK 2 not recieved"); }
     return(ErrorCode1);
 } 
/* unsigned char LDByteReadI2C( unsigned char ControlByte, unsigned char address, unsigned char *data, unsigned char length )
 {
   IdleI2C();                      // ensure module is idle
   StartI2C();                     // initiate START condition
   while ( SSPCON2bits.SEN );      // wait until start condition is over 
   WriteI2C( ControlByte );        // write 1 byte 
   IdleI2C();                      // ensure module is idle
   WriteI2C( address );            // WRITE word address to EEPROM
   IdleI2C();                      // ensure module is idle
   Delay10TCYx(100);
   RestartI2C();                   // generate I2C bus restart condition
   while ( SSPCON2bits.RSEN );     // wait until re-start condition is over 
   WriteI2C( ControlByte | 0x01 ); // WRITE 1 byte - R/W bit should be 1 for read
   Delay10TCYx(20);
   IdleI2C();                      // ensure module is idle
   getsI2C( data, length );       // read in multiple bytes
   ReadI2C();
   NotAckI2C();                    // send not ACK condition
   while ( SSPCON2bits.ACKEN );    // wait until ACK sequence is over 
   StopI2C();                      // send STOP condition
   while ( SSPCON2bits.PEN );      // wait until stop condition is over 
   return ( 0 );                   // return with no error
 }

unsigned char LDByteWriteI2C( unsigned char ControlByte, unsigned char LowAdd, unsigned char data )
{
  IdleI2C();                          // ensure module is idle
  StartI2C();                         // initiate START condition
  while ( SSPCON2bits.SEN );          // wait until start condition is over 
  WriteI2C( ControlByte | 0x00);            // write 1 byte - R/W bit should be 0
  IdleI2C();                          // ensure module is idle
  WriteI2C( LowAdd );                 // write address byte to EEPROM
  IdleI2C();                          // ensure module is idle
  WriteI2C ( data );                  // Write data byte to EEPROM
  IdleI2C();                          // ensure module is idle
  StopI2C();                          // send STOP condition
  while ( SSPCON2bits.PEN );          // wait until stop condition is over 
  while (EEAckPolling(ControlByte));  //Wait for write cycle to complete
  return (unsigned char)0;                       // return with no error
} */