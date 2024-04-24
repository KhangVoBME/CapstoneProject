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

const int NUM_ITEMS = 6;
const int MAX_ITEM_LENGTH = 20;
const int BUTTON_UP_PIN = 15;
const int BUTTON_SELECT_PIN = 32;
const int BUTTON_DOWN_PIN = 27; //this should be 13. 27 is the left button pin actually.
const int BUTTON_CENTER_PIN = 33; 
const int fsrPin = A0;
const int buzzerPin = A6;
string patientName;

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
int current_screen = 0; // 0 = Home screen, 1 = Menu screen, 2 = Data screen
unsigned long lastBuzz = 0;
const unsigned long buzzInterval = 10000; // 10 seconds

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
      alerts[numAlerts].message = "\nAlert triggered by:\n" + sensor;
      numAlerts++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  initRTC();
  display.begin();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2); 
  display.setRotation(1);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_CENTER_PIN, INPUT_PULLUP); 

  if (!sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1);
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
}

void displayHomeScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Name: ");
  display.println(patientName);
  display.print("Time: ");
  display.println(getDateTime());
  display.print("Alert Count: ");
  display.println(numAlerts);
  display.display();
}

void displayMenuScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  for (int i = 0; i < NUM_ITEMS; i++) {
    if (i == item_selected) {
      display.println(">" + String(menu_items[i]));
    } else {
      display.println("  " + String(menu_items[i]));
    }
  }
  display.display();
}

void displayDataScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  switch (item_selected) {
    case 0: // Alerts
      display.println("Alerts:");
      for (int i = 0; i < numAlerts; i++) {
        display.println(String(alerts[i].timestamp) + ": " + String(alerts[i].sensor));
      }
      break;
    case 1: // Humidity
      float humidity, temperature;
      readSHT4x(temperature, humidity);
      display.print("Humidity: ");
      display.print(humidity);
      display.println("%");
      break;
    case 2: // Temperature
      readSHT4x(temperature, humidity);
      display.print("Temperature: ");
      display.print(temperature);
      display.println(" C");
      break;
    case 3: // Pressure
      display.print("Pressure: ");
      display.println(readPressure());
      break;
    case 4: // Clock
      display.println("Current Time:");
      display.println(getDateTime());
      break;
    case 5: // Patient Info
      File patientInfoFile = SD.open("/patientinfo.txt");
      if (patientInfoFile) {
          patientName = patientInfoFile.readStringUntil('\n');
          String patientAge = patientInfoFile.readStringUntil('\n');
          String patientWeight = patientInfoFile.readStringUntil('\n');
          String patientWound = patientInfoFile.readStringUntil('\n');
          
          // Displaying patient information on the screen
          display.println("Patient Info:");
          display.println("Name: " + patientName);
          display.println("Age: " + patientAge);
          display.println("Weight: " + patientWeight);
          display.println("Wound: " + patientWound);

          patientInfoFile.close();
      } else {
          display.println("Failed to load info.");
          Serial.println("Failed to open patientinfo.txt");
      }
      break;
  }
  display.display();
}

void loop() {
  // Read sensors and check conditions for alerts
  float pressure = readPressure();
  float temperature, humidity;
  readSHT4x(temperature, humidity);

  if (pressure == 1 && humidity > 60 && temperature > 27.7) {
    buzz("Pressure\nHumidity\nTemperature");
  } else if ((temperature > 27.7 && humidity > 60) || (temperature > 27.7 && pressure == 1) || (pressure == 1 && humidity > 60)) {
    if (temperature > 27.7 && humidity > 60) {
      buzz("Humidity\nTemperature");
    }
    if (temperature > 27.7 && pressure == 1) {
      buzz("Pressure\nTemperature");
    }
    if (pressure == 1 && humidity > 60) {
      buzz("Pressure\nHumidity");
    }
  } else if (pressure == 1) {
    buzz("Pressure");
  } else if (humidity > 60) {
    buzz("Humidity");
  } else if (temperature > 27.7) {
    buzz("Temperature");
  }

  // Screen navigation and display updates
  if (digitalRead(BUTTON_SELECT_PIN) == LOW) {
    delay(200);
    current_screen = (current_screen + 1) % 3; // Cycle through screens
  }

  if (current_screen == 1) { // Menu Screen navigation
    if (digitalRead(BUTTON_UP_PIN) == LOW) {
      item_selected = (item_selected > 0) ? item_selected - 1 : NUM_ITEMS - 1;
      delay(200);
    }

    if (digitalRead(BUTTON_DOWN_PIN) == LOW) {
      item_selected = (item_selected < NUM_ITEMS - 1) ? item_selected + 1 : 0;
      delay(200);
    }
  }
 // function to clear alerts on Alerts screen when center is pressed
  if (current_screen == 2 && item_selected == 0 && digitalRead(BUTTON_CENTER_PIN) == LOW) { 
    numAlerts = 0; // Reset the number of alerts to 0
    Serial.println("All alerts cleared.");
    delay(200);
  }
  switch (current_screen) {
    case 0:
      displayHomeScreen();
      break;
    case 1:
      displayMenuScreen();
      break;
    case 2:
      displayDataScreen();
      break;
  }
}
