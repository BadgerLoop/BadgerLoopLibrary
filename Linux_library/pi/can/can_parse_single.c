#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <limits.h>
#include <inttypes.h>
#include <time.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <wiringSerial.h>

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
	//char serialData[256];
	char hex[2];
	char decimal[3];
	int breakOn = 1;

	uint8_t a_unsigned_array[2];

	//Send over netcat code
    /* int pipes[2];
	pid_t pid;
	FILE *stream; */

	canbus = popen("candump can0", "r");

	if (argc >= 2)
	{
		breakOn = 0;
	}

	if (canbus == NULL)
	{
		printf("Error opening CAN Bus for reading\n");
		return 0;
	}

	/*int arduinoSerial = serialOpen("/dev/ttyUSB0", 9600);
	if (arduinoSerial == -1)
	{
		printf("Error opening arduinoSerial\n");
		return 0;
	} */

	while (fgets(canMessage, sizeof(canMessage), canbus) != NULL)
	{
    	char buffer[50];
    	//printf("%s", canMessage);

    	//Get messageID
    	decimal[0] = canMessage[8];
		decimal[1] = canMessage[9];
		decimal[2] = canMessage[10];
		int messageID = (int)strtol(decimal, NULL, 10);

		/* hex[0] = canMessage[19];
		hex[1] = canMessage[20];
		int sensorID = (int)strtol(hex, NULL, 16); */
		if (messageID == 2) //BPM - optical encoder value
    	{

			//Get optical encoder value
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			uint8_t optEn = (uint8_t)strtol(hex, NULL, 16);

    		printf("bpm_optEn_%d\n", optEn);

    		sprintf (buffer, "bpm_optEn_%d\n", optEn);
    		
    		if (breakOn == 1)
			{
				break;
			}
    	}
    	/* if (messageID == 10) //BPM - crank speed value
    	{

			//Get crank speed value
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			uint8_t optEn = (uint8_t)strtol(hex, NULL, 16);

    		printf("bpm_optEn_%d\n", optEn);

    		sprintf (buffer, "bpm_optEn_%d\n", optEn);
    		
    		if (breakOn == 1)
			{
				break;
			}
    	} */
		if (messageID == 3) //BPM2 - battery voltage value
    	{

			//Get battery voltage value
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			uint8_t battVolt = (uint8_t)strtol(hex, NULL, 16);

			double battVoltScaled = ((double) battVolt) * 0.0196;
			if (battVoltScaled < 1)
			{
				battVoltScaled = 0;
			}

    		printf("bpm2_battVolt_%f\n", battVoltScaled);

    		sprintf (buffer, "bpm2_battVolt_%f\n", battVoltScaled);
    		
    		if (breakOn == 1)
			{
				break;
			}
    	}
		if (messageID == 4) //ECM - thermistor sensor value
    	{
			//Get thermistor sensor value
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			uint8_t therm = (uint8_t)strtol(hex, NULL, 16);

			double thermScaled = ((double) therm) / 1.0;

    		printf("ecm_therm_%f\n", thermScaled);

    		sprintf (buffer, "ecm_therm_%f\n", thermScaled);
    		
    		if (breakOn == 1)
			{
				break;
			}
    	}
    	if (messageID == 5) //ECM - proximity sensor value
    	{
			//Get thermistor sensor value
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			uint8_t therm2 = (uint8_t)strtol(hex, NULL, 16);

			double therm2Scaled = ((double) therm2) / 1.0;

    		printf("ecm_therm2_%f\n", therm2Scaled);

    		sprintf (buffer, "ecm_therm2_%f\n", therm2Scaled);
    		
    		if (breakOn == 1)
			{
				break;
			}
    	}

    	if (messageID == 6) //VCM - accel
    	{
    		//Get AFS_SEL value
    		/* AFS_SEL | Full Scale Range | LSB Sensitivity
         	 * --------+------------------+----------------
         	 * 0       | +/- 2g           | 8192 LSB/mg
         	 * 1       | +/- 4g           | 4096 LSB/mg
         	 * 2       | +/- 8g           | 2048 LSB/mg
         	 * 3       | +/- 16g          | 1024 LSB/mg
         	 */
			//int AFS_SEL = (int)strtol(hex, NULL, 16);
			double divisor = 1671.8354245;

			//Get ax value
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[25];
			hex[1] = canMessage[26];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double ax = (double) (int16_t) convertFrom8To16( a_unsigned_array[1], a_unsigned_array[0] );
			ax = ax/divisor;

			//Get ay value
			hex[0] = canMessage[28];
			hex[1] = canMessage[29];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[31];
			hex[1] = canMessage[32];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double ay = (double) (int16_t) convertFrom8To16( a_unsigned_array[1], a_unsigned_array[0] );
			ay = ay/divisor;

			//Get ax value
			hex[0] = canMessage[34];
			hex[1] = canMessage[35];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[37];
			hex[1] = canMessage[38];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double az = (double) (int16_t) convertFrom8To16( a_unsigned_array[1], a_unsigned_array[0] );
			az = az/divisor;

    		printf("vcm_accel_%f_%f_%f\n", ax, ay, az);

    		sprintf (buffer, "vcm_accel_%f_%f_%f\n", ax, ay, az);

    		if (breakOn == 1)
			{
				break;
			}
    	}
    	if (messageID == 7) //VCM - gyro
    	{
			double divisor = 16375;

			//Get ax value
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[25];
			hex[1] = canMessage[26];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double gx = (double) (int16_t) convertFrom8To16( a_unsigned_array[1], a_unsigned_array[0] );
			gx = gx/divisor;

			//Get ay value
			hex[0] = canMessage[28];
			hex[1] = canMessage[29];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[31];
			hex[1] = canMessage[32];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double gy = (double) (int16_t) convertFrom8To16( a_unsigned_array[1], a_unsigned_array[0] );
			gy = gy/divisor;

			//Get ax value
			hex[0] = canMessage[34];
			hex[1] = canMessage[35];
			a_unsigned_array[0] = (uint8_t)strtol(hex, NULL, 16);

			hex[0] = canMessage[37];
			hex[1] = canMessage[38];
			a_unsigned_array[1] = (uint8_t)strtol(hex, NULL, 16);

			double gz = (double) (int16_t) convertFrom8To16( a_unsigned_array[1], a_unsigned_array[0] );
			gz = gz/divisor;

    		printf("vcm_gyro_%f_%f_%f\n", gx, gy, gz);

    		sprintf (buffer, "vcm_gyro_%f_%f_%f\n", gx, gy, gz);
    		
    		if (breakOn == 1)
			{
				break;
			}
    	}
    	if (messageID == 8) //MCM - proximity sensor value
    	{
			//Get proximity sensor value value
			hex[0] = canMessage[22];
			hex[1] = canMessage[23];
			uint8_t prox = (uint8_t)strtol(hex, NULL, 16);

			double proxScaled = ((double) prox) / 1.0;

    		printf("mcm_prox_%f\n", proxScaled);

    		sprintf (buffer, "mcm_prox_%f\n", proxScaled);
    		
    		if (breakOn == 1)
			{
				break;
			}
    	}
    	//infinite while check
    	if (breakOn == 1)
		{
			break;
		}

		/*while (serialDataAvail(arduinoSerial) > 0)
		{
			char serialChar = serialGetchar(arduinoSerial);
			printf("%c", serialChar);
		} */

		/*//Send over netcat code
    	int pipes[2];
		pid_t pid;
		FILE *stream;

		switch(fork())
		{
    	case -1:
        	fprintf(stderr, "error forking\n");
        	break;
    	case 0:
        	dup2(pipes[0], STDIN_FILENO);

        	pid = getpid();
        	printf("in child, pid=%d\n", pid);

        	if(close(pipes[1]) == -1)
            	fprintf(stderr,"err closing write end pid=%d\n", pid);

       		if(close(pipes[0]) == -1)
            	fprintf(stderr,"err closing write end pid=%d\n", pid);

        	//char *argv[3] = {"192.168.2.1", "2999", NULL};
        	//execvp("nc", argv);
        	execl("nc", "192.168.2.1", "2999", (char*) NULL);
        	exit(EXIT_FAILURE);
        	break;
    	default:
        	stream = fdopen(pipes[1], "w");
        	pid = getpid();
        	printf("in parent, pid=%d\n", pid);

        	if (stream == NULL)
            	fprintf(stderr, "could not create file streami\n");

        	if(close(pipes[0]) == -1)
            	printf("err closing read end pid=%d\n", pid);

        	fputs(buffer,stream);

        	if(fclose(stream) == EOF)
            	fprintf(stderr, "error while closing stream\n");
        	break;
    	} */

    }

	/* switch(fork())
	{
    	case -1:
        	fprintf(stderr, "error forking\n");
        	break;
    	case 0:
        	dup2(pipes[0], STDIN_FILENO);

        	pid = getpid();
        	printf("in child, pid=%d\n", pid);

        	if(close(pipes[1]) == -1)
            	fprintf(stderr,"err closing write end pid=%d\n", pid);

       		if(close(pipes[0]) == -1)
            	fprintf(stderr,"err closing write end pid=%d\n", pid);

        	//char *argv[3] = {"192.168.2.1", "2999", NULL};
        	//execvp("nc", argv);
        	execl("nc", "192.168.2.1", "2999", (char*) NULL);
        	exit(EXIT_FAILURE);
        	break;
    	default:
        	stream = fdopen(pipes[1], "w");
        	pid = getpid();
        	printf("in parent, pid=%d\n", pid);

        	if (stream == NULL)
            	fprintf(stderr, "could not create file streami\n");

        	if(close(pipes[0]) == -1)
            	printf("err closing read end pid=%d\n", pid);

        	fputs(buffer,stream);

        	if(fclose(stream) == EOF)
            	fprintf(stderr, "error while closing stream\n");
        	break;
    } */

	return 0;
}