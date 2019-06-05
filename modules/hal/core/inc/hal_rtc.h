#ifndef HAL_RTC_H
#define HAL_RTC_H

typedef struct {
  unsigned char secs;
  unsigned char mins;
  unsigned char hours;
  unsigned char year;
  unsigned char month;
  unsigned char day;
  unsigned char dayno;
  unsigned char pm;
} rtc_time_t;

void rtc_get_time(rtc_time_t *t);
void rtc_set_time(rtc_time_t *t);

#endif
