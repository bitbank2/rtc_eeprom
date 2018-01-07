
#ifndef __RTC__
#define __RTC__
//
// Opens a file system handle to the RTC I2C device
// success = 1, failure = 0
//
int rtcInit(int iChannel, int iAddr);

//
// Opens a file system handle to the EEPROM I2C device
// success = 1, failure = 0
//
int eeInit(int iChannel, int iAddr);

//
// Closes all file system handles (rtc + eeprom)
//
void rtcShutdown(void);

//
// Read the current time/date
// success = 1, failure = 0
//
int rtcGetTime(struct tm *pTime);

//
// Set the current time/date
// success = 1, failure = 0
//
int rtcSetTime(struct tm *pTime);

//
// Read the current internal temperature
// Value in celcius * 4 (resolution of 0.25C)
// Accurate to +/-3C
//
int rtcGetTemp(void);

//
// Read a single byte from the current or given address
// and increment the internal address
// Pass -1 as the address to use the current
// returns 1 for success, 0 for failure
//
int eeReadByte(int iAddr, unsigned char *pData);

//
// Read a block of 32 bytes at the given address
// or from the last read address if iAddr == -1
// returns 1 for success, 0 for failure
//
int eeReadBlock(int iAddr, unsigned char *pData);

//
// Write a single byte to the given address or to the
// last address. Auto-increments the internal address
// Success = 1, failure = 0
//
int eeWriteByte(int iAddr, unsigned char ucByte);

//
// Write a block of 32-bytes. Must be on a block boundary
// Pass a starting address or -1 to use the last address
// Success = 1, failure = 0
//
int eeWriteBlock(int iAddr, unsigned char *pData);

#endif // __RTC__
