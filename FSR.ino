/*
one side connects to 3v
other goes to 10k which goes to ground
side not connected to 3v goes to a0 or any analog pin





*/
 
int fsrPin = A0;
int value = 0;
int percent = 0;     // the FSR and 10K pulldown are connected to a0
    // the analog reading from the FSR resistor divider
 
void setup(void) {
  // We'll send debugging information via the Serial monitor
  Serial.begin(115200);   
}
 
void loop() {
  value = analogRead(fsrPin) / 4095;
      //percent = (value/4096)*100;
      Serial.println(value);
      //Serial.print(percent - 10);
      //Serial.println("%");
      delay(100); //slow it down a bit
}