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
int audioPowerPin = 17;
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
int numTasks = 7;
Task** tasks = new Task*[numTasks];

int nextTaskHH;
int nextTaskMM;

//Real Time Clock
RTC_DS3231 rtc;
#define clock_interupt_pin 19

bool demoMode = false;
unsigned long currentMs;
unsigned long previousMs = 0;
unsigned long tickInterval = 250;

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

  //tn taskNum, th taskHour, tm taskMin, gp greenPin, yp yellowPin, rp redPin, pp pushPin, mw minWarn 
  //task(tn, th, tm, gp, yp, rp, pp, mw)
  tasks[0] = new Task( 1,  7, 30, 30, 31, 32, 22, 10);
  tasks[1] = new Task( 2,  7, 40, 48, 49,  8, 23, 10);
  tasks[2] = new Task( 3,  7, 45, 33, 34, 35, 24, 10);
  tasks[3] = new Task( 4, 15, 50, 36, 37, 38, 25, 10);
  tasks[4] = new Task( 5, 18,  0, 39, 40, 41, 26, 10);
  tasks[5] = new Task( 6, 20, 25, 42, 43, 44, 27, 10);
  tasks[6] = new Task( 7, 20, 30, 45, 46, 47, 28, 10);

  nextTaskHH = tasks[0]->getTaskHour();
  nextTaskMM = tasks[0]->getTaskMinute();

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
  pinMode(audioPowerPin, OUTPUT);
  pinMode(lcdBacklight, OUTPUT);
  digitalWrite(lcdBacklight, HIGH);
  digitalWrite(audioPowerPin, HIGH);
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

  setAlarm();
   
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
  int wakeMinutes = wakeHr * 60 + wakeMin;

  checkTasks(nowMinutes);
  
  if(nowMinutes >= sleepMinutes | nowMinutes < wakeMinutes){
    enterSleep();
  }
  
  changeScreen();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Supporting Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////

void checkTasks(int nowMinute) {
  for(int i = 0; i < numTasks; i++){
    if(!tasks[i]->taskFlag){             //check task if not flagged
      if(tasks[i]->checkTask(nowMinute)){
          startWarning();
        }
      if(tasks[i]->pushState & !tasks[i]->taskFlag){
        tasks[i]->taskFlag = true;
        totalScore += tasks[i]->taskScore;
        int nextTask = i+1;
        if(nextTask >= numTasks) nextTask = 0;
        nextTaskHH = tasks[nextTask]->getTaskHour();
        nextTaskMM = tasks[nextTask]->getTaskMinute();
        newScore(tasks[i]->taskScore);
      }  
    }//end task check
}//end for tasks loop

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
    digitalWrite(audioPowerPin, HIGH);
    Serial.println("Detaching interrupt");
    detachInterrupt(digitalPinToInterrupt(clock_interupt_pin));
}

void enterSleep(){
  //audio.play(sleepWave);
  for(int i = 0; i < numTasks; i++){
    totalScore += tasks[i]->goodNight();
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Final Score:");     
  lcd.setCursor(0, 1);
  lcd.print(String(totalScore));

  long msMin = 60000;
  if(demoMode) msMin = 1000;
  long delayMs = scoreReportMinutes * msMin;
  Serial.println(String(delayMs));
  delay(delayMs);

  //close out the day and turn off lights
  totalScore = 0;

  digitalWrite(audioPowerPin, LOW);
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
  setAlarm();

  for(int i = 0; i < numTasks; i++){
    tasks[i]->goodMorning();
  }
  if(demoMode){
     now = DateTime(now.year(), now.month(), now.day(), 7, 0);
  }
  audio.setVolume(1);
  audio.play(startupWave);
  audio.setVolume(5);
 }

void setAlarm(){
    attachInterrupt(digitalPinToInterrupt(clock_interupt_pin), onAlarm, FALLING);
    rtc.clearAlarm(1);
    rtc.clearAlarm(2);
    rtc.disableAlarm(2);
    
    //Set Alarm Hour
    DateTime alarm0 = DateTime (0, 0, 0, wakeHr, wakeMin); 
    if(demoMode){
      alarm0 = rtc.now() + TimeSpan(60 * 5);
    }
    rtc.writeSqwPinMode(DS3231_OFF);
    rtc.setAlarm1(alarm0, DS3231_A1_Hour);
    Serial.println("Alarm set for:");
    char timeStamp[] = "hh:mm AP";
    Serial.println(alarm0.toString(timeStamp));
}
