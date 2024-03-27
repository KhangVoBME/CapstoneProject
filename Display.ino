/*
Will have four outputs. SDA, SCL, Vin, Ground

When running this code itll ask to install libraries, make sure to do so.
*/


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);



void setup() {
  Serial.begin(115200);


  delay(250); // wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default

  

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();//splash screen
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();


  display.setRotation(1);
  
display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);


display.print("Hello World!");
display.display();


  // text display tests
  /*display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.print("Connecting to SSID\n'adafruit':");
  display.print("connected!");
  display.println("IP: 10.0.1.23");
  display.println("Sending val #0");
  display.display(); // actually display all of the above
  */
}

void loop() {
  /*if(!digitalRead(BUTTON_A)) display.print("A");
  if(!digitalRead(BUTTON_B)) display.print("B");
  if(!digitalRead(BUTTON_C)) display.print("C");
  delay(10);
  yield();
  display.display();
  */
}