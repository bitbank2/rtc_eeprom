#include <Arduino.h>
#include <Wire.h>
#include <rtc_eeprom.h>

// Wrapper function to write I2C data on Arduino
static void I2CWrite(int iAddr, unsigned char *pData, int iLen)
{
  Wire.beginTransmission(iAddr);
  Wire.write(pData, iLen);
  Wire.endTransmission();
} /* I2CWrite() */

// Wrapper function to read I2C data on Arduino
static void I2CRead(int iAddr, unsigned char *pData, int iLen)
{
int x;

  Wire.requestFrom(iAddr, iLen);
  x = 0;
  while (x < iLen && Wire.available())
  {
    pData[x] = Wire.read();
    x++;
  }
} /* I2CRead() */
//
// Read a byte from the EEPROM
//
void eeReadByte(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[4];

  if (iAddr != -1) // set the address
  {
    ucTemp[0] = (unsigned char)(iAddr >> 8);
    ucTemp[1] = (unsigned char)iAddr;
    I2CWrite(EEPROM_ADDR, ucTemp, 2);
    delay(10);
  }
  // otherwise just read from the last address and auto-increment
  I2CRead(EEPROM_ADDR, pData, 1);
} /* eeReadByte() */
//
// Read a block of 32 bytes from the given address
// or from the last read address if iAddr == -1
//
void eeReadBlock(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[4];

  if (iAddr != -1) // set the address
  {
    ucTemp[0] = (unsigned char)(iAddr >> 8);
    ucTemp[1] = (unsigned char)iAddr;
    I2CWrite(EEPROM_ADDR, ucTemp, 2);
  }
  // otherwise just read from the last address and auto-increment
  I2CRead(EEPROM_ADDR, pData, 32);
} /* eeReadBlock() */
//
// Write a byte to the given address
// or the previous address if iAddr == -1
//
void eeWriteByte(int iAddr, unsigned char ucByte)
{
unsigned char ucTemp[4];

        if (iAddr != -1) // send the address
        {
                ucTemp[0] = (unsigned char)(iAddr >> 8);
                ucTemp[1] = (unsigned char)iAddr;
                ucTemp[2] = ucByte;
                // The first data byte must be written with
                // the address atomically or it won't work
                I2CWrite(EEPROM_ADDR, ucTemp, 3);
        } // otherwise write from the last address and increment
        else
        {
                I2CWrite(EEPROM_ADDR, &ucByte, 1);
        }
} /* eeWriteByte() */
//
// Write a block of 32 bytes to the given address
// or from the last read/write address is iAddr == -1
//
void eeWriteBlock(int iAddr, unsigned char *pData)
{
unsigned char ucTemp[34];

        if (iAddr != -1) // send the address
        {
                ucTemp[0] = (unsigned char)(iAddr >> 8);
                ucTemp[1] = (unsigned char)iAddr;
                memcpy(&ucTemp[2], pData, 32);
                I2CWrite(EEPROM_ADDR, ucTemp, 34);
        } // otherwise write to the last address and increment
        else
        {
                I2CWrite(EEPROM_ADDR, pData, 32);
        }
} /* eeWriteBlock() */
//
// Turn on the RTC
//
void rtcInit(void)
{
uint8_t ucTemp[2];

  ucTemp[0] = 0xe; // control register
  ucTemp[1] = 0x1c; // enable main oscillator and interrupt mode for alarms
  I2CWrite(RTC_ADDR, ucTemp, 2);
}
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
uint8_t ucTemp[8];

  switch (type)
  {
    case ALARM_SECOND: // turn on repeating alarm for every second
      ucTemp[0] = 0xe; // control register
      ucTemp[1] = 0x1d; // enable alarm1 interrupt
      I2CWrite(RTC_ADDR, ucTemp, 2);
      ucTemp[0] = 0x7; // starting register for alarm 1
      ucTemp[1] = 0x80; // set bit 7 in the 4 registers to tell it a repeating alarm
      ucTemp[2] = 0x80;
      ucTemp[3] = 0x80;
      ucTemp[4] = 0x80;
      I2CWrite(RTC_ADDR, ucTemp, 5);
      break;
    case ALARM_MINUTE: // turn on repeating alarm for every minute
      ucTemp[0] = 0xe; // control register
      ucTemp[1] = 0x1e; // enable alarm2 interrupt
      I2CWrite(RTC_ADDR, ucTemp, 2);
      ucTemp[0] = 0xb; // starting register for alarm 2
      ucTemp[1] = 0x80; // set bit 7 in the 3 registers to tell it a repeating alarm
      ucTemp[2] = 0x80;
      ucTemp[3] = 0x80;
      I2CWrite(RTC_ADDR, ucTemp, 4);
      break;
    case ALARM_TIME: // turn on alarm to match a specific time
    case ALARM_DAY: // turn on alarm for a specific day of the week
    case ALARM_DATE: // turn on alarm for a specific date
      ucTemp[0] = 0xe; // control register
      ucTemp[1] = 0x1d; // enable alarm1 interrupt
      I2CWrite(RTC_ADDR, ucTemp, 2);
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
      I2CWrite(RTC_ADDR, ucTemp, 6);
      break;
  } // switch on type
} /* rtcSetAlarm() */
//
// Read the current internal temperature
// Value is celcius * 4 (resolution of 0.25C)
//
int rtcGetTemp(void)
{
unsigned char ucTemp[2];
int iTemp = 0;

  ucTemp[0] = 0x11; // MSB location
  I2CWrite(RTC_ADDR, ucTemp, 1);
  I2CRead(RTC_ADDR, ucTemp, 2);
  iTemp = ucTemp[0] << 8; // high byte
  iTemp |= ucTemp[1]; // low byte
  iTemp >>= 6; // lower 2 bits are fraction; upper 8 bits = integer part
  return iTemp;
} /* rtcGetTemp() */
//
// Set the current time/date
//
void rtcSetTime(struct tm *pTime)
{
unsigned char ucTemp[20];
uint8_t i;

// Values are stored as BCD
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

        I2CWrite(RTC_ADDR, ucTemp, 8);
} /* rtcSetTime() */

//
// Read the current time/date
//
void rtcGetTime(struct tm *pTime)
{
unsigned char ucTemp[20];

        ucTemp[0] = 0; // start of data registers we want
        I2CWrite(RTC_ADDR, ucTemp, 1); // write address of register to read
        I2CRead(RTC_ADDR, ucTemp, 7);
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

} /* rtcGetTime() */
//
// Reset the "fired" bits for Alarm 1 and 2
// Interrupts will not occur until these bits are cleared
//
void rtcClearAlarms(void)
{
uint8_t ucTemp[2];

  ucTemp[0] = 0xf; // control register
  ucTemp[1] = 0x0; // clear A1F & A2F (alarm 1 or 2 fired) bit to allow it to fire again
  I2CWrite(RTC_ADDR, ucTemp, 2);
} /* rtcClearAlarms() */

