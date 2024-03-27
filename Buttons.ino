/*
Wiring Guide:
Find ground pin using multimeter. Will be in one of the corners.
Ground pin goes to arduino ground.
Other pins will have two outputs. A resistor(10k) and digital pins on arduino.
The resistor side will connect to 3v on arduino.

Digital Pins used: 13, 12, 27, 33, 15
Order can be changed depending on final cirucit for ease of wiring
*/

#define BUTTON_PIN 13
//#define BUTTON_PIN 12
//#define BUTTON_PIN 27
//#define BUTTON_PIN 33
//#define BUTTON_PIN 15

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(115200);
}

void loop() {
//Low means pressed for some reason but it works
if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button was pressed");
    delay(250);
}


}
