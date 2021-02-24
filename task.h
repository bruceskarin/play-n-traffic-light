class task{
    public:
        int taskScore;
        bool taskFlag;
        int pushState;
        task(int tn, int th, int tm, int gp, int yp, int rp, int pp, int mw){
            taskNum = tn;
            taskHour = th;
            taskMinute = tm;
            greenPin = gp;
            yellowPin = yp;
            redPin = rp;
            pushPin = pp;
            minutesWarning = mw;
            taskFlag = false;
            taskWarn = false;
            warnSend = false;
            currentMs = millis();
            previousMs = currentMs;
            taskScore = 0;
            ledState = LOW;
            pinMode(gp, OUTPUT);
            pinMode(yp, OUTPUT);
            pinMode(rp, OUTPUT);
            pinMode(pp, INPUT);
        }
    bool checkTask(int nowMinutes){
        currentMs = millis();
        pushState = digitalRead(pushPin);
        int taskMinutes = taskHour * 60 + taskMinute; //convert time to minutes for easier comparisons
        int activeLed = redPin;
        if(nowMinutes > taskMinutes){                                       //Check for past due task
            taskScore = 0;
            blinkLed(redPin); //blink red led
            digitalWrite(yellowPin, LOW); //make sure yellow led is off
        }
        else if(nowMinutes >= taskMinutes - minutesWarning){                //Check past warning
            taskScore = 1;
            blinkLed(yellowPin);  //blink yellow led
            activeLed = yellowPin;
            if(!taskWarn){
                warnSend = true;
                taskWarn = true;
                Serial.println("warning sent");
            }
            else warnSend = false;
            digitalWrite(greenPin, LOW); //make sure green led is off
        }
        else{                                            //Otherwise task is still green
            taskScore = 2;
            blinkLed(greenPin); //blink green led
            activeLed = greenPin;
        }
    
        if(pushState == 1){ //if button was pressed, close out task
            digitalWrite(activeLed, HIGH); // turn active led on without blink
        }
        return warnSend;
    }
    int getTaskHour(){
        return taskHour;
    }
    int getTaskMinute(){
        return taskMinute;
    }
    private:
        int taskNum;
        int taskHour;
        int taskMinute;
        int greenPin;
        int yellowPin;
        int redPin;
        int pushPin;
        int minutesWarning;
        bool taskWarn;
        bool warnSend;
        unsigned long currentMs;
        unsigned long previousMs;
        int ledState;
        const long blinkInterval = 1000;

    void blinkLed(int ledPin){
        if (currentMs - previousMs >= blinkInterval) {
            previousMs = currentMs;  // save the last time you blinked the LED
 
            if (ledState == LOW) {  // if the LED is off turn it on and vice-versa:
            ledState = HIGH;
            } else {
                ledState = LOW;
            }
        }
        // set the LED with the ledState of the variable:
        digitalWrite(ledPin, ledState);
    } 

};
