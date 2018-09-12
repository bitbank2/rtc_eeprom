//
// DS3231 and xxx
// Real Time Clock + EEPROM library
//
// Written by Larry Bank - 1/5/2018
// Copyright (c) 2018 BitBank Software, Inc.
// bitbank@pobox.com
//
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "rtc.h"

static int rtc_i2c = -1;
static int ee_i2c = -1;
//
// Opens a file system handle to the EEPROM I2C device
//
int eeInit(int iChannel, int iAddr)
{
char filename[32];
 
	sprintf(filename, "/dev/i2c-%d", iChannel);
	if ((ee_i2c = open(filename, O_RDWR)) < 0)
	{
		fprintf(stderr, "Failed to open the i2c bus; need to run as root?\n");
		return -1;
	}

	if (ioctl(ee_i2c, I2C_SLAVE, iAddr) < 0)
	{
		close(ee_i2c);
		fprintf(stderr, "Failed to acquire bus access or talk to slave\n");
		ee_i2c = -1;
		return -1;
	}
	return 0;
} /* eeInit() */

int eeReadByte(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[4];
int rc;

	if (iAddr != -1) // send the address
	{
		ucTemp[0] = (unsigned char)(iAddr >> 8);
		ucTemp[1] = (unsigned char)iAddr;
		rc = write(ee_i2c, ucTemp, 2);
		usleep(10000);
	} // otherwise read from the last address and increment
	rc = read(ee_i2c, pData, 1);
	return (rc == 1);
} /* eeReadByte() */

//
// Read a block of 32 bytes at the given address
// or from the last read address if iAddr == -1
//
int eeReadBlock(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[4];
int rc;

	if (iAddr != -1) // send the address
	{
		ucTemp[0] = (unsigned char)(iAddr >> 8);
		ucTemp[1] = (unsigned char)iAddr;
		rc = write(ee_i2c, ucTemp, 2);
	} // otherwise read from the last address and increment
	rc = read(ee_i2c, pData, 32);
	return (rc == 32);
} /* eeReadBlock() */

int eeWriteByte(int iAddr, unsigned char ucByte)
{
unsigned char ucTemp[4];
int rc;

	if (iAddr != -1) // send the address
	{
		ucTemp[0] = (unsigned char)(iAddr >> 8);
		ucTemp[1] = (unsigned char)iAddr;
		ucTemp[2] = ucByte;
		// The first data byte must be written with
		// the address atomically or it won't work
		rc = write(ee_i2c, ucTemp, 3);
		return (rc == 3);
	} // otherwise write from the last address and increment
	else
	{
		rc = write(ee_i2c, &ucByte, 1);
		return (rc == 1);
	}
} /* eeWriteByte() */

int eeWriteBlock(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[34];
int rc;

	if (iAddr != -1) // send the address
	{
		ucTemp[0] = (unsigned char)(iAddr >> 8);
		ucTemp[1] = (unsigned char)iAddr;
		memcpy(&ucTemp[2], pData, 32);
		rc = write(ee_i2c, ucTemp, 34);
		return (rc == 34);
	} // otherwise write to the last address and increment
	else
	{
		rc = write(ee_i2c, pData, 32);
		return (rc == 32);
	}
} /* eeWriteBlock() */

//
// Closes all file system handles
//
void rtcShutdown(void)
{
	if (rtc_i2c >= 0) close(rtc_i2c);
	if (ee_i2c >= 0) close(ee_i2c);
} /* rtcShutdown() */
//
// Opens a file system handle to the RTC I2C device
//
int rtcInit(int iChannel, int iAddr)
{
char filename[32];
unsigned char ucTemp[2];
 
	sprintf(filename, "/dev/i2c-%d", iChannel);
	if ((rtc_i2c = open(filename, O_RDWR)) < 0)
	{
		fprintf(stderr, "Failed to open the i2c bus; need to run as root?\n");
		return -1;
	}

	if (ioctl(rtc_i2c, I2C_SLAVE, iAddr) < 0)
	{
		close(rtc_i2c);
		fprintf(stderr, "Failed to acquire bus access or talk to slave\n");
		rtc_i2c = -1;
		return -1;
	}
	ucTemp[0] = 0xe; // control register
	write(rtc_i2c, ucTemp, 1);
	read(rtc_i2c, &ucTemp[1], 1); // read contents
	ucTemp[1] &= ~64; // turn off square wave on battery
	ucTemp[1] &= ~4; // enable time on battery
	write(rtc_i2c, ucTemp, 2); // write it back
//	ucTemp[0] = 0xf; // control register
//	ucTemp[1] = 0; // turn on oscillator and turn off alarms
//	write(rtc_i2c, ucTemp, 2);
	return 0;

} /* rtcInit() */

//
// Read the current internal temperature
// Value is celcius * 4 (resolution of 0.25C)
//
int rtcGetTemp(void)
{
unsigned char ucTemp[2];
int rc, iTemp = 0;

	ucTemp[0] = 0x11; // MSB location
	write(rtc_i2c, ucTemp, 1);
	rc = read(rtc_i2c, ucTemp, 2);
	if (rc == 2)
	{
		iTemp = ucTemp[0] << 8; // high byte
		iTemp |= ucTemp[1]; // low byte
		iTemp >>= 6; // lower 2 bits are fraction; upper 8 bits = integer part
	}
	return iTemp;
} /* rtcGetTemp() */

//
// Set the current time/date
//
int rtcSetTime(struct tm *pTime)
{
unsigned char ucTemp[20];
int i;
	ucTemp[0] = 0; // start at register 0
	// seconds
	ucTemp[1] = ((pTime->tm_sec / 10) << 4);
	ucTemp[1] |= (pTime->tm_sec % 10);
	// minutes
	ucTemp[2] = ((pTime->tm_min / 10) << 4);
	ucTemp[2] |= (pTime->tm_min % 10);
	// hours (and set 24-hour format)
	ucTemp[3] = ((pTime->tm_hour / 10) << 4);
	ucTemp[3] |= (pTime->tm_hour % 10);
	// day of the week
	ucTemp[4] = pTime->tm_wday + 1;
	// day of the month
	ucTemp[5] = (pTime->tm_mday / 10) << 4;
	ucTemp[5] |= (pTime->tm_mday % 10);
	// month + century
	i = pTime->tm_mon+1; // 1-12 on the RTC
	ucTemp[6] = (i / 10) << 4;
	ucTemp[6] |= (i % 10);
	if (pTime->tm_year >= 100)
		ucTemp[6] |= 0x80; // century bit
	// year
	ucTemp[7] = (((pTime->tm_year % 100)/10) << 4);
	ucTemp[7] |= (pTime->tm_year % 10);

	write(rtc_i2c, ucTemp, 8);
	return 0;
} /* rtcSetTime() */

//
// Read the current time/date
//
int rtcGetTime(struct tm *pTime)
{
unsigned char ucTemp[20];
int i,rc;

	ucTemp[0] = 0; // start of data registers we want
	rc = write(rtc_i2c, ucTemp, 1); // write address of register to read
	i = read(rtc_i2c, ucTemp, 7);
	if (rc < 0 || i != 7)
	{
		return -1; // something went wrong
	}
	memset(pTime, 0, sizeof(struct tm));
	// convert numbers from BCD
	pTime->tm_sec = ((ucTemp[0] >> 4) * 10) + (ucTemp[0] & 0xf);
	pTime->tm_min = ((ucTemp[1] >> 4) * 10) + (ucTemp[1] & 0xf);
	// hours are stored in 24-hour format in the tm struct
	if (ucTemp[2] & 64) // 12 hour format
	{
		pTime->tm_hour = ucTemp[2] & 0xf;
		pTime->tm_hour += ((ucTemp[2] >> 4) & 1) * 10;
		pTime->tm_hour += ((ucTemp[2] >> 5) & 1) * 12; // AM/PM
	}
	else // 24 hour format
	{
		pTime->tm_hour = ((ucTemp[2] >> 4) * 10) + (ucTemp[2] & 0xf);	
	}
	pTime->tm_wday = ucTemp[3] - 1; // day of the week (0-6)
	// day of the month
	pTime->tm_mday = ((ucTemp[4] >> 4) * 10) + (ucTemp[4] & 0xf);
	// month
	pTime->tm_mon = (((ucTemp[5] >> 4) & 1) * 10 + (ucTemp[5] & 0xf)) -1; // 0-11
	pTime->tm_year = (ucTemp[5] >> 7) * 100; // century
	pTime->tm_year += ((ucTemp[6] >> 4) * 10) + (ucTemp[6] & 0xf);

	return 0;

} /* rtcGetTime() */
//
// Set Alarm for:
// ALARM_SECOND = Once every second
// ALARM_MINUTE = Once every minute
// ALARM_TIME = When a specific hour:second match
// ALARM_DAY = When a specific day of the week and time match
// ALARM_DATE = When a specific day of the month and time match
//
void rtcSetAlarm(uint8_t type, struct tm *pTime)
{
unsigned char ucTemp[8];

  switch (type)
  {
    case ALARM_SECOND: // turn on repeating alarm for every second
      ucTemp[0] = 0xe; // control register
      ucTemp[1] = 0x1d; // enable alarm1 interrupt
      write(rtc_i2c, ucTemp, 2);
      ucTemp[0] = 0x7; // starting register for alarm 1
      ucTemp[1] = 0x80; // set bit 7 in the 4 registers to tell it a repeating alarm
      ucTemp[2] = 0x80;
      ucTemp[3] = 0x80;
      ucTemp[4] = 0x80;
      write(rtc_i2c, ucTemp, 5);
      break;
    case ALARM_MINUTE: // turn on repeating alarm for every minute
      ucTemp[0] = 0xe; // control register
      ucTemp[1] = 0x1e; // enable alarm2 interrupt
      write(rtc_i2c, ucTemp, 2);
      ucTemp[0] = 0xb; // starting register for alarm 2
      ucTemp[1] = 0x80; // set bit 7 in the 3 registers to tell it a repeating alarm
      ucTemp[2] = 0x80;
      ucTemp[3] = 0x80;
      write(rtc_i2c, ucTemp, 4);
      break;
    case ALARM_TIME: // turn on alarm to match a specific time
    case ALARM_DAY: // turn on alarm for a specific day of the week
    case ALARM_DATE: // turn on alarm for a specific date
      ucTemp[0] = 0xe; // control register
      ucTemp[1] = 0x1d; // enable alarm1 interrupt
      write(rtc_i2c, ucTemp, 2);
// Values are stored as BCD
      ucTemp[0] = 0x7; // start at register 7
      // seconds
      ucTemp[1] = ((pTime->tm_sec / 10) << 4);
      ucTemp[1] |= (pTime->tm_sec % 10);
      // minutes
      ucTemp[2] = ((pTime->tm_min / 10) << 4);
      ucTemp[2] |= (pTime->tm_min % 10);
      // hours (and set 24-hour format)
      ucTemp[3] = ((pTime->tm_hour / 10) << 4);
      ucTemp[3] |= (pTime->tm_hour % 10);
      // day of the week
      ucTemp[4] = pTime->tm_wday + 1;
      // day of the month
      ucTemp[5] = (pTime->tm_mday / 10) << 4;
      ucTemp[5] |= (pTime->tm_mday % 10);
      // set the A1Mx bits (high bits of the 4 registers)
      // for the specific type of alarm
      if (type == ALARM_TIME) // A1Mx bits should be x1100
      {
        ucTemp[1] |= 0x80;
        ucTemp[2] |= 0x80;
      }
      else if (type == ALARM_DAY) // A1Mx bits should be 10000
      {
        ucTemp[4] |= 0x40; // DY/DT bit
      }
      // for matching the date, all bits are left as 0's (00000)
      write(rtc_i2c, ucTemp, 6);
      break;
  } // switch on type
} /* rtcSetAlarm() */

//
// Reset the "fired" bits for Alarm 1 and 2
// Interrupts will not occur until these bits are cleared
//
void rtcClearAlarms(void)
{
unsigned char ucTemp[2];

  ucTemp[0] = 0xf; // control register
  ucTemp[1] = 0x0; // clear A1F & A2F (alarm 1 or 2 fired) bit to allow it to fire again
  write(rtc_i2c, ucTemp, 2);
} /* rtcClearAlarms() */

