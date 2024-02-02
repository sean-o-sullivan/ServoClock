#include <Adafruit_PWMServoDriver.h>  // Include library for the servo driver
#include <RTClib.h>                   // Include library for the Real Time Clock
#include <Wire.h>                     // Include Wire library for I2C communication
#include <TimeLib.h>                  // Include library for time functions
#include <Timezone.h>                 // Include library for timezone functions

// Define TimeChangeRule for Western European Time (WET) and Western European Summer Time (WEST)
TimeChangeRule wET = {"WET", Last, Sun, Oct, 2, 0}; // UTC+0
TimeChangeRule wEST = {"WEST", Last, Sun, Mar, 1, 60}; // UTC+1

// Initialize timezone for Ireland using the defined rules
Timezone ireland(wET, wEST);

// Create objects for the servo drivers for hours and minutes
Adafruit_PWMServoDriver pwmH = Adafruit_PWMServoDriver(0x40);  // Hour driver on I2C address 0x40
Adafruit_PWMServoDriver pwmM = Adafruit_PWMServoDriver(0x41);  // Minute driver on I2C address 0x41 (A0 Address Jumper)

// Initialize the Real Time Clock (RTC)
RTC_DS1307 rtc;

// Define the operating frequency for the servos
int servoFrequency = 50;

// Define the off and on positions for the hour and minute segments
int segmentHOff[14] = {130,130,500,450,130,530,500,100,130,500,460,75,500,500};
int segmentMOff[14] = {100,130,500,490,130,480,500,100,70,500,485,75,500,500};
int segmentHOn[14] = {330,330,310,200,330,350,300,360,370,290,250,280,300,300};
int segmentMOn[14] = {280,330,290,300,330,250,315,285,270,300,280,287,277,280};

// Variables to store the current time in tens and units
int hourTens, hourUnits, minuteTens, minuteUnits;

// Define the digit segment states for a 7-segment display (0-9)
int digits[10][7] = {
  {1,1,1,1,1,1,0},
  {0,1,1,0,0,0,0},
  {1,1,0,1,1,0,1},
  {1,1,1,1,0,0,1},
  {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1},
  {1,0,1,1,1,1,1},
  {1,1,1,0,0,0,0},
  {1,1,1,1,1,1,1},
  {1,1,1,1,0,1,1}
};

// Variables to store the previous time segments
int prevHourTens = -1; 
int prevHourUnits = -1;
int prevMinuteTens = -1;
int prevMinuteUnits = -1;

// Offset for adjacent segments to move away from the middle when needed
int midOffset = 50;

void setup() {
  Serial.begin(9600);

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Initialize the PWM servo drivers
  pwmH.begin();
  pwmM.begin();
  pwmH.setOscillatorFrequency(27000000);  // Set the PWM oscillator frequency for fine calibration
  pwmM.setOscillatorFrequency(27000000);
  pwmH.setPWMFreq(servoFrequency);        // Set the servo operating frequency
  pwmM.setPWMFreq(servoFrequency);

  // Initialize the servo positions for the hour and minute displays to 88:88
  for (int i = 0; i <= 13; i++) {
    pwmM.setPWM(i, 0, segmentMOff[i]);
    delay(20);
  }
  for (int i = 0; i <= 13; i++) {
    pwmH.setPWM(i, 0, segmentHOff[i]);
    delay(20);
  }
  for (int i = 13; i >= 0; i--) {
    pwmM.setPWM(i, 0, segmentMOn[i]);
    delay(20);
  }
  for (int i = 13; i >= 0; i--) {
    pwmH.setPWM(i, 0, segmentHOn[i]);
    delay(20);
  }

  // Set the previous time segments to 88:88
  prevHourTens = 8; 
  prevHourUnits = 8;
  prevMinuteTens = 8;
  prevMinuteUnits = 8;
}

// Function to calculate the number of days until Christmas
int daysUntilChristmas() {
  DateTime now = rtc.now();
  DateTime christmas(now.year(), 12, 25, 0, 0, 0);
  if (now.month() == 12 && now.day() > 25) {
    // If today is after Christmas, set the target to next year's Christmas
    christmas = DateTime(now.year() + 1, 12, 25, 0, 0, 0);
  }
  TimeSpan timeUntilChristmas = christmas - now;
  return timeUntilChristmas.days();
}

// Function to update the display with the current time or countdown
void updateDisplay(int tens, int units, Adafruit_PWMServoDriver& pwm, int segmentOn[], int segmentOff[]) {
  if (digits[units][6] != digits[prevMinuteUnits][6]) {
    if (digits[prevMinuteUnits][2] == 1)
      pwm.setPWM(2, 0, segmentOn[2] + midOffset);
    if (digits[prevMinuteUnits][6] == 1)
      pwm.setPWM(4, 0, segmentOn[4] - midOffset);
  }
  delay(100);
  if (digits[units][6] == 1)
    pwm.setPWM(6, 0, segmentOn[6]);
  else
    pwm.setPWM(6, 0, segmentOff[6]);
  if (digits[tens][6] != digits[prevMinuteTens][6]) {
    if (digits[prevMinuteTens][2] == 1)
      pwm.setPWM(9, 0, segmentOn[9] + midOffset);
    if (digits[prevMinuteTens][6] == 1)
      pwm.setPWM(11, 0, segmentOn[11] - midOffset);
  }
  delay(100);
  if (digits[tens][6] == 1)
    pwm.setPWM(13, 0, segmentOn[13]);
  else
    pwm.setPWM(13, 0, segmentOff[13]);

  for (int i = 0; i <= 5; i++) {
    if (digits[tens][i] == 1)
      pwm.setPWM(i + 7, 0, segmentOn[i + 7]);
    else
      pwm.setPWM(i + 7, 0, segmentOff[i + 7]);
    delay(10);
    if (digits[units][i] == 1)
      pwm.setPWM(i, 0, segmentOn[i]);
    else
      pwm.setPWM(i, 0, segmentOff[i]);
    delay(10);
  }
}

void loop() {
  DateTime now = rtc.now();
  time_t utc = now.unixtime();         // Convert the DateTime to time_t format
  time_t local = ireland.toLocal(utc); // Convert UTC to Ireland local time using Timezone library

  // Calculate days until Christmas
  int daysToChristmas = daysUntilChristmas();

  if (daysToChristmas <= 10) {
    // Display countdown to Christmas
    int countdownTens = daysToChristmas / 10;
    int countdownUnits = daysToChristmas % 10;
    updateDisplay(countdownTens, countdownUnits, pwmH, segmentHOn, segmentHOff);
    updateDisplay(countdownUnits, countdownTens, pwmM, segmentMOn, segmentMOff);
  } else {
    // Display current time
    int currentHour = hour(local);
    int currentMinute = minute(local);

    int hourTens = currentHour / 10;
    int hourUnits = currentHour % 10;

    int minuteTens = currentMinute / 10;
    int minuteUnits = currentMinute % 10;

    updateDisplay(hourTens, hourUnits, pwmH, segmentHOn, segmentHOff);
    updateDisplay(minuteTens, minuteUnits, pwmM, segmentMOn, segmentMOff);

    // Update previous time segments
    prevHourTens = hourTens;
    prevHourUnits = hourUnits;
    prevMinuteTens = minuteTens;
    prevMinuteUnits = minuteUnits;
  }

  delay(500);
}
