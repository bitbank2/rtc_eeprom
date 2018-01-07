//
// DS3231 and AT24C32
// Real Time Clock + EEPROM library
//
// Written by Larry Bank - 1/5/2018
// Copyright (c) 2018 BitBank Software, Inc.
// bitbank@pobox.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

static int rtc_i2c = -1;
static int ee_i2c = -1;
//
// Opens a file system handle to the EEPROM I2C device
// success = 1, failure = 0
//
int eeInit(int iChannel, int iAddr)
{
char filename[32];
 
	sprintf(filename, "/dev/i2c-%d", iChannel);
	if ((ee_i2c = open(filename, O_RDWR)) < 0)
	{
		fprintf(stderr, "Failed to open the i2c bus; need to run as root?\n");
		return 0;
	}

	if (ioctl(ee_i2c, I2C_SLAVE, iAddr) < 0)
	{
		close(ee_i2c);
		fprintf(stderr, "Failed to acquire bus access or talk to slave\n");
		ee_i2c = -1;
		return 0;
	}
	return 1;
} /* eeInit() */

//
// Read a single byte from the current or given address
// and increment the internal address
// Pass -1 as the address to use the current
// returns 1 for success, 0 for failure
//
int eeReadByte(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[4];
int rc;

	if (ee_i2c < 0) return 0; // not initialized

	if (iAddr != -1) // send the address
	{
		if (iAddr & 0x1f) // must be on a page boundary
			return 0;
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
// returns 1 for success, 0 for failure
//
int eeReadBlock(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[4];
int rc;

	if (ee_i2c < 0) return 0; // not initialized

	if (iAddr != -1) // send the address
	{
		if (iAddr & 0x1f) // must be on a block boundary
			return 0;
		ucTemp[0] = (unsigned char)(iAddr >> 8);
		ucTemp[1] = (unsigned char)iAddr;
		rc = write(ee_i2c, ucTemp, 2);
	} // otherwise read from the last address and increment
	rc = read(ee_i2c, pData, 32);
	return (rc == 32);
} /* eeReadBlock() */

//
// Write a single byte to the given address or to the
// last address. Auto-increments the internal address
// Success = 1, failure = 0
//
int eeWriteByte(int iAddr, unsigned char ucByte)
{
unsigned char ucTemp[4];
int rc;

	if (ee_i2c < 0) return 0; // not initialized

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

//
// Write a block of 32-bytes. Must be on a block boundary
// Pass a starting address or -1 to use the last address
// Success = 1, failure = 0
//
int eeWriteBlock(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[34];
int rc;

	if (ee_i2c < 0) return 0; // not initialized

	if (iAddr != -1) // send the address
	{
		if (iAddr & 0x1f) // not a block boundary
			return 0;
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
// Closes all file system handles (rtc + eeprom)
//
void rtcShutdown(void)
{
	if (rtc_i2c >= 0) close(rtc_i2c);
	if (ee_i2c >= 0) close(ee_i2c);
	rtc_i2c = ee_i2c = -1;

} /* rtcShutdown() */
//
// Opens a file system handle to the RTC I2C device
// success = 1, failure = 0
//
int rtcInit(int iChannel, int iAddr)
{
int rc;
char filename[32];
unsigned char ucTemp[2];
 
	sprintf(filename, "/dev/i2c-%d", iChannel);
	if ((rtc_i2c = open(filename, O_RDWR)) < 0)
	{
		fprintf(stderr, "Failed to open the i2c bus; need to run as root?\n");
		return 0;
	}

	if (ioctl(rtc_i2c, I2C_SLAVE, iAddr) < 0)
	{
		close(rtc_i2c);
		fprintf(stderr, "Failed to acquire bus access or talk to slave\n");
		rtc_i2c = -1;
		return 0;
	}
	ucTemp[0] = 0xe; // control register
	rc = write(rtc_i2c, ucTemp, 1);
	rc = read(rtc_i2c, &ucTemp[1], 1); // read contents
	ucTemp[1] &= ~64; // turn off square wave on battery
	ucTemp[1] &= ~4; // enable time on battery
	rc = write(rtc_i2c, ucTemp, 2); // write it back
	return (rc == 2);

} /* rtcInit() */

//
// Read the current internal temperature
// Value in celcius * 4 (resolution of 0.25C)
// Accurate to +/-3C
//
int rtcGetTemp(void)
{
unsigned char ucTemp[2];
int rc, iTemp = 0;

	if (rtc_i2c < 0) return 0; // not initialized

	ucTemp[0] = 0x11; // MSB location
	rc = write(rtc_i2c, ucTemp, 1);
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
// success = 1, failure = 0
//
int rtcSetTime(struct tm *pTime)
{
unsigned char ucTemp[20];
int i, rc;

	if (rtc_i2c < 0) return 0; // not initialized

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

	rc = write(rtc_i2c, ucTemp, 8);
	return (rc == 8);
} /* rtcSetTime() */

//
// Read the current time/date
// success = 1, failure = 0
//
int rtcGetTime(struct tm *pTime)
{
unsigned char ucTemp[20];
int i,rc;

	if (rtc_i2c < 0) return 0; // not initialized

	ucTemp[0] = 0; // start of data registers we want
	rc = write(rtc_i2c, ucTemp, 1); // write address of register to read
	i = read(rtc_i2c, ucTemp, 7);
	if (rc < 0 || i != 7)
	{
		return 0; // something went wrong
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

	return 1;

} /* rtcGetTime() */
