#include "MS5803-01BA.h"
#include "I2Cdev.h"

/* Based on drivers ported from an Arduino library
 * found at https://github.com/millerlp/MS5803_01
 */

#define MS5803_I2C_ADDRESS    0x76 // or 0x77

#define CMD_RESET		0x1E	// ADC reset command
#define CMD_ADC_READ	0x00	// ADC read command
#define CMD_ADC_CONV	0x40	// ADC conversion command
#define CMD_ADC_D1		0x00	// ADC D1 conversion
#define CMD_ADC_D2		0x10	// ADC D2 conversion
#define CMD_ADC_256		0x00	// ADC resolution=256
#define CMD_ADC_512		0x02	// ADC resolution=512
#define CMD_ADC_1024	0x04	// ADC resolution=1024
#define CMD_ADC_2048	0x06	// ADC resolution=2048
#define CMD_ADC_4096	0x08	// ADC resolution=4096

#ifndef int64_t
typedef signed long long int64_t;
#endif

static float mbar; // Store pressure in mbar. 
static float tempC; // Store temperature in degrees Celsius
//    float tempF; // Store temperature in degrees Fahrenheit
//    float psiAbs; // Store pressure in pounds per square inch, absolute
//    float psiGauge; // Store gauge pressure in pounds per square inch (psi)
//    float inHgPress;  // Store pressure in inches of mercury
//    float mmHgPress;  // Store pressure in mm of mercury
static int32_t mbarInt; // pressure in mbar, initially as a signed long integer

// Create array to hold the 8 sensor calibration coefficients
static unsigned int      sensorCoeffs[8]; // unsigned 16-bit integer (0-65535)
// D1 and D2 need to be unsigned 32-bit integers (long 0-4294967295)
static uint32_t     D1 = 0;    // Store uncompensated pressure value
static uint32_t     D2 = 0;    // Store uncompensated temperature value
// These three variables are used for the conversion steps
// They should be signed 32-bit integer initially 
// i.e. signed long from -2147483648 to 2147483647
static int32_t	dT = 0;
static int32_t 	TEMP = 0;
// These values need to be signed 64 bit integers 
// (long long = int64_t)
static int64_t	Offset = 0;
static int64_t	Sensitivity  = 0;
static int64_t	T2 = 0;
static int64_t	OFF2 = 0;
static int64_t	Sens2 = 0;
// bytes to hold the results from I2C communications with the sensor
static BYTE HighByte;
static BYTE MidByte;
static BYTE LowByte;

static BYTE I2C_Bytes[3];

static uint16_t Resolution;

float temperature(void) {return tempC;}
    // Return pressure in mbar.
float pressure(void) {return mbar;}
//    // Return temperature in degress Fahrenheit.
//    float temperatureF() const        {return tempF;}
//    // Return pressure in psi (absolute)
//    float psia() const                {return psiAbs;}
//    // Return pressure in psi (gauge)
//    float psig() const                {return psiGauge;}
//    // Return pressure in inHg
//    float inHg() const                {return inHgPress;}
//    // Return pressure in mmHg
//    float mmHg() const                {return mmHgPress;}
    // Return the D1 and D2 values, mostly for troubleshooting
unsigned long D1val(void) {return D1;}
unsigned long D2val(void) {return D2;}

//-------------------------------------------------
// Set resolution
void MS_5803_setResolution(uint16_t res) {
    // The argument is the oversampling resolution, which may have values
    // of 256, 512, 1024, 2048, or 4096.
    Resolution = res;
}

//----------------------------------------------------------------
// Sends a power on reset command to the sensor.
void MS_5803_resetSensor(void) {
		I2Cdev_writeBytes(MS5803_I2C_ADDRESS, CMD_RESET, 0 ,0);
        Delay100TCYx(10); // ~10ms
}

unsigned char MS_5803_CRC(unsigned int n_prom[]) {
    int cnt;                // simple counter
    unsigned int n_rem;     // crc reminder
    unsigned int crc_read;  // original value of the CRC
    unsigned char  n_bit;
    n_rem = 0x00;
    crc_read = sensorCoeffs[7];     // save read CRC
    sensorCoeffs[7] = (0xFF00 & (sensorCoeffs[7])); // CRC byte replaced with 0
    for (cnt = 0; cnt < 16; cnt++)
    { // choose LSB or MSB
        if (cnt%2 == 1) {
            n_rem ^= (unsigned short)((sensorCoeffs[cnt>>1]) & 0x00FF);
        }
        else {
            n_rem ^= (unsigned short)(sensorCoeffs[cnt>>1] >> 8);
        }
        for (n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & (0x8000))
            {
                n_rem = (n_rem << 1) ^ 0x3000;
            }
            else {
                n_rem = (n_rem << 1);
            }
        }
    }
    n_rem = (0x000F & (n_rem >> 12));// // final 4-bit reminder is CRC code
    sensorCoeffs[7] = crc_read; // restore the crc_read to its original place
    // Return n_rem so it can be compared to the sensor's CRC value
    return (n_rem ^ 0x00); 
}

//-----------------------------------------------------------------
// Send commands and read the temperature and pressure from the sensor
unsigned long MS_5803_ADC(char commandADC) {
    // D1 and D2 will come back as 24-bit values, and so they must be stored in 
    // a long integer on 8-bit Arduinos.
    long result = 0;
    // Send the command to do the ADC conversion on the chip
    I2Cdev_writeBytes(MS5803_I2C_ADDRESS, CMD_ADC_CONV + commandADC, 0 ,0);
    // Wait a specified period of time for the ADC conversion to happen
    // See table on page 1 of the MS5803 data sheet showing response times of
    // 0.5, 1.1, 2.1, 4.1, 8.22 ms for each accuracy level. 
    switch (commandADC & 0x0F) 
    {
        case CMD_ADC_256 :
            Delay100TCYx(1); // ~1ms
            break;
        case CMD_ADC_512 :
            Delay100TCYx(3); // ~3ms
            break;
        case CMD_ADC_1024:
            Delay100TCYx(4); // ~4ms
            break;
        case CMD_ADC_2048:
            Delay100TCYx(6); // ~6ms
            break;
        case CMD_ADC_4096:
            Delay100TCYx(10); // ~10ms
            break;
    }
    // Now send the read command to the MS5803 
    I2Cdev_readBytes(MS5803_I2C_ADDRESS, CMD_ADC_READ, 3, I2C_Bytes);
    // Then request the results. This should be a 24-bit result (3 bytes)
    // Combine the bytes into one integer
    result = ((long)I2C_Bytes[0] << 16) + ((long)I2C_Bytes[1] << 8) + (long)I2C_Bytes[2];
    return result;
}

//-------------------------------------------------
BOOL MS_5803_init(BOOL Verbose) {
    MS_5803_resetSensor(); 
    
    if (Verbose) {
        // Display the oversampling resolution or an error message
        if (Resolution == 256 | Resolution == 512 | Resolution == 1024 | Resolution == 2048 | Resolution == 4096){
            /* Serial.print("Oversampling setting: ");
            Serial.println(_Resolution); */        
        } else {
            /* Serial.println("*******************************************");
            Serial.println("Error: specify a valid oversampling value");
            Serial.println("Choices are 256, 512, 1024, 2048, or 4096");
            Serial.println("*******************************************"); */
        }

    }
    // Read sensor coefficients
    for (int i = 0; i < 8; i++ ){
        // The PROM starts at address 0xA0
        I2Cdev_readBytes(MS5803_I2C_ADDRESS, 0xA0 + (i * 2), 2, I2C_Bytes);
        sensorCoeffs[i] = (((unsigned int)I2C_Bytes[0] << 8) + I2C_Bytes[1]);
        if (Verbose){
            /* // Print out coefficients 
            Serial.print("C");
            Serial.print(i);
            Serial.print(" = ");
            Serial.println(sensorCoeffs[i]);
            delay(10); */
        }
    }
    // The last 4 bits of the 7th coefficient form a CRC error checking code.
    unsigned char p_crc = sensorCoeffs[7];
    // Use a function to calculate the CRC value
    unsigned char n_crc = MS_5803_CRC(sensorCoeffs); 
    
    if (Verbose) {
        /* Serial.print("p_crc: ");
        Serial.println(p_crc);
        Serial.print("n_crc: ");
        Serial.println(n_crc); */
    }
    // If the CRC value doesn't match the sensor's CRC value, then the 
    // connection can't be trusted. Check your wiring. 
    if (p_crc != n_crc) {
        return false;
    }
    // Otherwise, return true when everything checks out OK. 
    return true;
}

//------------------------------------------------------------------
void MS_5803_readSensor(void) {
    // Choose from CMD_ADC_256, 512, 1024, 2048, 4096 for mbar resolutions
    // of 1, 0.6, 0.4, 0.3, 0.2 respectively. Higher resolutions take longer
    // to read.
    if (Resolution == 256){
        D1 = MS_5803_ADC(CMD_ADC_D1 + CMD_ADC_256); // read raw pressure
        D2 = MS_5803_ADC(CMD_ADC_D2 + CMD_ADC_256); // read raw temperature 
    } else if (Resolution == 512) {
        D1 = MS_5803_ADC(CMD_ADC_D1 + CMD_ADC_512); // read raw pressure
        D2 = MS_5803_ADC(CMD_ADC_D2 + CMD_ADC_512); // read raw temperature     
    } else if (Resolution == 1024) {
        D1 = MS_5803_ADC(CMD_ADC_D1 + CMD_ADC_1024); // read raw pressure
        D2 = MS_5803_ADC(CMD_ADC_D2 + CMD_ADC_1024); // read raw temperature
    } else if (Resolution == 2048) {
        D1 = MS_5803_ADC(CMD_ADC_D1 + CMD_ADC_2048); // read raw pressure
        D2 = MS_5803_ADC(CMD_ADC_D2 + CMD_ADC_2048); // read raw temperature
    } else if (Resolution == 4096) {
        D1 = MS_5803_ADC(CMD_ADC_D1 + CMD_ADC_4096); // read raw pressure
        D2 = MS_5803_ADC(CMD_ADC_D2 + CMD_ADC_4096); // read raw temperature
    }
    // Calculate 1st order temperature, dT is a long signed integer
    // D2 is originally cast as an uint32_t, but can fit in a int32_t, so we'll
    // cast both parts of the equation below as signed values so that we can
    // get a negative answer if needed
    dT = (int32_t)D2 - ( (int32_t)sensorCoeffs[5] * 256 );
    // Use integer division to calculate TEMP. It is necessary to cast
    // one of the operands as a signed 64-bit integer (int64_t) so there's no 
    // rollover issues in the numerator.
    TEMP = 2000 + ((int64_t)dT * sensorCoeffs[6]) / 8388608LL;
    // Recast TEMP as a signed 32-bit integer
    TEMP = (int32_t)TEMP;
    
    
    // All operations from here down are done as integer math until we make
    // the final calculation of pressure in mbar. 
    
    
    // Do 2nd order temperature compensation (see pg 9 of MS5803 data sheet)
    // I have tried to insert the fixed values wherever possible 
    // (i.e. 2^31 is hard coded as 2147483648).
    if (TEMP < 2000) {
        // For 1 bar model
        T2 = ((int64_t)dT * dT) / 2147483648ULL ; // 2^31 = 2147483648
        T2 = (int32_t)T2; // recast as signed 32bit integer
        OFF2 = 3 * ((TEMP-2000) * (TEMP-2000));
        Sens2 = 7 * ((TEMP-2000)*(TEMP-2000)) / 8 ;
    } else { // if TEMP is > 2000 (20.0C)
        // For 1 bar model
        T2 = 0;
        OFF2 = 0;
        Sens2 = 0;
        if (TEMP > 4500) {
            // Extra adjustment for high temps, only needed for 1 bar model
            Sens2 = Sens2 - ((TEMP-4500)*(TEMP-4500)) / 8;
        }
    }

    // Additional compensation for very low temperatures (< -15C)
    if (TEMP < -1500) {
        // For 1 bar model
        // Leave OFF2 alone in this case
        Sens2 = Sens2 + 2 * ((TEMP+1500)*(TEMP+1500));
    }
    
    // Calculate initial Offset and Sensitivity
    // Notice lots of casts to int64_t to ensure that the 
    // multiplication operations don't overflow the original 16 bit and 32 bit
    // integers
    
    // For 1 bar sensor
    Offset = (int64_t)sensorCoeffs[2] * 65536 + (sensorCoeffs[4] * (int64_t)dT) / 128;
    Sensitivity = (int64_t)sensorCoeffs[1] * 32768 + (sensorCoeffs[3] * (int64_t)dT) / 256;
    
    // Adjust TEMP, Offset, Sensitivity values based on the 2nd order 
    // temperature correction above.
    TEMP = TEMP - T2; // both should be int32_t
    Offset = Offset - OFF2; // both should be int64_t
    Sensitivity = Sensitivity - Sens2; // both should be int64_t
    
    // Final compensated pressure calculation. We first calculate the pressure
    // as a signed 32-bit integer (mbarInt), then convert that value to a
    // float (mbar). 

    // For 1 bar sensor
    mbarInt = ((D1 * Sensitivity) / 2097152 - Offset) / 32768;
    mbar = (float)mbarInt / 100;
    
    // Calculate the human-readable temperature in Celsius
    tempC  = (float)TEMP / 100; 
    
    // Start other temperature conversions by converting mbar to psi absolute
//    psiAbs = mbar * 0.0145038;
//    // Convert psi absolute to inches of mercury
//    inHgPress = psiAbs * 2.03625;
//    // Convert psi absolute to gauge pressure
//    psiGauge = psiAbs - 14.7;
//    // Convert mbar to mm Hg
//    mmHgPress = mbar * 0.7500617;
//    // Convert temperature to Fahrenheit
//    tempF = (tempC * 1.8) + 32;
    
}
