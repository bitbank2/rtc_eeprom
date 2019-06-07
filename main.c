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
	printf("getset_time - gets or sets the time of a DS3231 RTC\n");
	printf("written by Larry Bank\n\n");
	printf("Usage:\n");
	printf("getset_time set - sets the DS3231 time and date to the system date\n");
	printf("getset_time get - displays the DS3231 time and date\n");
} /* ShowHelp() */

int main(int argc, char *argv[])
{
int i;
struct tm *thetime;
time_t tt;

	if (argc != 2)
	{
		ShowHelp();
		return 0;
	}
	// I2C bus 1 is the default on RPI hardware
	// most other Linux systems expose I2C on bus 0
	i = rtcInit(1, 0x68); // open the I2C bus for the RTC
	if (i != 0)
	{
		return -1; // problem - quit
	}
	tt = time(NULL);  // get the current system time
	thetime = localtime(&tt);

	if (strcmp(argv[1], "get") == 0) // display RTC time
	{
		rtcGetTime(thetime);
		printf("DS3231 time = %02d:%02d:%02d\n", thetime->tm_hour, thetime->tm_min, thetime->tm_sec);
		printf("DS3231 date = %02d/%02d/%04d\n", thetime->tm_mon+1, thetime->tm_mday, thetime->tm_year + 1900);
	}
	else if (strcmp(argv[1], "set") == 0) // set RTC to system time
	{
		rtcSetTime(thetime); // set the current time
		printf("DS3231 time set to system time\n");
	}
	else
	{
		ShowHelp();
		return 0;
	}
	rtcShutdown(); // close the file handles

return 0;
} /* main() */
