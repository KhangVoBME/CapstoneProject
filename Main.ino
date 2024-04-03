//Include up here
#include "Adafruit_SHT4x.h"

//Change if order if needed
const int button1 13 //Left
const int button2 12 //Right
const int button3 27 //Down
const int button4 33 //Up
const int button5 15 //Center

const int buzzer = A12;
const int forceSensor = A0;

Adafruit_SHT4x sht4 = Adafruit_SHT4x();


void homeScreen(){
  display.print("Home Screen!");
  display.display();
}//homeScreen brace

void screenOff(){
  display.clearDisplay();
  display.display();
}//screenOff brace

void screenOn(){

}//screenOn brace

void playBuzzer(int buzzerFreq){
  pinMode(buzzer, OUTPUT);
  tone(buzzer, buzzerFreq);
  delay(1000);
  noTone(buzzer);
}//playBuzzer brace
/*
Use this line to play buzzer, value is the frequency used. We can have different sounds for different situations.
playBuzzer(1000);
*/

void setup() {

  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);
  pinMode(button5, INPUT);
  Serial.begin(115200);

  delay(250);
  display.begin(0x3C, true);
  display.display();//splash screen
  delay(1000);
  display.clearDisplay();
  display.display();
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);

  homeScreen();

}//setup brace

void loop() {
  forceReading = analogRead(forceSensor);//Will constantly read force

  if(digitalRead(button1) == LOW){
  delay(250);
  }//if brace

  else if(digitalRead(button2) == LOW){
  delay(250);
  }//else if brace

  else if(digitalRead(button3) == LOW){
  delay(250);
  }//else if brace

  else if(digitalRead(button4) == LOW){
  delay(250);
  }//else if brace

  else if(digitalRead(button5) == LOW){
  delay(250);
  }//else if brace

  else{//this will be button 5
  delay(250);
  }//else brace

}
/*
one side connects to 3v
other goes to 10k which goes to ground
side not connected to 3v goes to a0 or any analog pin
*/
double normal = 20; //0-20% threshold for normal pressure
double infection = 50; //21-50% threshold for risk of infection
double tdamage = 70; // 51-70% threshold for risk of tissue damage
int fsrPin = A0;
double value = 0;
double percent = 0;     // the FSR and 10K pulldown are connected to a0
    // the analog reading from the FSR resistor divider

unsigned long lastA = 0; //time from last alert
const unsigned long interval = 21600000; //6 hour interval update
 
void loop() {
    unsigned long currentTime = millis();            //gets current time
    value = analogRead(fsrPin); //reads pin    
    percent = map(value, 0, 1023, 0 ,100); //way to change analog reading to percentage
    

        if(percent <= normal){ //if  percentage is in certain threshold then it will output the correct message
        Serial.println("Normal Range");

        }else if(percent <= infection && percent>= normal){ 
        display.print("Risk of Infection");
        display.print("Pressure off by");
        display.print(percent-normal);
        display.print("% at: ");
        showTime(currentTime);

        }else if(percent <= tdamage && percent>= infection){
        display.print("Risk of Tissue Damage");
        display.print("Pressure off by");
        display.print(percent-normal);
        display.print("% at: ");
        showTime(currentTime);
        }
        if(currentTime  - lastA >= interval){ // checks if 6 hours has passed
            lastA = currentTime;
            sixH(currentTime); //calls to 6 hour threshold function
        }
       delay(100);
     }

void showTime(unsigned long currentTime) { // gets time to look right
  unsigned long hours = currentTime / 3600000;
  unsigned long minutes = (currentTime % 3600000) / 60000;
  
  if (hours < 10) {
    display.print("0");
  }
  display.print(hours);
  display.print(":");
  if (minutes < 10) {
    display.print("0");
  }
  display.print(minutes);
}

void sixH(unsigned long currentTime){ //function to check every 6 hours
  if (percent <= normal) { 
    display.print("Normal Range");
  } else if (percent <= infection && percent >= normal) {
    display.print("Risk of Infection");
    Sdisplay.print("Pressure off by ");
    display.print(percent-normal);
    display.print("% at:");
    showTime(currentTime);
  } else if (percent <= tdamage && percent >= infection) {
    display.print("Risk of Tissue Damage");
    display.print("Pressure off by ");
    display.print(percent-normal);
    display.print("% at:");
    showTime(currentTime);
  }
    delay(100);
}
