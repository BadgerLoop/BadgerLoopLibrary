#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <limits.h>
#include <inttypes.h>
#include <time.h>

uint16_t convertFrom8To16(uint8_t dataFirst, uint8_t dataSecond) {
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
}

struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

int main (int argc, char *argv[])
{
	FILE *canbus;
	char canMessage[256];
	char hex[2];

	uint8_t a_unsigned_array[2];

	struct timespec curr, prev;

	printf("CAN Bus Parser - Nicholas Jaunich\n");

	canbus = popen("candump can0", "r");

	if (canbus == NULL)
	{
		printf("Error opening CAN Bus for reading\n");
		return 0;
	}

	clock_gettime(CLOCK_REALTIME, &prev);
	clock_gettime(CLOCK_REALTIME, &curr);

	while (fgets(canMessage, sizeof(canMessage), canbus) != NULL)
	{
    	//printf("%s", canMessage);

    	//Get sensorID
		hex[0] = canMessage[19];
		hex[1] = canMessage[20];
		int sensorID = (int)strtol(hex, NULL, 16);

    	if (sensorID == 1)
    	{
    		//Get AFS_SEL value
    		/* AFS_SEL | Full Scale Range | LSB Sensitivity
         	 * --------+------------------+----------------
         	 * 0       | +/- 2g           | 8192 LSB/mg
         	 * 1       | +/- 4g           | 4096 LSB/mg
         	 * 2       | +/- 8g           | 2048 LSB/mg
         	 * 3       | +/- 16g          | 1024 LSB/mg
         	 */
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			int AFS_SEL = (int)strtol(hex, NULL, 16);
			double divisor = 16384;
			/*switch(AFS_SEL) {
				case 0:
					divisor = 8192;
					break;
   				case 1:
   					divisor = 4096;
					break;
				case 2:
   					divisor = 2048;
					break;
				case 3:
   					divisor = 1024;
					break;
   				default:
   					break;
			} */

			//Get ax value
			hex[0] = canMessage[25];
			hex[1] = canMessage[26];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[28];
			hex[1] = canMessage[29];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double ax = (double) (int16_t) convertFrom8To16( a_unsigned_array[0], a_unsigned_array[1] );
			ax = ax/divisor;

			//Get ay value
			hex[0] = canMessage[31];
			hex[1] = canMessage[32];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[34];
			hex[1] = canMessage[35];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double ay = (double) (int16_t) convertFrom8To16( a_unsigned_array[0], a_unsigned_array[1] );
			ay = ay/divisor;

			//Get ax value
			hex[0] = canMessage[37];
			hex[1] = canMessage[38];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[40];
			hex[1] = canMessage[41];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double az = (double) (int16_t) convertFrom8To16( a_unsigned_array[0], a_unsigned_array[1] );
			az = az/divisor;

    		//printf("SensorID = %d | MPU6050 (ax,ay,az) AFS_SEL = %d | ax = %+f g, ay = %+f g, az = %+f g\n", sensorID, AFS_SEL, ax, ay, az);
    		//time_t ltime; /* calendar time */

    		/* if (diff(prev,curr).tv_nsec > 100*1000000)
    			{
    				//ltime=time(NULL); // get current cal time
    				//printf("%s",asctime( localtime(&ltime) ) );

    				printf("%f %f %f\n", ax, ay, az);
    				clock_gettime(CLOCK_REALTIME, &prev);
    			} else {
    				printf("%s\n", "WAIT");
    			}
    			clock_gettime(CLOCK_REALTIME, &curr); */

    		if (argc > 1 && strcmp(argv[1], "raw") == 0) {

    			if (diff(prev,curr).tv_nsec > atoi(argv[2])*1000000)
    			{
    				//ltime=time(NULL); // get current cal time 
    				//printf("%s",asctime( localtime(&ltime) ) );

    				printf("%f %f %f\n", ax, ay, az);
    				clock_gettime(CLOCK_REALTIME, &prev);
    			} else {
    				//printf("%s\n", "WAIT");
    			}
    			clock_gettime(CLOCK_REALTIME, &curr);
    		}
    		else {
    			printf("SensorID = %d | MPU6050 (ax,ay,az) AFS_SEL = %d | ax = %+f m/s^2, ay = %+f m/s^2, az = %+f m/s^2\n", sensorID, AFS_SEL, ax*9.8, ay*9.8, az*9.8);
    		} 

    	}
    }

	return 0;
}