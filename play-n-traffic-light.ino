//Play N Traffic Light Task Reminder

//SD and Audio Includes
#include "SD.h"
#include "TMRpcm.h"
#include "SPI.h"
#include <avr/power.h>
#include <avr/sleep.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include "task.h"


//SD Card and Audio Parameters
TMRpcm audio;

int SD_CardPin = 53;
int audioOutPin = 6;
char warnWave[] = "warning.wav";
char startupWave[] = "startup.wav";
char villagerWave[] = "Villager.wav";
char tromboneWave[] = "trombone.wav";
char victoryWave[] = "victory.wav";
char sleepWave[] = "woods.wav";

// Display Parameters
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;  // lcd pins
const int lcdBacklight = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //connect LCD

bool isScreenChanged = true;
unsigned long previousMsLCD = 0;       // for LCD screen update
unsigned long intervalLCD = 5000;      // screen cycling interval in milliseconds
unsigned long currentMsLCD;

int totalScore = 0; 

//Display Screens
#define SCORE_SCREEN 0                                           
#define DATETIME_SCREEN 1
#define REMINDER_SCREEN 2

int screens = 0;      // start screen
int maxScreen = 2;   // number of screens (zero based count)

//Tasks
//task(int taskNum, int taskHour, int taskMin, int greenPin, int yellowPin, int redPin, int pushPin, int minWarn)
    //task(tn, th, tm, gp, yp, rp, pp, mw)
task task1( 1,  7, 25, 30, 31, 32, 22, 10);
task task2( 2,  7, 40, 48, 49,  8, 23, 10);
task task3( 3,  7, 45, 33, 34, 35, 24, 10);
task task4( 4, 15, 50, 36, 37, 38, 25, 10);
task task5( 5, 18,  0, 39, 40, 41, 26, 10);
task task6( 6, 20, 25, 42, 43, 44, 27, 10);
task task7( 7, 20, 30, 45, 46, 47, 28, 10);

int nextTaskHH = task1.getTaskHour();
int nextTaskMM = task1.getTaskMinute();

//Real Time Clock
RTC_DS3231 rtc;
#define clock_interupt_pin 19

bool demoMode = false;
unsigned long currentMs;
unsigned long previousMs = 0;
unsigned long tickInterval = 1000;

int scoreReportMinutes = 10;
int sleepHr = 20;
int sleepMin = 40;
int wakeHr = 7;
int wakeMin = 15;

DateTime now;
int nowHr = 7;
int nowMin = 0;        // for all current time operations

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Setup
///////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  Serial.begin(9600);                       // start serial debugging
  Serial.println("Begin setup");
  if (rtc.begin()){                         // check if rtc is connected
    if(rtc.lostPower()) {
        Serial.println("Power lost setting clock");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    //we don't need the 32K Pin, so disable it
    rtc.disable32K();
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Comment after setting
  }
  else {                       
    Serial.println("Couldn't find RTC");
    abort();
  }
  
  audio.speakerPin = audioOutPin;
  
  if(SD.begin(SD_CardPin)){
    Serial.println("SD card ready");
  }
  else {
    Serial.println("No SD card found");
    return;
  }
  pinMode(lcdBacklight, OUTPUT);
  digitalWrite(lcdBacklight, HIGH);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Play N Traffic");                 // print a messege at startup
  lcd.setCursor(0, 1);
  lcd.print("Task Reminder");
  
  audio.setVolume(5);
  audio.play(startupWave);
  
  delay(5000);
  
  now = rtc.now(); 
  if(demoMode){
    now = DateTime(now.year(), now.month(), now.day(), 7, 0);
  }
  

  //RTC interup setup
  pinMode(clock_interupt_pin, INPUT_PULLUP);  
  attachInterrupt(digitalPinToInterrupt(clock_interupt_pin), onAlarm, FALLING);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.disableAlarm(2);
  
  //Set Alarm Hour
  DateTime alarm0 = DateTime (0, 0, 0, wakeHr, wakeMin); 
  rtc.writeSqwPinMode(DS3231_OFF);
  rtc.setAlarm1(alarm0, DS3231_A1_Hour);
  Serial.println("Alarm set for:");
  char timeStamp[] = "hh:mm AP";
  Serial.println(alarm0.toString(timeStamp));
   
}//end of setup

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Main Loop
///////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  
  // get current time
  if(demoMode){
      now = now + tick();
  } else now = rtc.now();
  
  nowHr = int(now.hour());
  nowMin = int(now.minute());

  int nowMinutes = nowHr * 60 + nowMin;
  int sleepMinutes = sleepHr * 60 + sleepMin;

  checkTasks(nowMinutes);
  
  if(nowMinutes >= sleepMinutes){
    enterSleep();
  }
  
  changeScreen();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Supporting Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////

void checkTasks(int nowMinute) {
  
  if(!task1.taskFlag){             //task 1
    if(task1.checkTask(nowMinute)){
        startWarning();
      }
    if(task1.pushState & !task1.taskFlag){
      task1.taskFlag = true;
            totalScore += task1.taskScore;
      nextTaskHH = task2.getTaskHour();
      nextTaskMM = task2.getTaskMinute();
      newScore(task1.taskScore);
    }  
  }//end task 1 check

  if(!task2.taskFlag){             //task 2
    if(task2.checkTask(nowMinute)){
        startWarning();
      }
    if(task2.pushState & !task1.taskFlag){
      task2.taskFlag = true;
            totalScore += task2.taskScore;
      nextTaskHH = task3.getTaskHour();
      nextTaskMM = task3.getTaskMinute();
      newScore(task2.taskScore);
    }  
  }//end task 2 check

    if(!task2.taskFlag){             //task 2
    if(task2.checkTask(nowMinute)){
        startWarning();
      }
    if(task2.pushState & !task2.taskFlag){
      task2.taskFlag = true;
            totalScore += task2.taskScore;
      nextTaskHH = task3.getTaskHour();
      nextTaskMM = task3.getTaskMinute();
      newScore(task2.taskScore);
    }  
  }//end task 2 check

  if(!task3.taskFlag){             //task 3
    if(task3.checkTask(nowMinute)){
        startWarning();
      }
    if(task3.pushState & !task3.taskFlag){
      task3.taskFlag = true;
            totalScore += task3.taskScore;
      nextTaskHH = task4.getTaskHour();
      nextTaskMM = task4.getTaskMinute();
      newScore(task3.taskScore);
    }  
  }//end task 3 check

  if(!task4.taskFlag){             //task 4
    if(task4.checkTask(nowMinute)){
        startWarning();
      }
    if(task4.pushState & !task4.taskFlag){
      task4.taskFlag = true;
            totalScore += task4.taskScore;
      nextTaskHH = task5.getTaskHour();
      nextTaskMM = task5.getTaskMinute();
      newScore(task4.taskScore);
    }  
  }//end task 4 check

  if(!task5.taskFlag){             //task 5
    if(task5.checkTask(nowMinute)){
        startWarning();
      }
    if(task5.pushState & !task5.taskFlag){
      task5.taskFlag = true;
            totalScore += task5.taskScore;
      nextTaskHH = task6.getTaskHour();
      nextTaskMM = task6.getTaskMinute();
      newScore(task5.taskScore);
    }  
  }//end task 5 check

  if(!task6.taskFlag){             //task 6
    if(task6.checkTask(nowMinute)){
        startWarning();
      }
    if(task6.pushState & !task6.taskFlag){
      task6.taskFlag = true;
            totalScore += task6.taskScore;
      nextTaskHH = task7.getTaskHour();
      nextTaskMM = task7.getTaskMinute();
      newScore(task6.taskScore);
    }  
  }//end task 6 check

  if(!task7.taskFlag){             //task 7
    if(task7.checkTask(nowMinute)){
        Serial.println("warning recieved");
        startWarning();
      }
    if(task7.pushState & !task7.taskFlag){
      task7.taskFlag = true;
            totalScore += task7.taskScore;
      nextTaskHH = task1.getTaskHour();
      nextTaskMM = task1.getTaskMinute();
      newScore(task7.taskScore);
    }  
  }//end task 7 check

}//end check tasks


void startWarning() {
  audio.play(warnWave);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Screen Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////

void newScore(int points){               // print score added message
    lcd.clear();
    lcd.setCursor(0, 0);
    if(points > 0)
      lcd.print("Nice, you scored");
    else lcd.print("Bummer, you got");
    lcd.setCursor(0, 1);
    lcd.print(points, DEC);
    lcd.print(" points");
    previousMsLCD = currentMsLCD;
    if (points > 1)
      audio.play(victoryWave);
    else if (points > 0)
      audio.play(villagerWave);
    else audio.play(tromboneWave);
}

// print the day's total score
void scoreScreen(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Today's Score:");     
    lcd.setCursor(0, 1);
    lcd.print(totalScore, DEC);    
}

//clock display helper
void printDigits(byte digits){      
    if(digits < 10)
        lcd.print('0');
    lcd.print(digits);
}

//print current datetime  
void datetimeScreen() {       
  lcd.clear();  
  lcd.setCursor(0, 0);
  char monthDayYear[] = "DDD MMM DD, YYYY";
  lcd.print(now.toString(monthDayYear));
  lcd.setCursor(0, 1);
  char timeStamp[] = "hh:mm AP";
  lcd.print(now.toString(timeStamp));
}

// screen for next task time due
void reminderScreen(int tHr, int tMin) {
    int hr_12;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Next task due at");
    lcd.setCursor(0, 1);
      if (tHr==0 | tHr==12)
    hr_12=12;
   else hr_12=tHr%12;
    printDigits(hr_12);
    lcd.print(":");
    printDigits(tMin);
    if (tHr<12) lcd.print(" AM");
    else lcd.print(" PM"); 
 }

//Screen Cycling
void changeScreen() {
  //get the current milliseconds
  currentMsLCD = millis();                        
  
  // Start switching screen every defined intervalLCD
  if (currentMsLCD - previousMsLCD > intervalLCD)
  {
    previousMsLCD = currentMsLCD;
    
    screens++;
    if (screens > maxScreen) {
      screens = 0;  // all screens over -> start from 1st
    }
    isScreenChanged = true;
  }

  // Start displaying current screen
  if (isScreenChanged)   // only update the screen if the screen is changed.
  {
    isScreenChanged = false; // reset for next iteration
    switch (screens)
    {
      case SCORE_SCREEN:
        scoreScreen();                
        break;
      case DATETIME_SCREEN:              
        datetimeScreen();              
        break;
      case REMINDER_SCREEN:
        reminderScreen(nextTaskHH, nextTaskMM);                
        break;
      default:
        //NOT SET.
        break;
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
// Time and Sleep Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////
TimeSpan tick(){
  //get the current milliseconds
  currentMs = millis();
  TimeSpan oneMinute = TimeSpan(0);
  
  if (currentMs - previousMs >= tickInterval) {
    previousMs = currentMs;
    oneMinute = TimeSpan(60);
    DateTime tmp = now + oneMinute; 
    char timeStamp[] = "hh:mm AP";
    //Serial.println(tmp.toString(timeStamp));
 }
 return oneMinute;

}

void onAlarm() {
    Serial.println("Waking from alarm");
    sleep_disable(); // Disable sleep mode
    digitalWrite(lcdBacklight, HIGH);
    Serial.println("Detaching interrupt");
    detachInterrupt(digitalPinToInterrupt(clock_interupt_pin));
}

void enterSleep(){
  //audio.play(sleepWave);
  totalScore += task1.goodNight();
  totalScore += task2.goodNight();
  totalScore += task3.goodNight();
  totalScore += task4.goodNight();
  totalScore += task5.goodNight();
  totalScore += task6.goodNight();
  totalScore += task7.goodNight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Final Score:");     
  lcd.setCursor(0, 1);
  lcd.print(String(totalScore));

  delay(scoreReportMinutes * 60000);

  //close out the day and turn off lights
  totalScore = 0;

  digitalWrite(lcdBacklight, LOW);
  
  sleep_enable();                       // Enabling sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Setting the sleep mode, in this case full sleep
  
  Serial.println("Going to sleep");    // Print message to serial monitor
  Serial.flush();                       // Ensure all characters are sent to the serial monitor
  
  interrupts();                         // Allow interrupts again
  sleep_cpu();                          // Enter sleep mode

  //wakeup should resume here
  Serial.println("Resuming code in one minute");
  delay(61000);
  Serial.println("Re-attaching interrupt");
  attachInterrupt(digitalPinToInterrupt(clock_interupt_pin), onAlarm, FALLING);
  task1.goodMorning();
  task2.goodMorning();
  task3.goodMorning();
  task4.goodMorning();
  task5.goodMorning();
  task6.goodMorning();
  task7.goodMorning();
  if(demoMode){
     now = DateTime(now.year(), now.month(), now.day(), 7, 0);
  }
  audio.setVolume(2);
  audio.play(startupWave);
  audio.setVolume(5);
 }
