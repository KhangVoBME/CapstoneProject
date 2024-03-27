/*
Buzzer has no polarity
one pin goes to 100 ohm resitor to pin13(A12)
other goes to ground

Vin not needed
*/




const int buzzer = A12;

void setup(){
   //tone range: 100(quiet) - 500(practical) - 5k(ok kinda) - 10k(noise produced but not practical)
  pinMode(buzzer, OUTPUT);
  tone(buzzer, 7000);
  delay(1000);//length of tone in mS
  noTone(buzzer);
}

void loop(){
}