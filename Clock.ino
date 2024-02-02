//Michael Klements
//The DIY Life
//8 February 2020
//License: CC BY-NC-SA

#include <Adafruit_PWMServoDriver.h>      //Include library for servo driver
#include <RTClib.h>
#include <Wire.h>

Adafruit_PWMServoDriver pwmH = Adafruit_PWMServoDriver(0x40);    //Create an object of Hour driver
Adafruit_PWMServoDriver pwmM = Adafruit_PWMServoDriver(0x41);    //Create an object of Minute driver (A0 Address Jumper)
RTC_DS1307 rtc;

int servoFrequency = 50;      //Set servo operating frequency

int segmentHOff[14] = {130,130,500,450,130,530,500,130,130,500,500,80,500,500};
int segmentMOff[14] = {100,130,500,500,130,480,500,100,70,500,500,80,500,500};
int segmentHOn[14] = {330,330,310,200,330,350,300,360,370,290,250,280,300,300};
int segmentMOn[14] = {280,330,290,300,330,250,315,285,270,300,280,270,320,280};
int hourTens, hourUnits, minuteTens, minuteUnits;


int digits[10][7] = {{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};    //Position values for each digit


int prevHourTens = -1; 
int prevHourUnits = -1;
int prevMinuteTens = -1;
int prevMinuteUnits = -1;


int midOffset = 500;            //Amount by which adjacent segments to mid move away when required

void setup() 
{
  Serial.begin(9600);
  rtc.begin();
  if (! rtc.begin()) {
  Serial.println("Couldn't find RTC");
  while (1);
}
  rtc.adjust(DateTime(2023, 10, 3, 22, 30, 0));

  pwmH.begin();   //Start each board
  pwmM.begin();
  pwmH.setOscillatorFrequency(27000000);    //Set the PWM oscillator frequency, used for fine calibration
  pwmM.setOscillatorFrequency(27000000);
  pwmH.setPWMFreq(servoFrequency);          //Set the servo operating frequency
  pwmM.setPWMFreq(servoFrequency);

  
  for(int i=0 ; i<=13 ; i++)    //Set all of the servos to on or up (88:88 displayed)
  {
    pwmH.setPWM(i, 0, segmentHOn[i]);
    delay(10);
    pwmM.setPWM(i, 0, segmentMOn[i]);
    delay(10);
  }
  delay(2000);
}

void loop()
{
  DateTime now = rtc.now();

  int hour = now.hour();
  int minute = now.minute();
  
  int hourTens = hour / 10;
  int hourUnits = hour % 10;
  
  int minuteTens = minute / 10;
  int minuteUnits = minute % 10;
  
  if(minuteUnits != prevMinuteUnits || minuteTens != prevMinuteTens || hourUnits != prevHourUnits || hourTens != prevHourTens)
    if(digits[minuteUnits][6]!=digits[prevMinuteUnits][6])      //Move adjacent segments for Minute units
  {
    if(digits[prevMinuteUnits][2]==1)
      pwmM.setPWM(2, 0, segmentMOn[2]+midOffset);
    if(digits[prevMinuteUnits][6]==1)
      pwmM.setPWM(4, 0, segmentMOn[4]-midOffset);
  }
  delay(100);                                                 //Delay allows adjacent segments to move before moving middle
  if(digits[minuteUnits][6]==1)                               //Move Minute units middle segment if required
    pwmM.setPWM(6, 0, segmentMOn[6]);
  else
    pwmM.setPWM(6, 0, segmentMOff[6]);
  if(digits[minuteTens][6]!=digits[prevMinuteTens][6])        //Move adjacent segments for Minute tens
  {
    if(digits[prevMinuteTens][2]==1)
      pwmM.setPWM(9, 0, segmentMOn[9]+midOffset);
    if(digits[prevMinuteTens][6]==1)
      pwmM.setPWM(11, 0, segmentMOn[11]-midOffset);
  }
  delay(100);                                                 //Delay allows adjacent segments to move before moving middle
  if(digits[minuteTens][6]==1)                                //Move Minute tens middle segment if required
    pwmM.setPWM(13, 0, segmentMOn[13]);
  else
    pwmM.setPWM(13, 0, segmentMOff[13]);
  if(digits[hourUnits][6]!=digits[prevHourUnits][6])          //Move adjacent segments for Hour units
  {
    if(digits[prevMinuteUnits][2]==1)
      pwmM.setPWM(2, 0, segmentMOn[2]+midOffset);
    if(digits[prevMinuteUnits][6]==1)
      pwmM.setPWM(4, 0, segmentMOn[4]-midOffset);
  }
  delay(100);                                                 //Delay allows adjacent segments to move before moving middle
  if(digits[hourUnits][6]==1)                                 //Move Hour units middle segment if required
    pwmH.setPWM(6, 0, segmentHOn[6]);
  else
    pwmH.setPWM(6, 0, segmentHOff[6]);
  if(digits[hourTens][6]!=digits[prevHourTens][6])            //Move adjacent segments for Hour tens
  {
    if(digits[prevMinuteTens][2]==1)
      pwmM.setPWM(9, 0, segmentMOn[9]+midOffset);
    if(digits[prevMinuteTens][6]==1)
      pwmM.setPWM(11, 0, segmentMOn[11]-midOffset);
  }
  delay(100);                                                 //Delay allows adjacent segments to move before moving middle
  if(digits[hourTens][6]==1)                                  //Move Hour tens middle segment if required
    pwmH.setPWM(13, 0, segmentHOn[13]);
  else
    pwmH.setPWM(13, 0, segmentHOff[13]);



    for (int i=0 ; i<=5 ; i++)                        //Move the remaining segments
    {
    if(digits[hourTens][i]==1)                      //Update the hour tens
      pwmH.setPWM(i+7, 0, segmentHOn[i+7]);
    else
      pwmH.setPWM(i+7, 0, segmentHOff[i+7]);
    delay(10);
    if(digits[hourUnits][i]==1)                     //Update the hour units
      pwmH.setPWM(i, 0, segmentHOn[i]);
    else
      pwmH.setPWM(i, 0, segmentHOff[i]);
    delay(10);
    if(digits[minuteTens][i]==1)                    //Update the minute tens
      pwmM.setPWM(i+7, 0, segmentMOn[i+7]);
    else
      pwmM.setPWM(i+7, 0, segmentMOff[i+7]);
    delay(10);
    if(digits[minuteUnits][i]==1)                   //Update the minute units
      pwmM.setPWM(i, 0, segmentMOn[i]);
    else
      pwmM.setPWM(i, 0, segmentMOff[i]);
    delay(10);
  }

  Serial.println(hourTens); 
  Serial.println(hourUnits);
  Serial.println(minuteTens); 
  Serial.println(minuteUnits); 
  Serial.println("");

  prevHourTens = hourTens;           //Update previous displayed numerals
  prevHourUnits = hourUnits;
  prevMinuteTens = minuteTens;
  prevMinuteUnits = minuteUnits;

  delay(500);
}
