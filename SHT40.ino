/*
vin goes to 3v
ground to ground
scl and sda connected

vout pin not connected


*/

#include "Adafruit_SHT4x.h"

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void setup() {
  Serial.begin(115200);

  while (!Serial)
    delay(10);

  Serial.println("Adafruit SHT4x test");//This secton waits for the sensor to boot
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);


  sht4.setPrecision(SHT4X_HIGH_PRECISION);//higher precision takes longer, HIGH MED, LOW


  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater(SHT4X_NO_HEATER);//Apparanetly can heat too?? set to no heater, messes with temp reading

  
}


void loop() {
  sensors_event_t humidity, temp;
  
  uint32_t timestamp = millis();//Start time count
  sht4.getEvent(&humidity, &temp);//Read both temp and humidity
  timestamp = millis() - timestamp;//Time elapsed

  //Both units, choose one
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Temperature: "); Serial.print((temp.temperature*1.8) + 32); Serial.println(" degrees F");


  /*rH definition: Put simply, relative humidity (RH) is a measure of the water vapor content of air.
   More explicitly, it is the amount of water vapor present in air expressed as a percentage (%RH) of
    the amount needed to achieve saturation at the same temperature.
  */
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

//Prints sensor read duration
  //Serial.print("Read duration (ms): ");
  //Serial.println(timestamp);

  delay(1000);
}