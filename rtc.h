
#ifndef __RTC__
#define __RTC__

// Alarm types
enum {
  ALARM_SECOND=0,
  ALARM_MINUTE,
  ALARM_TIME,
  ALARM_DAY,
  ALARM_DATE
};

int rtcInit(int iChannel, int iAddr);
int eeInit(int iChannel, int iAddr);
void rtcShutdown(void);
int rtcGetTime(struct tm *pTime);
int rtcSetTime(struct tm *pTime);
int rtcGetTemp(void);
int eeReadByte(int iAddr, unsigned char *pData);
int eeReadBlock(int iAddr, unsigned char *pData);
int eeWriteByte(int iAddr, unsigned char ucByte);
int eeWriteBlock(int iAddr, unsigned char *pData);
void rtcSetAlarm(unsigned char type, struct tm *pTime);
void rtcClearAlarms(void);

#endif // __RTC__
