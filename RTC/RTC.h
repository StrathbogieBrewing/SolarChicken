#ifndef RTC_H
#define RTC_H

#include <inttypes.h>




class RTC {
public:
  RTC();
  
  void begin(uint32_t seconds);
  uint8_t getTicks();
  uint32_t getSeconds();
  void setSeconds(uint32_t seconds);
  
private:

};

extern RTC Clock;

#endif
