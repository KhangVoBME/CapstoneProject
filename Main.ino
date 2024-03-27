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

}//loop brace
