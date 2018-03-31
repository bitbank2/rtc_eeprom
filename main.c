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

void ShowHelp(void)
{
	printf("RTC library demo program\n\n");
	printf("Usage: ./rtc_test <command>\n");
	printf("Available commands:\n\n");
	printf("getsys -  Display the current system time\n");
	printf("getchip - Display the current DS3231 time\n");
	printf("setchip - Write the system time to the DS3231\n");
} /* ShowHelp() */

int main(int argc, char *argv[])
{
int i;
struct tm *thetime, mytime;
time_t tt;

	if (argc != 2)
	{
		ShowHelp();
		return 0;
	}
	i = rtcInit(0, 0x68); // open the I2C bus for the RTC
	if (i != 1)
	{
		printf("DS3231 failed to initialize\n");
		return -1; // problem - quit
	}
	i = eeInit(0, 0x57); // open the I2C bus for the EERPOM
	if (i != 1)
	{
		printf("EEPROM failed to initialize\n");
	}
	tt = time(NULL);  // get the current time
	thetime = gmtime(&tt);

// Take the command line action
	if (strcmp(argv[1], "getsys") == 0) // display the system time
	{
		printf("Time+Date read from the system clock\n");
	}
	else if (strcmp(argv[1], "getchip") == 0) // display the chip time
	{
		rtcGetTime(&mytime);
		thetime = &mytime;
		printf("Time+Date read from the DS3231\n");
	}
	else if (strcmp(argv[1], "setchip") == 0) // set chip time
	{
		rtcSetTime(thetime);
		printf("Time+Date written to the DS3231\n");
	}
	printf("Time: %02d:%02d:%02d\n", thetime->tm_hour, thetime->tm_min, thetime->tm_sec);
	printf("Date: %02d/%02d/%04d\n", thetime->tm_mon+1, thetime->tm_mday, thetime->tm_year + 1900);

	// Display the temperature
	i = rtcGetTemp();
	printf("DS3231 Temperature = %2.1fC\n", (float)i / 4);

	rtcShutdown(); // close the file handles

return 0;
} /* main() */
