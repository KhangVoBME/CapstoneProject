// pretty sure most of the sd card code can be removed but after i copied and pasted a bunch of random stuff from the sd card file,
// it worked so i didnt really want to take the risk of removing the random sd card code.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "RTClib.h"
#include "Adafruit_SHT4x.h"
#include "FS.h"
#include "SD.h"

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
Adafruit_SHT4x sht4 = Adafruit_SHT4x();
RTC_PCF8523 rtc;

#define WHITE 0xFFFF
#define VBATPIN A13
const int NUM_ITEMS = 6;
const int MAX_ITEM_LENGTH = 20;
const int BUTTON_UP_PIN = 15;
const int BUTTON_SELECT_PIN = 32;
const int BUTTON_DOWN_PIN = 13; //this should be 13. 27 is the left button pin actually.
const int BUTTON_LEFT_PIN = 27;
const int BUTTON_CENTER_PIN = 33; 
const int fsrPin = A0;
const int buzzerPin = A6;
int coords = 0;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char menu_items[NUM_ITEMS][MAX_ITEM_LENGTH] = {
    {"Alerts"},
    {"Humidity"},
    {"Temp"},
    {"Pressure"},
    {"Clock"},
    {"Patient Info"}};

struct Alert {
  String timestamp;
  String sensor;
  String message;
};


const int MAX_ALERTS = 10; // max number of alerts
Alert alerts[MAX_ALERTS];   // array to store alerts
int numAlerts = 0;           // current number of alerts
int currentAlertIndex = 0;   // index of the current alert being displayed

int item_selected = 0; //index  of current menu item selected
int current_screen = 0; // 0=menu screen 1= data screen for selected menu option
unsigned long lastBuzz = 0;
const unsigned long buzzInterval = 10000; // 10 sec

void initRTC() {
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.start();

   // Calibrate RTC
  float drift = 43; // seconds plus or minus over observation period - set to 0 to cancel previous calibration.
  float period_sec = (7 * 86400);  // total observation period in seconds (86400 = seconds in 1 day:  7 days = (7 * 86400) seconds )
  float deviation_ppm = (drift / period_sec * 1000000); //  deviation in parts per million (μs)
  float drift_unit = 4.34; // use with offset mode PCF8523_TwoHours

  int offset = round(deviation_ppm / drift_unit);
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)).unixtime() + offset);
}


String getDateTime() {
  DateTime now = rtc.now();
  String dateTimeString = "";
  dateTimeString += String(now.day(), DEC) + '/';
  dateTimeString += String(now.month(), DEC) + '/';
  dateTimeString += String(now.year(), DEC) + " ";
  if (now.hour() < 10) {
    dateTimeString += "0";
  }
  dateTimeString += String(now.hour(), DEC) + ':';
  dateTimeString += String(now.minute(), DEC) + ':';
  dateTimeString += String(now.second(), DEC);
  return dateTimeString;
}

// read temp/humidity values from sensor 
void readSHT4x(float &temperature, float &humidity) {
  sensors_event_t humidity_event, temperature_event;
  sht4.getEvent(&humidity_event, &temperature_event);
  temperature = temperature_event.temperature;
  humidity = humidity_event.relative_humidity;
}

//read pressure 
float readPressure() {
  int value = analogRead(fsrPin) / 4095;
  return value;
}

//this function is called if sensor condition is met
void buzz(String sensor) {
  // basically checks for 10 seconds between buzzes
  unsigned long currentTime = millis();
  if (currentTime - lastBuzz >= buzzInterval) {
    pinMode(buzzerPin, OUTPUT);
    tone(buzzerPin, 10000);
    delay(1000);
    noTone(buzzerPin);
    lastBuzz = currentTime; //update the last buzz

    // write the alerts into the sd card 
    String filename = "/sensors.txt";
    File file = SD.open(filename, FILE_APPEND); // set to only APPEND the file.
    if (file) { // basically, there needs to be a file called sensors already. the code does not check/create a new file.
      file.print(getDateTime());
      file.print(" : \n"); 
      file.println(sensor);
      file.close();
      Serial.println("alert to sd card ");
    } else {
      Serial.println("failed to open");
    }

    // add to alerts 
    if (numAlerts < MAX_ALERTS) {
      alerts[numAlerts].timestamp = getDateTime();
      alerts[numAlerts].sensor = sensor;
      //alerts[numAlerts].message = "\nAlert triggered by:\n" + sensor;
      alerts[numAlerts].message = sensor + "warning!\n";
      numAlerts++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  initRTC();
  Wire.begin();
  display.begin();

  display.display();//splash screen
  delay(2000);

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2); 
  display.setRotation(1);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_CENTER_PIN, INPUT_PULLUP); 

  if (sht4.begin()) {
    Serial.println("Connected");
    
  } else{
    Serial.println("Not Connected");
  }

  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  // read patient info from the txt file below
  File patientInfoFile = SD.open("/patientinfo.txt");
  if (patientInfoFile) {
    // reads each line as a different variable. 
    String patientName = patientInfoFile.readStringUntil('\n');
    String patientAge = patientInfoFile.readStringUntil('\n');
    String patientWeight = patientInfoFile.readStringUntil('\n');
    String patientWound = patientInfoFile.readStringUntil('\n');

    // use display print later. mostly for debugging 
    Serial.println("Patient Name: " + patientName);
    Serial.println("Patient Age: " + patientAge);
    Serial.println("Patient Weight: " + patientWeight);
    Serial.println("Patient Wound: " + patientWound);

    patientInfoFile.close();
  } else {
    Serial.println("failed to open patientinfo.txt");
  }
}

// this is for menu navigation
void loop() {

  ///Polyfit values: 1.2995*10^-6x^3, -1.939*10^-4x^2, 0.0157x, 3.2715
  float measuredvbat = analogReadMilliVolts(VBATPIN);
measuredvbat *= 2;    // we divided by 2, so multiply back
measuredvbat /= 1000; // convert to volts!
Serial.print("VBat: " ); Serial.println(measuredvbat);
///



  if (sht4.begin()) {//SHT40 connected
    display.clearDisplay();
     if (digitalRead(BUTTON_UP_PIN) == LOW && current_screen != 1) {
    item_selected--;
    if (item_selected < 0) {
      item_selected = NUM_ITEMS - 1;
    }
    delay(200); // debounce
  }

  if (digitalRead(BUTTON_DOWN_PIN) == LOW && current_screen != 1) {
    item_selected++;
    if (item_selected >= NUM_ITEMS) {
      item_selected = 0;
    }
    delay(200); 
  }

  if (digitalRead(BUTTON_SELECT_PIN) == LOW) {
      current_screen = 1;
    delay(200); 
  }

   if (digitalRead(BUTTON_LEFT_PIN) == LOW) {
      current_screen = 0;
    delay(200); 
  }

  // read pressure, temperature, and humidity values
  float pressure = readPressure();
  float temperature, humidity;
  readSHT4x(temperature, humidity);

  // call the buzzer function (alerts and sd card) if condition is met 
  // check if all 3 conditions are true 
  if (pressure == 1 && humidity > 60 && temperature > 27.7) {
    buzz("Pressure\nHumidity\nTemperature");
  }
  // checks if 2 conditions are true 
  else if ((temperature > 27.7 && humidity > 60) || (temperature > 27.7 && pressure == 1) || (pressure == 1 && humidity > 60)) {
    if (temperature > 27.7 && humidity > 60) {
      buzz("Humidity\nTemperature");
    }
    if (temperature > 27.7 && pressure == 1) {
      buzz("Pressure\nTemperature");
    }
    if (pressure == 1 && humidity > 60) {
      buzz("Pressure\nHumidity");
    }
  }
  // check if only 1 condition is true
  else if (pressure == 1) {
    buzz("Pressure");
  }
  else if (humidity > 60) {
    buzz("Humidity");
  }
  else if (temperature > 27.7) {
    buzz("Temperature");
  }

  display.clearDisplay();
  // this was all necessary in order to make the menu options bigger. have to print the menu options dynamically 
  // current screen==0 is the MENU screen
  if (current_screen == 0) {
    int start_index = item_selected;
    int end_index = start_index + 3;
    if (end_index > NUM_ITEMS) {
      end_index = NUM_ITEMS;
    }
    for (int i = start_index; i < end_index; i++) {
      if (i == item_selected) {
        display.setTextColor(SH110X_WHITE);
        display.setTextSize(2); // font size
        display.setCursor(0, (i - start_index) * 20); // spacing 
        display.print(">");
      } else {
        display.setTextColor(SH110X_WHITE);
        display.setTextSize(2); 
        display.setCursor(10, (i - start_index) * 20); 
      }
      display.println(menu_items[i]);

    if(item_selected == 0){
      coords = 0;
    }else if(item_selected > 0){
      coords = item_selected * 9;//Change this value depending on number of menu items and how much the scroll bar "moves"
    }

      display.fillRect(126, coords, 3, 20, SH110X_WHITE);
    }
    // current screen==1 is the data screen depending on the selected menu option
  } else if (current_screen == 1) {
    if (item_selected == 0) { // alerts option
      // display alerts
      display.setCursor(0, 0);
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(1);


 /*display.print("Number of Alerts: ");
      display.println(numAlerts);
      if (numAlerts > 0) {
        display.println();
        display.print(alerts[currentAlertIndex].timestamp);
        display.println(alerts[currentAlertIndex].message);
      }*/





      if(numAlerts == 0){
        display.setCursor(42, 0);
         display.println("No Alerts");
      }else if(numAlerts > 0){
        display.setCursor(0, 0);
        display.print("You have ");
        display.print(numAlerts);
        display.print(" ");
        

        if(numAlerts == 1){
          display.print("alert.");
        }else if(numAlerts > 1){
          display.print("alerts.");
        }
        
        display.println();
        display.println();
        display.print("Alert #");
        display.print(currentAlertIndex + 1);
        display.println();
        display.print(alerts[currentAlertIndex].timestamp);
        display.println();
        display.print(alerts[currentAlertIndex].message);
      }

      
      // move to previous alert 
      if (digitalRead(BUTTON_UP_PIN) == LOW && current_screen == 1 && item_selected == 0) {
        currentAlertIndex--;
        if (currentAlertIndex < 0) {
          currentAlertIndex = numAlerts - 1;
        }
        delay(200);
      }
    // move to next alert 
      if (digitalRead(BUTTON_DOWN_PIN) == LOW && current_screen == 1 && item_selected == 0) {
        currentAlertIndex = (currentAlertIndex + 1) % numAlerts;
        delay(200); 
      }
    // clear all alerts
      if (digitalRead(BUTTON_CENTER_PIN) == LOW) {
        numAlerts = 0;
        Serial.println("clear");
        delay(200); 
      }
    } else if (item_selected == 1) { // humidity option
      display.setCursor(0, 0);
      display.setTextColor(SH110X_WHITE);
      display.print(humidity);
      display.println("% rH");
    } else if (item_selected == 2) { // temperature option in C and F
      display.setCursor(0, 0);
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(1);
      display.print((temperature * 9 / 5) + 32);
      display.println(" degrees F");
      display.print(temperature);
      display.println(" degrees C");
    } else if (item_selected == 3) { //pressure option 
      display.setCursor(0, 0);
      display.setTextColor(SH110X_WHITE);
      display.print(pressure);
    } else if (item_selected == 4) { // clock option 
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      String dateTimeString = getDateTime();
      String date = dateTimeString.substring(0, 10);
      String day = daysOfTheWeek[rtc.now().dayOfTheWeek()];
      String time = dateTimeString.substring(11);
      display.println("Date: " + date);
      display.println("Day: " + day);
      display.println("Time: " + time);
    } else if (item_selected == 5) { // patient info option
      display.setCursor(0, 0);
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(1);
      File patientInfoFile = SD.open("/patientinfo.txt");
      if (patientInfoFile) {
        String patientName = patientInfoFile.readStringUntil('\n');
        String patientAge = patientInfoFile.readStringUntil('\n');
        String patientWeight = patientInfoFile.readStringUntil('\n');
        String patientWound = patientInfoFile.readStringUntil('\n');

        // display patient information
        display.println("Name: " + patientName);
        display.println("Age: " + patientAge);
        display.println("Weight: " + patientWeight);
        display.println("Wound: " + patientWound);

        // close the file
        patientInfoFile.close();
      } else { //
        display.println("failed to open patientinfo.txt");
      }
    } else { // mostly for debugging.
      display.setCursor(0, 0);
      display.setTextColor(SH110X_WHITE);
      display.println("Invalid selection");
    }
  }

  display.display();
    
} else{//SHT40 not connected
    while(!sht4.begin()){
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      display.setCursor(0,0);
      display.clearDisplay();
      delay(1000);
      display.print("Please Connect Sensors!");
      display.display();

    }
}



 
}