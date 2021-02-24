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

long previousMsWarn = 0;           
long warnInterval = 10000;         // warning interval in milliseconds
unsigned long currentMsWarn;

//Real Time Clock
RTC_DS3231 rtc;
#define clock_interupt_pin 19

// Display Parameters
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;  // lcd pins
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //connect LCD

bool isScreenChanged = true;
long previousMsLCD = 0;       // for LCD screen update
long intervalLCD = 5000;      // screen cycling interval in milliseconds
unsigned long currentMsLCD;

int totalScore = 0; 

//Display Screens
#define SCORE_SCREEN 0                                           
#define DATETIME_SCREEN 1
#define REMINDER_SCREEN 2

int screens = 0;      // start screen
int maxScreen = 2;   // number of screens (zero based count)


//Warning parameters
int pushpressed = 0;
//Tasks
//task(int taskNum, int taskHour, int taskMin, int greenPin, int yellowPin, int redPin, int pushPin, int minWarn)
    //task(tn, th, tm, gp, yp, rp, pp, mw)
task task1( 1, 17, 30, 30, 31, 32, 22, 10);
task task2( 2, 18, 30, 48, 49,  8, 23, 10);
task task3( 3, 19, 00, 33, 34, 35, 24, 10);
task task4( 4, 22, 30, 36, 37, 38, 25,  5);
task task5( 5, 22, 40, 39, 40, 41, 26, 10);
task task6( 6, 22, 50, 42, 43, 44, 27, 10);
task task7( 7, 23, 04, 45, 46, 47, 28,  5);

int nextTaskHH = task1.getTaskHour();
int nextTaskMM = task1.getTaskMinute();

int sleepHr = 22;
int sleepMin = 25;
int wakeHr = 22;
int wakeMin = 27;

DateTime now;
int nowHr;
int nowMin;        // for all current time operations

////////
//Setup
////////
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
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Play N Traffic");                 // print a messege at startup
  lcd.setCursor(0, 1);
  lcd.print("Task Reminder");

  audio.setVolume(5);
  audio.play(startupWave);

  pinMode(clock_interupt_pin, INPUT_PULLUP);  //RTC interup setup
  attachInterrupt(digitalPinToInterrupt(clock_interupt_pin), onAlarm, FALLING);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.disableAlarm(2); 
  DateTime alarm0 = DateTime (0, 0, 0, wakeHr, wakeMin); //Set Alarm Hour
  rtc.writeSqwPinMode(DS3231_OFF);
  rtc.setAlarm1(alarm0, DS3231_A1_Hour);
  Serial.println("Alarm set for: ");
  Serial.println(wakeHr);
   
}//end of setup

///////////
//Main Loop
///////////

void loop() {
  
  currentMsLCD = millis();                         // set current milliseconds for LCD screen switching time interval
  currentMsWarn = millis();


  now = rtc.now();                    // get current time
  nowHr = int(now.hour());
  nowMin = int(now.minute());

  if(nowHr > sleepHr & nowMin >= sleepMin){
    enterSleep();
  }

  checkTasks();                                  //check task states
  
  changeScreen();                                // call the screen cycle function

}

////////////////////
//Supporting Functions
///////////////////

void checkTasks() {

  int nowMinute = nowHr * 60 + nowMin;
  
  if(!task1.taskFlag){             //task 1
    if(task1.checkTask(nowMinute)){
        startWarning();
      }
    if(task1.pushState & !task1.taskFlag){
      task1.taskFlag = true;
      pushpressed = 1;
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
      pushpressed = 1;
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
      pushpressed = 1;
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
      pushpressed = 1;
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
      pushpressed = 1;
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
      pushpressed = 1;
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
      pushpressed = 1;
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
      pushpressed = 1;
      totalScore += task7.taskScore;
      nextTaskHH = task1.getTaskHour();
      nextTaskMM = task1.getTaskMinute();
      newScore(task7.taskScore);
    }  
  }//end task 7 check

}//end check tasks


void startWarning() {                    // function to start audio warning for a defined interval
  audio.play(warnWave);
}


///////////////////
//Screen Functions
///////////////////

void newScore(int points){               // print score added message
    lcd.clear();
    lcd.setCursor(0, 0);
    if(points > 0)
      lcd.print("Nice, you scored");
    else lcd.print("Bummer, you got");
    lcd.setCursor(0, 1);
    lcd.print(points, DEC);
    lcd.print(" points");
    previousMsLCD = currentMsLCD;     //reset screen cycle time
    if (points > 1)
      audio.play(victoryWave);
    else if (points > 0)
      audio.play(villagerWave);
    else audio.play(tromboneWave);
}

void scoreScreen(){               // print the day's total score
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Today's Score:");     
    lcd.setCursor(0, 1);
    lcd.print(totalScore, DEC);    
}


void printDigits(byte digits){      //clock display helper
    if(digits < 10)
        lcd.print('0');
    lcd.print(digits);
}

void datetimeScreen() {     //print current datetime         
  int hr_12;
  lcd.clear();  
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  lcd.setCursor(6, 0);
  if (nowHr==0)
    hr_12=12;
   else hr_12=nowHr%12;
  printDigits(hr_12);
  lcd.print(":");
  printDigits(nowMin);
  if (nowHr<12) lcd.print(" AM");
  else lcd.print(" PM");
  lcd.setCursor(0, 1);
  lcd.print("Date:");
  lcd.setCursor(6, 1);
  lcd.print(now.month(), DEC);
  lcd.print("/");
  lcd.print(now.day(), DEC);
  lcd.print("/");
  lcd.print(now.year(), DEC);
}

void reminderScreen(int tHr, int tMin) {              // screen for next task time due
    int hr_12;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Next task due at");
    lcd.setCursor(0, 1);
      if (tHr==0)
    hr_12=12;
   else hr_12=tHr%12;
    printDigits(hr_12);
    lcd.print(":");
    printDigits(tMin);
    if (tHr<12) lcd.print(" AM");
    else lcd.print(" PM"); 
 }

//Screen Cycling
void changeScreen() {                 //function for Screen Cycling
  // Start switching screen every defined intervalLCD
  if (currentMsLCD - previousMsLCD > intervalLCD)             // save the last time you changed the display
  {
    previousMsLCD = currentMsLCD;
    //nowMin += 2;                                      //for accelerated time testing
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

void onAlarm() {
    Serial.println("Waking from alarm");
    sleep_disable(); // Disable sleep mode
}

void enterSleep(){
  sleep_enable();                       // Enabling sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Setting the sleep mode, in this case full sleep
  
  Serial.println("Going to sleep");    // Print message to serial monitor
  Serial.flush();                       // Ensure all characters are sent to the serial monitor
  
  interrupts();                         // Allow interrupts again
  sleep_cpu();                          // Enter sleep mode

  /* The program will continue from here when it wakes */
  audio.play(startupWave);
 }
