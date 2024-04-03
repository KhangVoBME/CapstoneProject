/*
one side connects to 3v
other goes to 10k which goes to ground
side not connected to 3v goes to a0 or any analog pin





*/
int normal = 20; //0-20% threshold for normal pressure
int infection = 50; //21-50% threshold for risk of infection
int tdamage = 70; // 51-70% threshold for risk of tissue damage
int fsrPin = A0;
int value = 0;
int percent = 0;     // the FSR and 10K pulldown are connected to a0
    // the analog reading from the FSR resistor divider

unsigned long lastA = 0; //time from last alert
const unsigned long interval = 21600000; //6 hour interval update

void setup(void) {
  // We'll send debugging information via the Serial monitor
  Serial.begin(115200);   
}
 
void loop() {
    unsigned long currentTime = millis();            //gets current time
    value = analogRead(fsrPin); //reads pin    
    percent = map(value, 0, 1023, 0 ,100); //way to change analog reading to percentage
    

        if(percent <= normal){ //if  percentage is in certain threshold then it will output the correct message
        Serial.println("Normal Range");

        }else if(percent <= infection && percent>= normal){ 
        Serial.println("Risk of Infection");
        Serial.println("Pressure was off by");
        Serial.print(normal-percent);
        Serial.print("% at: ");
        showTime(currentTime);

        }else if(percent <= tdamage && percent>= infection){
        Serial.println("Risk of Tissue Damage");
        Serial.println("Pressure was off by");
        Serial.print(normal-percent);
        Serial.print("% at: ");
        showTime(currentTime);
        }
        if(currentTime  - lastA >= interval){ // checks if 6 hours has passed
            lastA = currentTime;
            sixH(currentTime); //calls to 6 hour threshold function
        }
     }

void showTime(unsigned long currentTime) { // gets time to look right
  unsigned long hours = currentTime / 3600000;
  unsigned long minutes = (currentTime % 3600000) / 60000;
  
  if (hours < 10) {
    Serial.print("0");
  }
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) {
    Serial.print("0");
  }
  Serial.print(minutes);
}

void sixH(unsigned long currentTime){ //function to check every 6 hours
  if (percent <= normal) { 
    Serial.println("Normal Range");
  } else if (percent <= infection && percent >= normal) {
    Serial.println("Risk of Infection");
    Serial.print("Pressure was off by ");
    Serial.print(normal - percent);
    Serial.print("% at:");
    showTime(currentTime);
  } else if (percent <= tdamage && percent >= infection) {
    Serial.println("Risk of Tissue Damage");
    Serial.print("Pressure was off by ");
    Serial.print(normal - percent);
    Serial.print("% at:");
    showTime(currentTime);
  }
}
