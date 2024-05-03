#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "RTClib.h"
#include "Adafruit_SHT4x.h"
#include "FS.h"
#include "SD.h"
#include "InterpolationLib.h"

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
Adafruit_SHT4x sht4 = Adafruit_SHT4x();
RTC_PCF8523 rtc;

#define WHITE 0xFFFF
#define VBATPIN A13
const int NUM_ITEMS = 7;

const int MAX_ITEM_LENGTH = 20;
const int BUTTON_UP_PIN = 15;
const int BUTTON_SELECT_PIN = 32;
const int BUTTON_DOWN_PIN = 13;
const int BUTTON_LEFT_PIN = 27;
const int BUTTON_CENTER_PIN = 33;
const int fsrPin = A0;
const int buzzerPin = A6;
const int numValues = 10;
double batVTable[11] = { 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 4.0, 4.1, 4.2 };
double batPercTable[11] = { 0, 0, 6, 15, 33, 50, 59, 72, 83, 94, 100 };
int CurrentPerc = 0;
int coords = 0;
int settingsIndex = 1;
bool mute = 0;
bool dev = 1;
int standby = 0;
String devMessage = "On";
String muteMessage = "Off";
String standbyMessage = "Off";
bool messagesPage = 0;
bool units = 0;
int fsrReading;     // the analog reading from the FSR resistor divider
int fsrVoltage;     // the analog reading converted to voltage
unsigned long fsrResistance;  // The voltage converted to resistance
unsigned long fsrConductance;
long fsrForce;
int pThresh = 2500;
int hThresh = 60;
int tThresh = 26.1;

char daysOfTheWeek[7][12] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char menu_items[NUM_ITEMS][MAX_ITEM_LENGTH] = {
    { "Alerts"},
    { "Humidity"},
    { "Temp"},
    { "Pressure"},
    { "Clock"},
    { "Records"},
    { "Settings"}
};

/*char menu_items2[NUM_ITEMS2][MAX_ITEM_LENGTH] = {
    {"Mute"},
    {"Standby"},
    {"Developer Mode"}};*/

struct Alert
{
    String timestamp;
    String message;
};

const int MAX_ALERTS = 30; // max number of alerts
Alert alerts[MAX_ALERTS];   // array to store alerts
int numAlerts = 0;          // current number of alerts
int currentAlertIndex = 0;  // index of the current alert being displayed
const char* alertFileName = "/testalert.txt";
int item_selected = 0;// index of the current menu item selected
int current_screen = 0;
unsigned long lastBuzz = 0;
const unsigned long buzzInterval = 10000; // 10 sec

void initRTC()
{
    Wire.begin();
    if (!rtc.begin())
    {
        //Serial.println("Couldn't find RTC");
        while (1) ;
    }

    if (!rtc.initialized() || rtc.lostPower())
    {
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

String getDateTime()
{
    DateTime now = rtc.now();
    String dateTimeString = "";

    dateTimeString += String(now.day(), DEC) + '/';
    dateTimeString += String(now.month(), DEC) + '/';
    dateTimeString += String(now.year(), DEC) + " ";

    int hour = now.hour();
    bool isPM = false;

    if (hour >= 12)
    {
        isPM = true;
        if (hour > 12)
        {
            hour -= 12;
        }
    }

    if (hour == 0)
    {
        hour = 12;
    }

    dateTimeString += String(hour, DEC) + ':';

    //need leading zero.
    if (now.minute() < 10)
    {
        dateTimeString += "0";
    }
    dateTimeString += String(now.minute(), DEC) + ':';

    if (now.second() < 10)
    {
        dateTimeString += "0";
    }
    dateTimeString += String(now.second(), DEC);

    // append am/pm 
    dateTimeString += (isPM ? " PM" : " AM");

    return dateTimeString;
}

String getDateTime12Hour()
{
    DateTime now = rtc.now();

    int hour = now.hour();
    int minute = now.minute();
    int second = now.second();
    bool isPM = (hour >= 12);

    if (hour == 0)
    {
        hour = 12;
    }
    else if (hour > 12)
    {
        hour -= 12;
    }

    String formattedTime = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + ":" + (second < 10 ? "0" : "") + String(second) + (isPM ? " PM" : " AM");

    return formattedTime;
}

// read temp/humidity values from sensor 
void readSHT4x(float &temperature, float &humidity)
{
    sensors_event_t humidity_event, temperature_event;
    sht4.getEvent(&humidity_event, &temperature_event);
    temperature = temperature_event.temperature;
    humidity = humidity_event.relative_humidity;
}

// read pressure 
float readPressure()
{
    int value = analogRead(fsrPin);
    return value;
}

// buzzer function called when sensor condition is met (alert system)
void buzz(String sensor)
{
    unsigned long currentTime = millis();
    if (currentTime - lastBuzz >= buzzInterval)
    {
        pinMode(buzzerPin, OUTPUT);

        if (mute == 0)
        {
            tone(buzzerPin, 10000);
        }
        else
        {

        }

        delay(1000);
        noTone(buzzerPin);
        lastBuzz = currentTime;

        //get timestamp
        String timestamp = getDateTime();

        // format the entry to sd card
        String alertEntry = "Timestamp: " + timestamp + "\nMessage: " + sensor + "\n";


        File sensorsFile = SD.open("/sensors.txt", FILE_APPEND);
        if (sensorsFile)
        {
            sensorsFile.print(alertEntry);
            sensorsFile.close();
            //Serial.println("alert to sd card ");
        }
        else
        {
            //Serial.println("failed to open");
        }

        File alertFile = SD.open("/testalert.txt", FILE_APPEND);
        if (alertFile)
        {
            alertFile.print(alertEntry);
            alertFile.close();
            //Serial.println("test alert");
        }
        else
        {
            //Serial.println("test alert fail");
        }
    }
}

// this is the function that reads the alerts file and extracts the data 
void readAlertsFromFile()
{
    File file = SD.open("/testalert.txt");
    if (file)
    {
        numAlerts = 0;
        file.seek(0);
        while (file.available() && numAlerts < MAX_ALERTS)
        {
            String line = file.readStringUntil('\n');
            if (line.startsWith("Timestamp: "))
            {
                alerts[numAlerts].timestamp = line.substring(11); // ignoring the characters "Timestamp: " to extract what comes after 
                line = file.readStringUntil('\n');
                if (line.startsWith("Message: "))
                {
                    alerts[numAlerts].message = line.substring(9); // same thing as above with the timestamp
                    numAlerts++;
                }
            }
        }
        file.close();
    }
    else
    {
        //Serial.println("alert dead");
    }
}

void clearAlertsFile()
{
    File alertFile = SD.open("/testalert.txt", FILE_WRITE);

    if (alertFile)
    {
        alertFile.seek(0);

        alertFile.write(0);

        alertFile.close();
        //Serial.println("alert clear");
    }
    else
    {
        //Serial.println("alert deaded");
    }
}

void setup()
{
    Serial.begin(115200);
    initRTC();
    Wire.begin();
    display.begin();

    display.display(); // splash screen
    delay(2000);

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.setRotation(1);
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_CENTER_PIN, INPUT_PULLUP);

    if (sht4.begin())
    {
        //Serial.println("SHT4x Connected");
    }
    else
    {
        //Serial.println("SHT4x Not Connected");
    }

    if (!SD.begin())
    {
        //Serial.println("SD Card Mount Failed");
        return;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        //Serial.println("No SD card attached");
        return;
    }

    //Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        //Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        //Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        //Serial.println("SDHC");
    }
    else
    {
        //Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    //Serial.printf("SD Card Size: %lluMB\n", cardSize);

    readAlertsFromFile();

}
void loop()
{
    if (sht4.begin())
    {

        if (dev == 1)
        {
            pThresh = 2500;
            hThresh = 60;
            tThresh = 26.1;
        }
        else if (dev == 0)
        {
            pThresh = 4094;
            hThresh = 60;
            tThresh = 33;
        }
        display.clearDisplay();

        // make the up and down buttons only work in menu screen
        if (digitalRead(BUTTON_UP_PIN) == LOW && current_screen != 0 && current_screen != 2)
        {
            item_selected--;
            if (item_selected < 0)
            {
                item_selected = NUM_ITEMS - 1;
            }
            delay(200); // debounce
        }

        if (digitalRead(BUTTON_DOWN_PIN) == LOW && current_screen != 0 && current_screen != 2)
        {
            item_selected++;
            if (item_selected >= NUM_ITEMS)
            {
                item_selected = 0;
            }
            delay(200);
        }

        // need to disable the right button from screen 2-> screen 0
        if (digitalRead(BUTTON_SELECT_PIN) == LOW)
        {
            if (current_screen == 2)
            {
                delay(200);
            }
            else
            {
                current_screen = (current_screen + 1) % 3; // cycle through the screens 
                delay(200);
            }
        }

        // need to disable the left button from screen 0 ->2
        if (digitalRead(BUTTON_LEFT_PIN) == LOW)
        {
            if (current_screen == 0)
            {
                delay(200);
            }
            else
            {
                current_screen = (current_screen == 0) ? 2 : current_screen - 1;
                delay(200);
            }
        }

        float pressure = readPressure();
        float temperature, humidity;
        readSHT4x(temperature, humidity);


        if (pressure > pThresh && humidity > hThresh && temperature > tThresh)
        {
            buzz("Pressure, Humidity, Temperature");
        }
        else if (temperature > tThresh && humidity > hThresh)
        { // check if 2 conditions are true 
            buzz("Humidity, Temperature");
        }
        else if (temperature > tThresh && pressure > pThresh)
        {
            buzz("Pressure, Temperature");
        }
        else if (pressure > pThresh)
        { // checks for 1 condition true
            buzz("Pressure");
        }
        else if (humidity > hThresh)
        {
            buzz("Humidity");
        }
        else if (temperature > tThresh)
        {
            buzz("Temperature");
        }

        display.clearDisplay();

        if (current_screen == 0)
        {
            // this is the HOME SCREEN.
            display.setCursor(0, 0);
            display.setTextColor(SH110X_WHITE);
            display.setTextSize(1);

            //String date = getDateTime().substring(0, 10); 
            String day = daysOfTheWeek[rtc.now().dayOfTheWeek()];
            String time12Hour = getDateTime12Hour();
            DateTime now = rtc.now();

            display.print(day + " ");
            display.print(now.month());
            display.print("/");
            display.print(now.day());
            display.print("/");
            display.print("24");

            //Battery Percentage
            display.setCursor(95, 0);
            float measuredvbat = analogReadMilliVolts(VBATPIN);
            measuredvbat *= 2;    // we divided by 2, so multiply back
            measuredvbat /= 1000; // convert to volts
            while (!Serial) {; }
            for (float xValue = measuredvbat; xValue <= measuredvbat; xValue += 1)
            {
                CurrentPerc = Interpolation::Linear(batVTable, batPercTable, numValues, xValue, false);
                display.print(CurrentPerc);
                display.println("%");
            }
            //
            //Battery graphic
            display.drawRect(114, 0, 12, 7, SH110X_WHITE);
            display.fillRect(126, 2, 2, 3, SH110X_WHITE);
            if (CurrentPerc > 74)
            {
                display.fillRect(115, 1, 3, 5, SH110X_WHITE);
                display.fillRect(118, 1, 3, 5, SH110X_WHITE);
                display.fillRect(121, 1, 2, 5, SH110X_WHITE);
                display.fillRect(123, 1, 2, 5, SH110X_WHITE);
            }
            else if (CurrentPerc > 49 && CurrentPerc < 75)
            {
                display.fillRect(115, 1, 3, 5, SH110X_WHITE);
                display.fillRect(118, 1, 3, 5, SH110X_WHITE);
                display.fillRect(121, 1, 2, 5, SH110X_WHITE);
            }
            else if (CurrentPerc > 24 && CurrentPerc < 50)
            {
                display.fillRect(115, 1, 3, 5, SH110X_WHITE);
                display.fillRect(118, 1, 3, 5, SH110X_WHITE);
            }
            else if (CurrentPerc > 0 && CurrentPerc < 25)
            {
                display.fillRect(115, 1, 3, 5, SH110X_WHITE);
            }
            else if (CurrentPerc == 0)
            {
            }
            display.println(time12Hour);
            display.println();

            // this is for the number of alerts 
            readAlertsFromFile();


            if (numAlerts == 0)
            {
                display.println("You have no alerts.");
            }
            else if (numAlerts > 0)
            {
                display.print("You have ");
                display.print(String(numAlerts));
                display.print(" ");


                if (numAlerts == 1)
                {
                    display.println("alert.");
                }
                else if (numAlerts > 1)
                {
                    display.println("alerts!");
                }
            }
            display.println();
            display.println();
            display.println();
            display.println("                Menu");
            display.fillTriangle(121, 56, 121, 62, 127, 59, SH110X_WHITE);
            //display.println("Alerts: " + String(numAlerts));
        }
        else if (current_screen == 1)
        {
            // this is now the code for the MENU 
            int start_index = item_selected;
            int end_index = start_index + 3;
            if (end_index > NUM_ITEMS)
            {
                end_index = NUM_ITEMS;
            }
            for (int i = start_index; i < end_index; i++)
            {
                if (i == item_selected)
                {
                    display.setTextColor(SH110X_WHITE);
                    display.setTextSize(2);
                    display.setCursor(0, (i - start_index) * 20);
                    display.print(">");
                }
                else
                {
                    display.setTextColor(SH110X_WHITE);
                    display.setTextSize(2);
                    display.setCursor(10, (i - start_index) * 20);
                }
                display.println(menu_items[i]);

                //Scroll bar
                if (item_selected == 0)
                {
                    coords = 0;
                }
                else if (item_selected > 0)
                {
                    coords = item_selected * 9;//Change this value depending on number of menu items and how much the scroll bar "moves"
                }

                display.fillRect(126, coords, 3, 20, SH110X_WHITE);
                /////
            }
        }
        else if (current_screen == 2)
        {
            // this is now the screen for item selected 
            if (item_selected == 0)
            { // alerts
                /*if(digitalRead(BUTTON_SELECT_PIN) == LOW && messagesPage == 0){
                  messagesPage = 1;
                  delay(200);
                }
                if(digitalRead(BUTTON_LEFT_PIN) == LOW && messagesPage == 1){

                  messagesPage = 0;
                  delay(200);
                }*/
                //Serial.println(messagesPage);
                if (messagesPage == 0)
                {
                    readAlertsFromFile();

                    display.setCursor(0, 0);
                    display.setTextColor(SH110X_WHITE);
                    display.setTextSize(1);

                    if (numAlerts == 0)
                    {
                        display.setCursor(42, 0);
                        display.println("No Alerts");
                    }
                    else
                    {
                        display.print("Alert ");
                        display.print(currentAlertIndex + 1);
                        display.print(" of ");
                        display.println(numAlerts);
                        //display.print("Timestamp: ");
                        display.println(alerts[currentAlertIndex].timestamp);
                        //Alerts: Warning: Triggered: etc.
                        String message = alerts[currentAlertIndex].message;

                        // Printing each sensor on a different line using the comma. 
                        int lastIndex = 0;
                        int index = message.indexOf(", ");
                        while (index != -1)
                        {
                            display.print(message.substring(lastIndex, index));
                            display.print(", ");
                            lastIndex = index + 2;
                            index = message.indexOf(", ", lastIndex);
                        }

                        display.print(message.substring(lastIndex));
                        display.println(" Warning!\n");
                        display.println();
                        if (alerts[currentAlertIndex].message == "Pressure")
                        {
                            display.setCursor(0, 37);
                            display.print("Relieve pressure or readjust bandage.");
                        }
                        else if (alerts[currentAlertIndex].message == "Temperature")
                        {
                            display.setCursor(0, 30);
                            display.print("Monitor symptoms.May need to seek physician,possible infection.");
                        }
                        else if (alerts[currentAlertIndex].message == "Humidity")
                        {
                            display.setCursor(0, 37);
                            display.print("Clean wound area and replace dressings.");
                        }
                        display.setCursor(88, 56);
                        display.setTextSize(1);
                        display.print("Clear");
                        display.drawCircle(123, 59, 4, SH110X_WHITE);
                        display.fillCircle(123, 59, 2, SH110X_WHITE);
                        display.setCursor(9, 56);
                        display.setTextSize(1);
                        display.println("Back");
                        display.fillTriangle(6, 56, 6, 62, 0, 59, SH110X_WHITE);


                    }

                    if (digitalRead(BUTTON_UP_PIN) == LOW && item_selected == 0)
                    {
                        currentAlertIndex--;
                        if (currentAlertIndex < 0)
                        {
                            currentAlertIndex = numAlerts - 1;
                        }
                        delay(200);
                    }

                    if (digitalRead(BUTTON_DOWN_PIN) == LOW && item_selected == 0)
                    {
                        currentAlertIndex = (currentAlertIndex + 1) % numAlerts;
                        delay(200);
                    }

                    //deletes the file, then makes a new file with the same exact name.

                    if (digitalRead(BUTTON_CENTER_PIN) == LOW)
                    {

                        SD.remove(alertFileName);
                        File newFile = SD.open(alertFileName, FILE_WRITE); // create a new file 
                        if (newFile)
                        {
                            newFile.close();
                            //Serial.println("alert thanosd");
                        }
                        else
                        {
                            //Serial.println("clear system failed ");
                        }
                        numAlerts = 0;
                        currentAlertIndex = 0;
                        delay(200);
                    }
                }
                else if (messagesPage = 1)
                {

                    display.print("test");

                }
            }
            else if (item_selected == 1)
            { // humidity
                display.setCursor(0, 0);
                display.setTextSize(2);
                display.setTextColor(SH110X_WHITE);
                display.print(humidity);
                display.println("% rH");
                display.setCursor(9, 56);
                display.setTextSize(1);
                display.println("Back");
                display.fillTriangle(6, 56, 6, 62, 0, 59, SH110X_WHITE);
            }
            else if (item_selected == 2)
            { // temperature 
                display.setCursor(0, 0);
                display.setTextColor(SH110X_WHITE);
                display.setTextSize(2);

                if (units == 0)
                {
                    display.print((temperature * 9 / 5) + 32);
                    display.println(" F");
                    display.drawCircle(65, 2, 2, SH110X_WHITE);
                }
                else
                {
                    display.print(temperature);
                    display.println(" C");
                    display.drawCircle(65, 2, 2, SH110X_WHITE);
                }
                display.setCursor(9, 56);
                display.setTextSize(1);
                display.println("Back");
                display.fillTriangle(6, 56, 6, 62, 0, 59, SH110X_WHITE);
                display.setCursor(95, 56);
                display.print("C/ F");
                display.drawCircle(123, 59, 4, SH110X_WHITE);
                display.fillCircle(123, 59, 2, SH110X_WHITE);
                display.drawCircle(93, 55, 1, SH110X_WHITE);
                display.drawCircle(110, 55, 1, SH110X_WHITE);
                if (digitalRead(BUTTON_CENTER_PIN) == LOW)
                {
                    units = !units; //mute variable to a bool


                    delay(200);
                }
            }
            else if (item_selected == 3)
            { // pressure.
                display.setCursor(0, 0);
                display.setTextSize(2);
                display.setTextColor(SH110X_WHITE);
                int pressure2 = pressure;
                display.print(pressure2 / 8902.17);
                display.print(" PSI");
                display.setCursor(9, 56);
                display.setTextSize(1);
                display.println("Back");
                display.fillTriangle(6, 56, 6, 62, 0, 59, SH110X_WHITE);
            }
            else if (item_selected == 4)
            { // clock
                display.setCursor(0, 0);
                display.setTextSize(2);
                display.setTextColor(SH110X_WHITE);

                String date = getDateTime().substring(0, 10);
                String day = daysOfTheWeek[rtc.now().dayOfTheWeek()];

                String time12Hour = getDateTime12Hour();

                DateTime now = rtc.now();

                display.print(day + " ");
                display.print(now.month());
                display.print("/");
                display.print(now.day());
                display.print("/");
                display.println("24");
                display.print(time12Hour);
                display.setCursor(9, 56);
                display.setTextSize(1);
                display.println("Back");
                display.fillTriangle(6, 56, 6, 62, 0, 59, SH110X_WHITE);
            }
            else if (item_selected == 5)
            { //Records
                display.setCursor(0, 0);
                display.setTextColor(SH110X_WHITE);
                display.setTextSize(1);
                File patientInfoFile = SD.open("/patientinfo.txt");
                if (patientInfoFile)
                {
                    String patientName = patientInfoFile.readStringUntil('\n');
                    String patientAge = patientInfoFile.readStringUntil('\n');
                    String patientWeight = patientInfoFile.readStringUntil('\n');
                    String patientWound = patientInfoFile.readStringUntil('\n');

                    display.println("Name: " + patientName);
                    display.println("Age: " + patientAge);
                    display.println("Weight: " + patientWeight);
                    display.println("Wound: " + patientWound);

                    patientInfoFile.close();
                }
                else
                {
                    display.println("failed to open patientinfo.txt");
                }
                display.setCursor(9, 56);
                display.setTextSize(1);
                display.println("Back");
                display.fillTriangle(6, 56, 6, 62, 0, 59, SH110X_WHITE);
            }
            else if (item_selected == 6)
            { //Settings

                display.setCursor(0, 0);
                display.setTextColor(SH110X_WHITE);
                display.setTextSize(2);
                if (digitalRead(BUTTON_UP_PIN) == LOW)
                {
                    settingsIndex--;
                    if (settingsIndex == 0)
                    {
                        settingsIndex = 3;
                    }
                    delay(200);
                }
                if (digitalRead(BUTTON_DOWN_PIN) == LOW)
                {
                    settingsIndex++;

                    if (settingsIndex == 4)
                    {
                        settingsIndex = 1;
                    }
                    delay(200);
                }

                if (settingsIndex == 1)
                {
                    display.setCursor(0, 0);
                    display.print(">Mute");
                    display.setCursor(92, 7);
                    display.setTextSize(1);
                    display.print("...");
                    display.println(muteMessage);
                    display.setCursor(0, 18);
                    display.setTextSize(2);
                    display.print("Standby");
                    display.setCursor(92, 25);
                    display.setTextSize(1);
                    display.print("...");
                    display.print(standbyMessage);
                    display.setCursor(0, 36);
                    display.setTextSize(2);
                    display.print("DevMode");
                    display.setCursor(92, 43);
                    display.setTextSize(1);
                    display.print("...");
                    display.print(devMessage);
                    display.setCursor(82, 56);
                    display.setTextSize(1);
                    display.print("Change");
                    display.drawCircle(123, 59, 4, SH110X_WHITE);
                    display.fillCircle(123, 59, 2, SH110X_WHITE);

                    if (digitalRead(BUTTON_CENTER_PIN) == LOW)
                    {
                        mute = !mute;

                        muteMessage = (mute) ? "On" : "Off"; //update the muteMessage string
                        delay(200);
                    }
                }
                else if (settingsIndex == 2)
                {
                    display.setCursor(0, 0);
                    display.print("Mute");
                    display.setCursor(92, 7);
                    display.setTextSize(1);
                    display.print("...");
                    display.println(muteMessage);
                    display.setCursor(0, 18);
                    display.setTextSize(2);
                    display.print(">Standby");
                    display.setCursor(92, 25);
                    display.setTextSize(1);
                    display.print("...");
                    display.print(standbyMessage);
                    display.setCursor(0, 36);
                    display.setTextSize(2);
                    display.print("DevMode");
                    display.setCursor(92, 43);
                    display.setTextSize(1);
                    display.print("...");
                    display.print(devMessage);
                    display.setCursor(82, 56);
                    display.setTextSize(1);
                    display.print("Change");
                    display.drawCircle(123, 59, 4, SH110X_WHITE);
                    display.fillCircle(123, 59, 2, SH110X_WHITE);

                    if (digitalRead(BUTTON_CENTER_PIN) == LOW)
                    {
                        standby++;
                        if (standby == 4)
                        {
                            standby = 0;
                        }

                        if (standby == 0)
                        {
                            standbyMessage = "Off";
                        }
                        else if (standby == 1)
                        {
                            standbyMessage = "10m";
                        }
                        else if (standby == 2)
                        {
                            standbyMessage = "1hr";
                        }
                        else if (standby == 3)
                        {
                            standbyMessage = "3hr";
                        }
                        delay(200);
                    }
                }
                else if (settingsIndex == 3)
                {
                    display.setCursor(0, 0);
                    display.print("Mute");
                    display.setCursor(92, 7);
                    display.setTextSize(1);
                    display.print("...");
                    display.println(muteMessage);
                    display.setCursor(0, 18);
                    display.setTextSize(2);
                    display.print("Standby");
                    display.setCursor(92, 25);
                    display.setTextSize(1);
                    display.print("...");
                    display.print(standbyMessage);
                    display.setCursor(0, 36);
                    display.setTextSize(2);
                    display.print(">DevMode");
                    display.setCursor(92, 43);
                    display.setTextSize(1);
                    display.print("...");
                    display.print(devMessage);
                    display.setCursor(82, 56);
                    display.setTextSize(1);
                    display.print("Change");
                    display.drawCircle(123, 59, 4, SH110X_WHITE);
                    display.fillCircle(123, 59, 2, SH110X_WHITE);

                    if (digitalRead(BUTTON_CENTER_PIN) == LOW)
                    {
                        dev = !dev;

                        devMessage = (dev) ? "On" : "Off";
                        delay(200);
                    }
                }

                display.setCursor(9, 56);
                display.setTextSize(1);
                display.println("Back");
                display.fillTriangle(6, 56, 6, 62, 0, 59, SH110X_WHITE);
            }
            else
            {
                display.setCursor(0, 0);
                display.setTextColor(SH110X_WHITE);
                //display.println("Invalid selection");
            }
        }
        display.display();
    }
    else
    {
        while (!sht4.begin())
        {//Check Sensor initialization
            display.setTextSize(1);
            display.setTextColor(SH110X_WHITE);
            display.setCursor(0, 0);
            display.clearDisplay();
            delay(1000);
            display.print("Please Connect Sensors!");
            display.display();
        }
    }
}
