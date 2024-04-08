/*
one side connects to 3v
other goes to 10k which goes to ground
side not connected to 3v goes to a0 or any analog pin





*/
double normal = 20; //0-20% threshold for normal pressure
double infection = 50; //21-50% threshold for risk of infection
double tdamage = 70; // 51-70% threshold for risk of tissue damage
int fsrPin = A0;
double value = 0;
double percent = 0;     // the FSR and 10K pulldown are connected to a0
    // the analog reading from the FSR resistor divider

unsigned long lastA = 0; //time from last alert
const unsigned long interval = 21600000; //6 hour interval update

void setup(void) {
    pinMode(fsrPin, INPUT);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // We'll send debugging information via the Serial monitor
  Serial.begin(115200);   
}
 
void loop() {
    unsigned long currentTime = millis();            //gets current time
    value = analogRead(fsrPin); //reads pin    
    percent = map(value, 0, 1023, 0 ,100); //way to change analog reading to percentage
    

       // if(percent <= normal){ //if  percentage is in certain threshold then it will output the correct message
      //  display.println("Normal Range");

        if(percent <= infection && percent>= normal){ 
          display.println("Risk of Infection");
          display.println("Pressure off by");
          display.print(percent-normal);
          display.print("% at: ");
        showTime(currentTime);

        }else if(percent <= tdamage && percent>= infection){
          display.println("Risk of Tissue Damage");
          display.println("Pressure off by");
          display.print(percent-normal);
          display.print("% at: ");
        showTime(currentTime);
        }
        if(currentTime  - lastA >= interval){ // checks if 6 hours has passed
            lastA = currentTime;
            sixH(currentTime); //calls to 6 hour threshold function
        }
        delay(1000);
        display.display();
     }

void showTime(unsigned long currentTime) { // gets time to look right
  unsigned long hours = currentTime / 3600000;
  unsigned long minutes = (currentTime % 3600000) / 60000;
  
  if (hours < 10) {
      display.print("0");
  }
    display.print(hours);
    display.print(":");
  if (minutes < 10) {
      display.print("0");
  }
    display.print(minutes);
}

void sixH(unsigned long currentTime){ //function to check every 6 hours
  if (percent <= normal) { 
      display.println("Normal Range");
  } else if (percent <= infection && percent >= normal) {
      display.println("Risk of Infection");
      display.print("Pressure off by ");
      display.print(percent-normal);
      display.print("% at:");
    showTime(currentTime);
  } else if (percent <= tdamage && percent >= infection) {
      display.println("Risk of Tissue Damage");
      display.print("Pressure off by ");
      display.print(percent-normal);
      display.print("% at:");
    showTime(currentTime);
  }
    delay(1000);
    display.display();
}
