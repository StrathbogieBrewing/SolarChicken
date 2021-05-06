/*
  RTC Library - Demo
 
 This sketch demonstrates the RTC API
 
*/

// include the library code:
#include <RTC.h>



void setup() {
  Serial.begin(19200); 
}

void loop() {

	Serial.println(Clock.getSeconds());
	Serial.println((int)Clock.getTicks());

  	delay(900);
}
