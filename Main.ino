//Include up here
#include "Adafruit_SHT4x.h"

//Change if order if needed
const int button1 = 13; // Left
const int button2 = 12; // Right
const int button3 = 27; // Down
const int button4 = 33; // Up
const int button5 = 15; // Center

const int buzzer = A12;
const int forceSensor = A0;

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
bool screenIsOn = false; // Track screen status

void homeScreen() {
  display.clearDisplay(); // Clear any existing stuff on display
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);

  // LINE 1: Call and display date and time
  displayDateTime();

  // LINE 2: Call and display notifications
  displayNotifications();

  // LINE 3: Call and display 6-hour update status
  bool isGood = checkUpdateStatus();
  displayUpdateStatus(isGood);

  display.display(); 
}//homeScreen brace

void screenOff() {
  display.clearDisplay();
  display.display();
  screenIsOn = false;
}//screenOff brace

void displaySplash() {
  display.print("HealGuard Version 2.0");
  display.display();
  delay(2000); // Splash Screen for 2 sec
  screenOn(); // Turn on the screen after splash which should call home after
}//displaysplash brace

void displayDateTime() {
  String dateTime = ""; //needs work
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0); // print on first
  display.println(update); // 
  display.println(dateTime);
}//datetime brace

void displayNotifications() {
  String notifications = ""; // still needs setting up
   display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 8); // print on second
  display.println(update); // 
  display.println(notifications);
}//displaynotifbrace

bool checkUpdateStatus() {
  float temperature = readTemperature(); // Get temperature reading
  bool isGood = (temperature >= 36.1 && temperature <= 37.2);
  return isGood;
}//check status brace

void displayUpdateStatus(bool isGood) {
  String status = isGood ? "Good" : "Bad"; // Determine status based on isGood value
  String update = "6 Hour Update: " + status;
  
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 16); // print on third
  display.println(update); 
  display.display(); // Update the display
}// display updatestatus brace

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
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);

  // Display splash screen initially
  displaySplash();
}//setup brace

void loop() {
  forceReading = analogRead(forceSensor); // Will constantly read force

  if (digitalRead(button1) == LOW) {
    delay(250);
    }//if brace
 else if (digitalRead(button2) == LOW) {
    delay(250);
     }//else if brace 
else if (digitalRead(button3) == LOW) {
    delay(250);
      } //else if brace
else if (digitalRead(button4) == LOW) {
    delay(250);
  } //else if brace
else if (digitalRead(button5) == LOW) {
    delay(250);
    screenIsOn = true;
    displaySplash(); // Turn on the screen and display splash screen
    delay(100); // debouncing delay just in case
    homeScreen(); // Display the home screen after splash screen
  } 

}//loop brace

void screenOn() {
  display.clearDisplay(); // Clear any existing info on screen
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
}//clear screen when screen on 
