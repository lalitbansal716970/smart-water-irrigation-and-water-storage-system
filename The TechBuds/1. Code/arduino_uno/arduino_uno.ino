#include <SoftwareSerial.h>
#include <Servo.h>
SoftwareSerial ArduinoUno(7,8);
Servo myservo;
int pos = 0;
String f;
void setup() {
  Serial.begin(9600);
  ArduinoUno.begin(115200);
  pinMode(3,OUTPUT);
  pinMode(2,INPUT);
  myservo.attach(9);
}

void loop() {
digitalWrite(3,HIGH);
delay(10);
digitalWrite(3,LOW);
int lp = pulseIn(2,HIGH);
int d = (lp/2)*0.0343;
if(d<=10) {
  Serial.println(d);
  Serial.print("less"); 
  for(pos=0;pos<=180;pos++) {
    myservo.write(pos);
    delay(10); 
  } 
}
else {
  Serial.println(d);
  Serial.print("more");  
  for(pos=180;pos<=0;pos--) {
    myservo.write(pos);
    delay(10);
  } 
}
f = String(d);
Serial.println("------------------------------------");
Serial.print("\t Distance - "+String(d) + "\n");
Serial.println("------------------------------------");
ArduinoUno.print(f);
delay(2000);
}
