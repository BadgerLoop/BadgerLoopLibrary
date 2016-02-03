/**
 * \project vl6180-driver
 * \brief   a generic driver providing the most basic functionality 
 *          for the vl6180 proximity sensor. In a nutshell, you 
 *          implement the two functions read(...) and write(...),
 *          on the microcontroller of your choice, and the rest is 
            done for you.
 * \author Joshua Vasquez
 * \date October 8, 2014
*/

/* unsigned int LDByteWriteI2C(unsigned char ControlByte, unsigned char LowAdd, unsigned char data);
unsigned int LDByteReadI2C(unsigned char ControlByte, unsigned char Address, unsigned char *Data, unsigned char Length);
unsigned int ACKStatus(void); */

float ALS_Gain_;
float ALS_IntegrationTime_;

/// Constants:
const float ALS_LuxResolution_ = 0.32;

/// Registers:
#define IDENTIFICATION_MODEL_ID 0x000
#define IDENTIFICATION_MODEL_REV_MAJOR 0x001
#define IDENTIFICATION_MODEL_REV_MINOR 0x002
#define IDENTIFICATION_MODULE_REV_MAJOR 0x003
#define IDENTIFICATION_MODULE_REV_MINOR 0x004
#define IDENTIFICATION_DATE_HI          0x006
#define IDENTIFICATION_DATE_LOW         0x007
#define IDENTIFICATION_TIME             0x008
#define SYSTEM_MODE_GPIO0               0x010
#define SYSTEM_MODE_GPIO1               0x011
#define SYSTEM_HISTORY_CTRL             0x012
#define SYSTEM_INTERRUPT_CONFIG_GPIO    0x014
#define SYSTEM_INTERRUPT_CLEAR          0x015
#define SYSTEM_FRESH_OUT_OF_RESET       0x016
#define SYSTEM_GROUPED_PARAMETER_HOLD   0x017
#define SYSRANGE_START                  0x018
#define SYSRANGE_THRESH_HIGH            0x019
#define SYSRANGE_THRESH_LOW             0x01A
#define SYSRANGE_INTERMEASUREMENT_PERIOD 0x01B
#define SYSRANGE_MAX_CONVERGENCE_TIME 0x01C
#define SYSRANGE_CROSSTALK_COMPENSATION_RATE 0x01E
#define SYSRANGE_CROSSTALK_VALID_HEIGHT 0x021
#define SYSRANGE_EARLY_CONVERGENCE_ESTIMATE 0x022
#define SYSRANGE_PART_TO_PART_RANGE_OFFSET 0x024
#define SYSRANGE_RANGE_IGNORE_VALID_HEIGHT 0x025
#define SYSRANGE_RANGE_IGNORE_THRESHOLD 0x026
#define SYSRANGE_MAX_AMBIENT_LEVEL_MULT 0x02C
#define SYSRANGE_RANGE_CHECK_ENABLES    0x02D
#define SYSRANGE_RECALIBRATE            0x02E
#define SYSRANGE_REPEAT_RATE            0x031
#define SYSALS_START                    0x038
#define SYSALS_THRESH_HIGH              0x03A
#define SYSALS_THERSH_LOW               0x03C
#define SYSALS_INTERMEASUREMENT_PERIOD  0x03E
#define SYSALS_ANALOGUE_GAIN            0x03F
#define SYSALS_INTEGRATION_PERIOD       0x040
#define RESULT_RANGE_STATUS             0x04D
#define RESULT_ALS_STATUS               0x04E
#define RESULT_INTERRUPT_STATUS_GPIO    0x04F
#define RESULT_ALS_VAL                  0x050
#define RESULT_HISTORY_BUFFER_0         0x052
#define RESULT_HISTORY_BUFFER_1         0x054
#define RESULT_HISTORY_BUFFER_2         0x056
#define RESULT_HISTORY_BUFFER_3         0x058
#define RESULT_HISTORY_BUFFER_4         0x05A
#define RESULT_HISTORY_BUFFER_5         0x05C
#define RESULT_HISTORY_BUFFER_6         0x05E
#define RESULT_HISTORY_BUFFER_7         0x060
#define RESULT_RANGE_VAL                0x062
#define RESULT_RANGE_RAW                0x064
#define RESULT_RANGE_RETURN_RATE        0x066
#define RESULT_RANGE_REFERENCE_RATE     0x068
#define RESULT_RANGE_RETURN_SIGNAL_COUNT 0x06C
#define RESULT_RANGE_REFERENCE_SIGNAL_COUNT 0x070
#define RESULT_RANGE_RETURN_AMB_COUNT 0x074
#define RESULT_RANGE_REFERENCE_AMB_COUNT 0x078
#define RESULT_RANGE_RETURN_CONV_TIME 0x07C
#define RESULT_RANGE_REFERENCE_CONV_TIME 0x080
#define READOUT_AVERAGING_SAMPLE_PERIOD 0x10A
#define FIRMWARE_BOOTUP 0x119
#define FIRMWARE_RESULT_SCALER 0x120
#define I2C_SLAVE_DEVICE_ADDRESS 0x212
#define INTERLEAVED_MODE_ENABLE 0x2A3

bool vl6180_clearModes(void)
{
    //writeOne(I2C, SYSALS_START, 0x00);
    //writeOne(I2C, SYSRANGE_START, 0x00);
    //writeOne(I2C, INTERLEAVED_MODE_ENABLE, 0);  /// Disable interleaved mode.
    
    //LDByteWriteI2C(0x29, SYSALS_START, 0x00);
    //LDByteWriteI2C(0x29, SYSRANGE_START, 0x00);
    //LDByteWriteI2C(0x29, INTERLEAVED_MODE_ENABLE, 0x00);
    
    I2Cdev_writeBytes(0x29, SYSALS_START, 1, 0x00);
    I2Cdev_writeBytes(0x29, SYSRANGE_START, 1, 0x00);
    I2Cdev_writeBytes(0x29, INTERLEAVED_MODE_ENABLE, 1, 0x00);
    
    return true;
}

int newRangeData(void)
{
    uint8_t readStatus;
    //readOne(I2C, RESULT_INTERRUPT_STATUS, &readStatus);
    //LDByteReadI2C(0x29, RESULT_INTERRUPT_STATUS_GPIO, &readStatus, 1);
    I2Cdev_readBytes(0x29, RESULT_INTERRUPT_STATUS_GPIO, 1, &readStatus);
    
    readStatus &= 0x07;         // mask off upper bits [7:3].
    //return (readStatus == 4);   // 4 indicates new sample is ready.
    if (readStatus == 4)
    {
        return 1;
    } else {
        return 0;
    }
       
}

int newALS_Data(void)
{
    uint8_t readStatus;
    //readOne(I2C, RESULT_INTERRUPT_STATUS, &readStatus);
    //LDByteReadI2C(0x29, RESULT_INTERRUPT_STATUS_GPIO, &readStatus, 1);
    I2Cdev_readBytes(0x29, RESULT_INTERRUPT_STATUS_GPIO, 1, readStatus);
    
    readStatus = (0xC0 & readStatus) >> 3;  // mask off bits [7:6] and [2:0].
    //return (readStatus == 4);
    if (readStatus == 4)
    {
        return 1;
    } else {
        return 0;
    }
}


/**
 * \brief continuous modes necessitate register changes only. Single-shot
 *        mode does not.
 */
void setMode(int mode)
{
    //vl6180-params.mode_ = mode;
    //uint8_t readStatus = 0;

    switch (mode) {
        //case (RANGE_SINGLE_SHOT):
        case 1:
            vl6180_clearModes();   // Disable interleaved and continous modes.
            /// no additional tweaks needed at this point.
        //case (ALS_SINGLE_SHOT):
        case 2:
            vl6180_clearModes();   // Disable interleaved and continous modes.
            /// no additional tweaks needed at this point.
        //case (ALS_CONTINUOUS):
        case 3:
            vl6180_clearModes();
            //writeOne(I2C, SYSALS_START, 0x03);
            //LDByteWriteI2C(0x29, SYSALS_START, 0x03);
            I2Cdev_writeBytes(0x29, SYSALS_START, 1, 0x03);
            break;
        //case (RANGE_CONTINUOUS):
        case 4:
            vl6180_clearModes();
            //writeOne(I2C, SYSRANGE_START, 0x03);
            //LDByteWriteI2C(0x29, SYSRANGE_START, 0x03);
            I2Cdev_writeBytes(0x29, SYSRANGE_START, 1, 0x03);
            break;
        //case (RANGE_CONTINUOUS_AND_ALS_SINGLE_SHOT):
        case 5:
            vl6180_clearModes();
            //writeOne(I2C, SYSRANGE_START, 0x03);    /// set range mode to cnts.
            //LDByteWriteI2C(0x29, SYSRANGE_START, 0x03);
            I2Cdev_writeBytes(0x29, SYSRANGE_START, 1, 0x03);
            /// Leave single-shot setting alone for ALS.
            break;
        //case (RANGE_SINGLE_SHOT_AND_ALS_CONTINUOUS):
        case 6:
            vl6180_clearModes();
            //writeOne(I2C, SYSALS_START, 0x03);    /// set ALS mode to cnts.
            //LDByteWriteI2C(0x29, SYSALS_START, 0x03);
            I2Cdev_writeBytes(0x29, SYSALS_START, 1, 0x03);
            /// Leave single-shot setting alone for range.
            break;
        //case (INTERLEAVED):
        case 7:
            //writeOne(I2C, INTERLEAVEd_MODE_ENABLE, 0x01);
            //LDByteWriteI2C(0x29, INTERLEAVED_MODE_ENABLE, 0x01);
            I2Cdev_writeBytes(0x29, INTERLEAVED_MODE_ENABLE, 1, 0x01);
            /// do stuff;
            break;
        // default not needed since we're using enums!
        default:
            break;
    }
}

void setALS_Gain( float gain)
{
    //writeOne(...);  // TODO: FIXME
}


/* uint16_t readALS_FromContinuous()
{

}

uint8_t readRangeFromContinuous()
{

} */

float singleShotReadALS()
{
    uint16_t newData[2];
    uint16_t rawRegVal;

    setMode(2);
    //writeOne(I2C, SYSALS_START, 0x01);
    //LDByteWriteI2C(0x29, SYSALS_START, 0x01);
    I2Cdev_writeBytes(0x29, SYSALS_START, 1, 0x01);
    while (newALS_Data() == 0);
    //read(I2C, RESULT_ALS_VAL, newData);
    //LDByteReadI2C(0x29, RESULT_ALS_VAL, &newData, 1);
    I2Cdev_readBytes(0x29, RESULT_ALS_VAL, 1, &newData);
    rawRegVal = ((uint16_t)newData[0] << 8) & newData[1];

    return ALS_LuxResolution_ * (rawRegVal / ALS_Gain_) * 
           (100 / ALS_IntegrationTime_);
}

/**
 * \brief return the range value in mm
 */
uint8_t singleShotReadRange()
{
    uint8_t newData;

    setMode(1);
    //writeOne(I2C, SYSRANGE_START, 0x01);
    //LDByteWriteI2C(0x29, SYSRANGE_START, 0x01);
    I2Cdev_writeBytes(0x29, SYSRANGE_START, 1, 0x01);
    while (newRangeData() == 0);
    //readOne(I2C, RESULT_RANGE_VAL, &newData);
    //LDByteReadI2C(0x29, RESULT_RANGE_VAL, &newData, 1);
    I2Cdev_readBytes(0x29, RESULT_RANGE_VAL, 1, &newData);
    return newData;
}

void setSysALS_IntegrationPeriod(int ms)
{
    uint8_t encodedMs = ms - 1;
    //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, encodedMs);
    //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, (unsigned char)encodedMs);
    I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, &encodedMs);
}

void setSysALS_AnalogGain(float gain)
{
    if (gain <= 1.0)
    {
        //vl6180-params.ALS_Gain_ = 1.0;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 6); /// Default gain: 1.0
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 6);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x06);
    }
    else if (gain <= 1.25)
    {
        //vl6180-params.ALS_Gain_ = 1.25;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 5); 
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 5);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x05);
    }
    else if (gain <= 1.67)
    {
        //vl6180-params.ALS_Gain_ = 1.67;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 4); 
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 4);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x04);
    }
    else if (gain <= 2.5)
    {
        //vl6180-params.ALS_Gain_ = 2.5;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 3); 
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 3);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x03);
    }
    else if (gain <= 5.0)
    {
        //vl6180-params.ALS_Gain_ = 5.0;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 2); 
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 2);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x02);
    }
    else if (gain <= 10)
    {
        //vl6180-params.ALS_Gain_ = 10.0;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 1); 
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 1);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x01);
    }
    else if (gain <= 20)
    {
        //vl6180-params.ALS_Gain_ = 20.0;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 0); 
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 0);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x00);
    }
    else if (gain <= 40)
    {
        //vl6180-params.ALS_Gain_ = 40.0;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 7); 
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 7);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x07);
    }
    else
    {
        //vl6180-params.ALS_Gain_ = 1.0;
        //writeOne(I2C, SYSALS_INTEGRATION_PERIOD, 1, 6); /// Default gain: 1.0
        //LDByteWriteI2C(0x29, SYSALS_INTEGRATION_PERIOD, 6);
        I2Cdev_writeBytes(0x29, SYSALS_INTEGRATION_PERIOD, 1, 0x06);
    }
}

void initVL6180(void)
{
    vl6180_clearModes();
    //setALS_Gain(1);
    setSysALS_AnalogGain(1);
    setSysALS_IntegrationPeriod(100);
}

/* bool newRangeData()
{
    uint8_t readStatus;
    //readOne(I2C, RESULT_INTERRUPT_STATUS, &readStatus);
    LDByteReadI2C(0x29, RESULT_INTERRUPT_STATUS, &readStatus, 1);
    
    readStatus &= 0x07;         // mask off upper bits [7:3].
    //return (readStatus == 4);   // 4 indicates new sample is ready.
    if (readStatus == 4)
    {
        return true;
    } else {
        return false;
    }
       
} */