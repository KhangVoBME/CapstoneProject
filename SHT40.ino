/*
vin goes to 3v
ground to ground
scl and sda connected

vout pin not connected
*/

#include "Adafruit_SHT4x.h"
#include <Adafruit_SSD1306.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define temperature and humidity thresholds
#define NTEMP_MIN 97.7   // Normal temp: 97.7F - 99.5F
#define NTEMP_MAX 99.5
#define ETEMP_MIN 99.7   // Elevated temp: 99.7F - 101.3F
#define ETEMP_MAX 101.3
#define DTEMP_MIN 101.5  // Dangerous temp: Above 101.3F
#define NH_MIN 50.0      // Normal humidity: 50-69%
#define NH_MAX 79.0

unsigned long lastA = 0; // Time from last alert
const unsigned long interval = 21600000; // 6 hour interval update

void setup() {
  Serial.begin(115200);

  while (!Serial)
    delay(10);

  Serial.println("Adafruit SHT4x test"); // This section waits for the sensor to boot
  if (!sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1)
      delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  sht4.setPrecision(SHT4X_HIGH_PRECISION); // higher precision takes longer, HIGH MED, LOW

  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater(SHT4X_NO_HEATER); // Apparently can heat too?? set to no heater, messes with temp reading

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize display
}

void loop() {
  unsigned long currentTime = millis(); // Update current time
  
  sensors_event_t humidity, temp;
  
  uint32_t timestamp = millis();  // Start time count
  sht4.getEvent(&humidity, &temp); // Read both temp and humidity
  timestamp = millis() - timestamp; // Time elapsed

  // Check temperature ranges and output notification to display
  if ((temp.temperature * 1.8 + 32) >= ETEMP_MIN && (temp.temperature * 1.8 + 32) <= ETEMP_MAX) {
    display.println("Elevated temperature level: ");
    display.print(temp.temperature * 1.8 + 32);
    display.print("°F");
  } else if ((temp.temperature * 1.8 + 32) >= DTEMP_MIN) {
    display.println("Dangerous temperature level: ");
    display.print(temp.temperature * 1.8 + 32);
    display.print("°F");
  }

  // Check humidity ranges and output notification to display
  if (humidity.relative_humidity > NH_MAX || humidity.relative_humidity < NH_MIN) {
    display.println("Dangerous Humidity Level: ");
    display.print(humidity.relative_humidity);
    display.print("%");
  }

  // Check if it's time for a periodic notification
  if (currentTime - lastA >= interval) { // checks if 6 hours has passed
    lastA = currentTime;
    sixH_SHT40(temp, humidity); // Calls to 6 hour threshold function
  }
  
  delay(1000);
  display.display();
}

void sixH_SHT40(sensors_event_t temp, sensors_event_t humidity) {
  if ((temp.temperature * 1.8 + 32) >= NTEMP_MIN && (temp.temperature * 1.8 + 32) <= NTEMP_MAX) {
    display.println("Normal temperature level: ");
    display.print(temp.temperature * 1.8 + 32);
    display.print("°F");
  } else if ((temp.temperature * 1.8 + 32) >= ETEMP_MIN && (temp.temperature * 1.8 + 32) <= ETEMP_MAX) {
    display.println("Elevated temperature level: ");
    display.print(temp.temperature * 1.8 + 32);
    display.print("°F");
  } else if ((temp.temperature * 1.8 + 32) >= DTEMP_MIN) {
    display.println("Dangerous temperature level: ");
    display.print(temp.temperature * 1.8 + 32);
    display.print("°F");
  }

  // Check humidity ranges and output notification to display
  if (humidity.relative_humidity > NH_MAX || humidity.relative_humidity < NH_MIN) {
    display.println("Dangerous Humidity Level: ");
    display.print(humidity.relative_humidity);
    display.print("%");
  } else if (humidity.relative_humidity >= NH_MIN && humidity.relative_humidity <= NH_MAX) {
    display.println("Normal Humidity: ");
    display.print(humidity.relative_humidity);
    display.print("%");
  }
  
  delay(1000);
  display.display();
}
