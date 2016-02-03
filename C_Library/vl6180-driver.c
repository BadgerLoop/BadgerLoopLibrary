
/*******************************************************************************
Copyright � 2014, STMicroelectronics International N.V.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of STMicroelectronics nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED. 
IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************/
/*
 * $Date: 2015-07-06 15:44:31 +0200 (Mon, 06 Jul 2015) $
 * $Revision: 2430 $
 */

/**
 * @file vl6180x_i2c.c
 *
 * Copyright (C) 2014 ST MicroElectronics
 *
 * provide variable word size byte/Word/dword VL6180x register access via i2c
 *
 */
#include "I2Cdev.c"
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <I2Cdev.h>



int VL6180x_WrByte(VL6180xDev_t dev, uint16_t index, uint8_t data)
{
	
    bool returnValH = I2Cdev_writeByte2(0x29, index, 1, &data);
   
    if (returnVal)
    {
        return 1;

    } else {
	
        return 0;
    }

    return 0;
}

int VL6180x_WrWord(VL6180xDev_t dev, uint16_t index, uint16_t data){
	
	//--> uint8_t length: processor word size is?
    bool returnVal = I2Cdev_writeByte2(0x29, index, uint8_t length, &data);
   
    if (returnVal)
    {
        return 1;

    } else {
	
        return 0;
    }

    return 0;
}

int VL6180x_WrDWord(VL6180xDev_t dev, uint16_t index, uint32_t data){
	
	//length: processor word size is?
    bool returnVal = I2Cdev_writeDWord(0x29, regAddr, uint8_t length, &data);
   
    if (returnVal)
    {
        return 1;

    } else {
	
        return 0;
    }

    return 0;
}

/* 
 	int VL6180x_UpdateByte(VL6180xDev_t dev, uint16_t index, uint8_t AndData, uint8_t OrData){
    VL6180x_I2C_USER_VAR
    int  status;
    uint8_t *buffer;
    DECL_I2C_BUFFER

    VL6180x_GetI2CAccess(dev);

    buffer=VL6180x_GetI2cBuffer(dev,3);
    buffer[0]=index>>8;
    buffer[1]=index&0xFF;

    status=VL6180x_I2CWrite(dev, (uint8_t *)buffer,(uint8_t)2);
    if( !status ){
        //read data direct onto buffer
        status=VL6180x_I2CRead(dev, &buffer[2],1);
        if( !status ){
            buffer[2]=(buffer[2]&AndData)|OrData;
            status=VL6180x_I2CWrite(dev, buffer, (uint8_t)3);
        }
    }

    VL6180x_DoneI2CAcces(dev);

    return status;
} 
 */

int VL6180x_RdByte(VL6180xDev_t dev, uint16_t index, uint8_t *data){
    //length: processor word size is?
    bool returnVal = I2Cdev_readByte2(0x29, index, uint8_t length, &data);
   
    if (returnVal)
    {
        return 1;

    } else {
	
        return 0;
    }

    return 0;
}

int VL6180x_RdWord(VL6180xDev_t dev, uint16_t index, uint16_t *data){
	//length: processor word size is?
	    bool returnVal = I2Cdev_readWord2(0x29, index, uint8_t length, &data);
	   
	    if (returnVal)
	    {
	        return 1;

	    } else {
		
	        return 0;
	    }

	    return 0;
	
}

/* int  VL6180x_RdDWord(VL6180xDev_t dev, uint16_t index, uint32_t *data){
    VL6180x_I2C_USER_VAR
    int status;
    uint8_t *buffer;
    DECL_I2C_BUFFER

    VL6180x_GetI2CAccess(dev);
    buffer=VL6180x_GetI2cBuffer(dev,4);

    buffer[0]=index>>8;
    buffer[1]=index&0xFF;

    status=VL6180x_I2CWrite(dev, (uint8_t *) buffer, (uint8_t)2);
    if( !status ){
        status=VL6180x_I2CRead(dev, buffer,4);
        if( !status ){
            // VL6180x register are Big endian if cpu is be direct read direct into data is possible
            *data=((uint32_t)buffer[0]<<24)|((uint32_t)buffer[1]<<16)|((uint32_t)buffer[2]<<8)|((uint32_t)buffer[3]);
        }
    }
    VL6180x_DoneI2CAcces(dev);
    return status;
} */

