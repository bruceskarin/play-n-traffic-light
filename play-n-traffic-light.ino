//Play N Traffic Light Task Reminder

//SD and Audio Includes
#include "SD.h"
#include "TMRpcm.h"
#include "SPI.h"

#include <LiquidCrystal.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>

int SD_CardPin = 4;
int audioOutPin = 9;

int pushVal = 0;                           
int val;
int val2;
int addr = 0;

RTC_DS3231 rtc;

// Display Setup
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
int maxScrees / ( exime  c2;= anx  n n/ ezrdero based)


//Push Button and LED Setup
int pushpressed = 0;

const int led1gPin =  30;
const int led1yPin =  31;
const int led1rPin =  32;
const int led2gPin =  33;
const int led2yPin =  34;
const int led2rPin =  35;
const int led3gPin =  36;
const int led3yPin =  37;
const int led3rPin =  38;
const int led4gPin =  39;
const int led4yPin =  40;
const int led4rPin =  41;
const int led5gPin =  42;
const int led5yPin =  43;
const int led5rPin =  44;
const int led6gPin =  45;
const int led6yPin =  46;
const int led6rPin =  47;
const int led7gPin =  48;
const int led7yPin =  49;
const int led7rPin =  50;

long previousMsLed = 0;
const long blinkInterval = 1000;           // interval at which to blink (milliseconds)
unsigned long currentMsLed;

int led1state, led2state, led3state, led4state, led5state, led6state, led7state = LOW;

int push1state, push2state, push3state, push4state, push5state, push6state, push7state, stopinState = 0;          //push button states
int task1flag, task2flag, task3flag, task4flag, task5flag, task6flag, task7flag = false;                          // task flags 
int push1pin = 10;  //Push button pin locations
int push2pin = 22;
int push3pin = 23;
int push4pin = 24;
int push5pin = 25;
int push6pin = 26;
int push7pin = 27;

long previousMsWarn = 0;           
long warnInterval = 5000;         // warning interval in milliseconds
unsigned long currentMsWarn;

//  Task Due Times in 24hr Format
int task1HH = 7;          //    HH - hours 
int task1MM = 30;         //    MM - minutes
int task2HH = 8;          
int task2MM = 30;         
int task3HH = 9;
int task4MM = 0;
int task5HH = 13;
int task5MM = 0;
int task6HH = 16;
int task6MM = 0;
int task7HH = 20;
int task7MM = 0;

int nextTaskHH = task1HH;
int nextTaskMM = task1MM;

int minutesWarning = 5;

int nowHr = 7;
int nowMin = 0;        // for all current time operations

////////
//Setup
////////
void setup() {

  Serial.begin(9600);                      // start serial debugging
  if (! rtc.begin()) {                      // check if rtc is connected 
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
  }
  
  tmrpcm.speakerPin = audioOutPin;
  Serial.begin(9600);
  if (!SD.begin(SD_CardPin)) {
    Serial.println("No SD Card Found");
    return;
  }
  tmrpcm.setVolume(5);
  
//  rtc.adjust(DateTime(2019, 1, 10, 7, 59, 30));              // uncomment to manualy set the RTC datetime

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Play N Traffic");                                 // print a messege at startup
  lcd.setCursor(0, 1);
  lcd.print("Task Reminder");
  delay(5000);
  pinMode(push1pin, INPUT);                                    // define push button pins type
//  pinMode(push2pin, INPUT);
//  pinMode(push3pin, INPUT);
//  pinMode(push4pin, INPUT);
//  pinMode(push5pin, INPUT);
//  pinMode(push6pin, INPUT);
//  pinMode(push7pin, INPUT);
  
  pinMode(led1gPin, OUTPUT);
  pinMode(led1yPin, OUTPUT);
  pinMode(led1rPin, OUTPUT);
//  pinMode(led2gPin, OUTPUT);
//  pinMode(led2yPin, OUTPUT);
//  pinMode(led2rPin, OUTPUT);
//  pinMode(led3gPin, OUTPUT);
//  pinMode(led3yPin, OUTPUT);
//  pinMode(led3rPin, OUTPUT);
//  pinMode(led4gPin, OUTPUT);
//  pinMode(led4yPin, OUTPUT);
//  pinMode(led4rPin, OUTPUT);
//  pinMode(led5gPin, OUTPUT);
//  pinMode(led5yPin, OUTPUT);
//  pinMode(led5rPin, OUTPUT);
//  pinMode(led6gPin, OUTPUT);
//  pinMode(led6yPin, OUTPUT);
//  pinMode(led6rPin, OUTPUT);
//  pinMode(led7gPin, OUTPUT);
//  pinMode(led7yPin, OUTPUT);
//  pinMode(led7rPin, OUTPUT);

}//end of setup

///////////
//Main Loop
///////////

void loop() {
  
  currentMsLCD = millis();                         // set current milliseconds for LCD screen switching time interval
  currentMsLed = millis();
  currentMsWarn = millis();

  push1state = digitalRead(push1pin);          // look for push button state changes
//  push2state = digitalRead(push2pin);
//  push3state = digitalRead(push3pin);
//  push4state = digitalRead(push4pin);
//  push5state = digitalRead(push5pin);
//  push6state = digitalRead(push6pin);
//  push7state = digitalRead(push7pin);

//  DateTime now = rtc.now();                    // get current time
//  nowHr = int(now.hour());
//  nowMin = int(now.minute());


  checkTasks();                                  //check task states
  
  changeScreen();                                // call the screen cycle function

}

////////////////////
//Supporting Functions
///////////////////

void checkTasks() {
  int taskScore = 0;
  int nowMinute = nowHr * 60 + nowMin;
  int taskMinute = 0;
  int activeLed = 0;
  
  if(!task1flag){                                                     //if task not completed (flag is false) continue check
    
    taskMinute = task1HH * 60 + task1MM; //convert time to minutes for easier comparisons
    
    if(nowMinute > taskMinute){                                       //Check for past due task
      taskScore = -1;
      activeLed = led1rPin;
      led1state = blinkLed(activeLed, led1state); //blink red led
      digitalWrite(led1yPin, LOW); //make sure yellow led is off
    }
    else if(nowMinute >= taskMinute - minutesWarning){                //Check past warning
      taskScore = 1;
      activeLed = led1yPin;
      led1state = blinkLed(activeLed, led1state);  //blink yellow led
      digitalWrite(led1gPin, LOW); //make sure green led is off
    }
    else{                                                             //Otherwise task is still green
      taskScore = 2;
      activeLed = led1gPin;
      led1state = blinkLed(activeLed, led1state); //blink green led
    }
   
    if(push1state == 1){ //if button was pressed, close out task
      task1flag = true;
      totalScore += taskScore;
      digitalWrite(activeLed, HIGH); // turn active led on without blink
      nextTaskHH = task2HH;
      nextTaskMM = task2MM;
      newScore(taskScore);
    }
  }// end incomplete task 1 check

//// Replicate for other tasks

}//End of check tasks

int blinkLed(int ledPin, int ledState){
    if (currentMsLed - previousMsLed >= blinkInterval) {
      previousMsLed = currentMsLed;  // save the last time you blinked the LED
      if (ledState == LOW) {  // if the LED is off turn it on and vice-versa:
        ledState = HIGH;
        
      } else {
        ledState = LOW;
      }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
  return ledState;
}

void startWarning() {                    // function to start audio warning for a defined interval

  if (currentMs - previousMs >= interval) {
    previousMs = currentMs;         // save the last time you blinked the LED
   
    Serial.println("Playing warning");
    if(!tmrpcm.isPlaying()){
      tmrpcm.play("warning.wav");
    }
    if (pushpressed == 1) {
    tmrpcm.stopPlayback();
    ledState = LOW;
    digitalWrite(ledPin, ledState);
  
  }
  }
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

  lcd.clear();  
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  lcd.setCursor(6, 0);
  printDigits(nowHr);
  lcd.print(":");
  printDigits(nowMin);
  lcd.setCursor(0, 1);
  lcd.print("Date:");
  lcd.setCursor(6, 1);
  lcd.print("2/2/2021");                        //switch when rtc installed
//  lcd.print(now.day(), DEC);
//  lcd.print("/");
//  lcd.print(now.month(), DEC);
//  lcd.print("/");
//  lcd.print(now.year(), DEC);
}

void reminderScreen(int tHr, int tMin) {              // screen for next task time due
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Next task due at");
    lcd.setCursor(0, 1);
    printDigits(tHr);
    lcd.print(":");
    printDigits(tMin); 
 }

//Screen Cycling
void changeScreen() {                 //function for Screen Cycling
  // Start switching screen every defined intervalLCD
  if (currentMsLCD - previousMsLCD > intervalLCD)             // save the last time you changed the display
  {
    previousMsLCD = currentMsLCD;
    nowMin += 2;
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
