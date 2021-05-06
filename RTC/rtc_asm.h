#ifndef RTC_ASM_H
#define RTC_ASM_H



#if defined(__cplusplus)
extern "C" {
#endif
	
#include <stdint.h>

void rtc_begin(void);
uint8_t rtc_getTicks(void);
uint32_t rtc_getSeconds(void);
void rtc_setSeconds(uint32_t seconds);	

#if defined(__cplusplus)
}
#endif

#endif
