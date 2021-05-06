#include <avr/wdt.h>

#include "RTC.h"

// These constants won't change.  They're used to give names
// to the pins used:
const int kLoadVolts = A0;
const int kBatteryVolts = A1;  
const int kSolarVolts = A2;  

const int kYellowLED = 6;      // the number of the yellow LED pin
const int kGreenLED =  7;      // the number of the green LED pin

const int kLoad =  9;      // the number of the load control
const int kShunt =  10;      // the number of the shunt control

const int kFloatVoltage = 13800;
const int kBoostVoltage = 14200;
const int kBoostReturn = 12400;
const int kSolarThreshold = 3000;

const uint32_t kTwilightSeconds = 30L * 60L;
const uint32_t kRequiredDayLengthSeconds = 15L * 60L * 60L;
const uint32_t kSecondsInDay = 24L * 60L * 60L;
const uint32_t kLogPeriodSeconds = 1L;

int BatteryVoltage = 0;        // value read from the pot
int SolarVoltage = 0;          // value output to the PWM (analog out)
int LoadVoltage = 0;
int setPoint = kBoostVoltage;

int nightTimer = 0;
int dayTimer = 0;

uint32_t secondsFromDawnToDusk = 0;
uint32_t secondsAfterDawn = 0;

void printTimeStamp(uint32_t time);

void setup() 
{
  // initialize serial communications at 9600 bps:
  Serial.begin(19200); 
  Serial.println("*** Chicken Light Timer ***");
  
  MCUCSR &= ~(1 << EXTRF);  // clear the external reset flag
  
  // start the RTC
  Clock.begin(secondsAfterDawn);
  printTimeStamp(secondsAfterDawn);
  Serial.print(" Start Up - ");
  Serial.println(secondsAfterDawn);
  
  // Initialise output pins
  pinMode(kYellowLED, OUTPUT);
  digitalWrite(kYellowLED, LOW);
  pinMode(kGreenLED, OUTPUT);
  digitalWrite(kGreenLED, HIGH);
  pinMode(kLoad, OUTPUT);
  digitalWrite(kLoad, HIGH);    // start with light on for testing purposes
  delay(1000); 
  digitalWrite(kLoad, LOW);
  pinMode(kShunt, OUTPUT);
  digitalWrite(kShunt, LOW);
  
  wdt_enable(WDTO_2S); 
}

void loop() 
{
  // slow the loop down a little
  delay(100); 

  // ** fast reaction code **

  BatteryVoltage = map(analogRead(kBatteryVolts), 0, 1023, 0, 20957);  
  if((BatteryVoltage > setPoint) && (digitalRead(kShunt) == LOW)){
    digitalWrite(kShunt, HIGH);
    digitalWrite(kGreenLED, HIGH); 
  }  
  
  // update charge state
  if(setPoint == kBoostVoltage){
    if(BatteryVoltage > kBoostVoltage){
      setPoint = kFloatVoltage;
      digitalWrite(kYellowLED, HIGH); 
    }
  } else {
    if(BatteryVoltage < kBoostReturn){
        setPoint = kBoostVoltage;
        digitalWrite(kYellowLED, LOW); 
    }
  }

  // check for day rollover
  if(Clock.getSeconds() >= kSecondsInDay) {
    Clock.setSeconds(0);
  }
 
  // check for one second boundary, just return if done
  uint32_t currentTime = Clock.getSeconds();
  
  if(currentTime == secondsAfterDawn) {
    return;
  } else {
    secondsAfterDawn = currentTime;
  }
  
  wdt_reset();
  
  // ** 1 Hz slow reaction code **

  // update charge control output
  if(BatteryVoltage < setPoint) {
    digitalWrite(kShunt, LOW);
    digitalWrite(kGreenLED, LOW);
  }
    
  // determine if dawn event
  SolarVoltage = map(analogRead(kSolarVolts), 0, 1023, 0, 20957);
  
  if(SolarVoltage > kSolarThreshold){
    // might be day time 
    if(dayTimer < kTwilightSeconds){
        dayTimer++;
        if(dayTimer == kTwilightSeconds){
          // reset clock to be referenced to dawn
          secondsAfterDawn = kTwilightSeconds;   
          Clock.setSeconds(secondsAfterDawn);
          nightTimer = 0;   
          // update debug log
          printTimeStamp(secondsAfterDawn); 
          Serial.println("\tDay Detect - Clock Reset");
          // turn light off
          digitalWrite(kLoad, LOW);
        }
     }
  } else {
    // might just be noise
    if((dayTimer > 0) && (dayTimer != kTwilightSeconds)){
      dayTimer--;
    }
  } 
  
  if(SolarVoltage < kSolarThreshold){
    // might be night time 
    if(nightTimer < kTwilightSeconds){
        nightTimer++;
        if(nightTimer == kTwilightSeconds){
          dayTimer = 0;
          secondsFromDawnToDusk = secondsAfterDawn;
          if(secondsFromDawnToDusk < kTwilightSeconds) secondsFromDawnToDusk = kTwilightSeconds;
          secondsFromDawnToDusk -= kTwilightSeconds;
          // update debug log
          printTimeStamp(secondsAfterDawn); 
          Serial.println("\tNight Detect"); 
        }
     }
  } else {
    // might just be noise
    if((nightTimer > 0) && (nightTimer != kTwilightSeconds)){
      nightTimer--;
    }
  }
  
  // end of the artificial day turn light off
  if(secondsAfterDawn == kRequiredDayLengthSeconds){
    printTimeStamp(secondsAfterDawn); 
    Serial.println(" Light Off");
    digitalWrite(kLoad, LOW);
  } 
 
 if(secondsFromDawnToDusk < kRequiredDayLengthSeconds){
  uint32_t secondsAfterDawnToTurnOnLight = secondsFromDawnToDusk - kTwilightSeconds;
  if(secondsAfterDawn == secondsAfterDawnToTurnOnLight){
    printTimeStamp(secondsAfterDawn); 
    Serial.println(" Light On");
    digitalWrite(kLoad, HIGH);
  } 
 }

  digitalWrite(kGreenLED, HIGH);  // device alive flash led

  // debug log to the serial monitor every 10 seconds     
  if(secondsAfterDawn % kLogPeriodSeconds == 0){  
     debug();                        // text dump of status to serial port
  }
    
}

void debug(void)
{
    printTimeStamp(secondsAfterDawn); 
    if(digitalRead(kShunt) == HIGH){
      Serial.print(" Off" ); 
    } else {
      Serial.print(" On" ); 
    }
    if(setPoint == kBoostVoltage){
      Serial.print(" Boost"); 
    } else {
      Serial.print(" Float"); 
    }
    Serial.print(" BatV:" );                       
    Serial.print(BatteryVoltage);      
    Serial.print(" SolV:");      
    Serial.print(SolarVoltage);   
    Serial.print(" NTmr:");      
    Serial.print(nightTimer);
    Serial.print(" DTmr:");      
    Serial.print(dayTimer);    
    Serial.print(" DLen:");      
    Serial.println(secondsFromDawnToDusk);    
}

char* u8toa(char* str, uint8_t value)
{  
    // do 10's
    char digit = '0';
    while(value > 9){
        value -= 10;
        digit++;
    }
    *str++ = digit;

    // do 1's
    digit = '0' + value;
    *str++ = digit;
    *str = '\0';
    
    return str;
}

void printTimeStamp(uint32_t time)
{
   char buffer[16];
   char* str = buffer;
  
   uint32_t whole_minutes = time / 60;  
   uint8_t seconds = (uint8_t)(time - (60 * whole_minutes));           // leftover seconds

   uint32_t whole_hours = whole_minutes / 60;
   uint8_t minutes = (uint8_t)(whole_minutes - (60 * whole_hours));    // leftover minutes

   uint32_t whole_days = whole_hours / 24;
   uint8_t hours = (uint8_t)(whole_hours - (24 * whole_days));         // leftover hours
 
   str = u8toa(str, hours);
   *str++ = ':';
   
   str = u8toa(str, minutes);
   *str++ = ':';
   
   str = u8toa(str, seconds);
   
   Serial.print(buffer);
}
