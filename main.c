//
// RTC + EEPROM test app
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "rtc.h"

static unsigned char ucTestPattern[32] = {0,31,1,30,2,29,3,28,4,27,5,26,6,25,7,24,
8,23,9,22,10,21,11,20,12,19,13,18,14,17,15,16};

int main(int argc, char *argv[])
{
int i;
struct tm *thetime;
time_t tt;

	tt = time(NULL);  // get the current time
	thetime = gmtime(&tt);


	i = rtcInit(0, 0x68); // open the I2C bus for the RTC
	if (i != 0)
	{
		return -1; // problem - quit
	}
	i = eeInit(0, 0x57); // open the I2C bus for the EERPOM
	if (i != 0)
	{
		return -1;
	}

//	rtcSetTime(thetime); // set the current time

	eeWriteBlock(1024, ucTestPattern); // write our 32-byte test pattern
//	for (i=0; i<32; i++)
//	{
//		eeWriteByte(1024+i, ucTestPattern[i]);
//		usleep(20000);
//	}
	usleep(20000); // give it time to finish the write
#ifdef BOGUS
	for (i=0; i<120; i++) // read values twice a second for 1 minute
	{
		rtcGetTime(thetime);
		printf("Current time = %02d:%02d:%02d\n", thetime->tm_hour, thetime->tm_min, thetime->tm_sec);
		printf("Current date = %02d/%02d/%04d\n", thetime->tm_mon+1, thetime->tm_mday, thetime->tm_year + 1900);
		usleep(500000);
	}
#endif
	// read back the test pattern and see if it worked
	{
	unsigned char ucTemp[32];
		for (i=0; i<32; i++) // read back the data block 1 byte at a time
		{
			if (i==0)
				eeReadByte(1024, ucTemp); // read first byte
			else
				eeReadByte(-1, &ucTemp[i]); // read the rest
		printf("byte %d = %02x\n", i, ucTemp[i]);
		}
		if (memcmp(ucTemp, ucTestPattern, 32) == 0)
			printf("EEPROM works!\n");
		else
			printf("EEPROM failed!\n");
	}
	rtcShutdown(); // close the file handles

return 0;
} /* main() */
