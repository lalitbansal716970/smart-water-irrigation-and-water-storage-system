//Including Libraries
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <PubSubClient.h>
#include <Servo.h>
Servo myservo;
//Setting Hotspot Pwd and SSID
const char* ssid = "Redmi Not 5 pro";
const char* password = "123456789";

//Declaring Sensors object and varibles globally.......
SoftwareSerial NodeMCU(D5,D8);
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); //Defining the pin and the dhttype
Adafruit_BMP085 bmp;
#define I2C_SCL D6
#define I2C_SDA D7
float dst,bt,bp,ba,t,h,dp;
String distance;
bool bmp085_present=true;
int sense_Pin = A0; // sensor input at Analog pin A0
int value = 0,value1;

//Device Credentials
#define ORG "gm69yw"
#define DEVICE_TYPE "iotlalit"
#define DEVICE_ID "123456789"
#define TOKEN "lalit123456"

// Default values of IBM Cloud in MQTT Protocol.....
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/evt/Data/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

// WIFI Connection to connect it through mqtt
WiFiClient wifiClient;
PubSubClient client(server, 1883,wifiClient);
// Void setup Code to be run once only....
void setup() {
  Serial.begin(9600); //Baud Rate 115200
  NodeMCU.begin(115200);
  pinMode(D5,INPUT);//rx
  pinMode(D8,OUTPUT);//tx
  myservo.attach(9);
  dht.begin(); // DHT Sensor Begin
  Wire.begin(I2C_SDA, I2C_SCL); // Setting BMP 180 GPIO Pins in Nodemcu
  Serial.print("Connecting to "); //Priting Connecting to SSID_name
  Serial.print(ssid);
  WiFi.begin(ssid, password); // WIFI is trying to connect
  // if wifi is not connected print dots(.) with delay of 0.5sec
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); 
  }
  Serial.println("");// new line
  Serial.print("WiFi connected, IP address: ");// when wifi is connected the print wifi connected IP Address: wifi.localIP()
  Serial.println(WiFi.localIP());
}
//Void loop code to repeat always.....
void loop() {

  delay(2000);
  //Serial communication btw arduino and nodemcu.........

  String content = "";
  char character;
  while(NodeMCU.available()) {
      character = NodeMCU.read();
      content.concat(character);
  }
  if (content != "") {
    Serial.println("Distance: " + content);
  }
  distance = content;
  //..........................................................
  // if bmp wont work then print below statement
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    return;
  }
   h = dht.readHumidity(); // Humidity by DHT11
   t = dht.readTemperature(); // Temperature by DHT11
  if (isnan(h) || isnan(t)) { // if dht11 don't work print below statement
    Serial.println("Failed to read from DHT sensor!");
    //return;
  }
  double gamma = log(h/100) + ((17.62*t) / (243.5+t)); //The dew point is the temperature where water vapour condenses into liquid water.
  dp = 243.5*gamma / (17.62-gamma);// dp means dew point - he dew point shows the amount of moisture in the air. The higher the dew point is,
  bp =  bmp.readPressure()/100;// this is pressure in pascal //the higher the level of moisture in the air at a given temperature.
  ba =  bmp.readAltitude(); // altitude
  bt =  bmp.readTemperature(); // Temperature in BMP180 USE either of one BMP Temperature or DHT11 Temperature...
  dst = bmp.readSealevelPressure()/100; // Sea level Pressure

  // Moisture sensor code
  Serial.print("MOISTURE LEVEL : ");
  value= analogRead(sense_Pin);
  value1 = value/10;
  if(value1<50) {
    Serial.println("Plant is Wet. no need to provide water!!");
    for(pos=0;pos<=180;pos++) {
      myservo.write(pos);
      delay(10); 
    }
  }
  else {
    Serial.println("Plant is Wet. no need to provide water!!");
    for(pos=180;pos<=0;pos--) {
      myservo.write(pos);
      delay(10);
    }
  }
  Serial.println(value1);
  Serial.println("preesure="+String(bp));
  Serial.println("temperature="+String(bt));
  Serial.println("altitude="+String(ba));
  Serial.println("sea preesure="+String(dst));
  Serial.println("dew point="+String(dp));
  Serial.println("Temperature(DHT11)="+String(t) + "\t Humidity="+String(h));
  PublishData(t, h, dp, bp, ba,bt,dst,value1, distance); // publish data by sensors
  if (!client.loop()) { // call function mqttconnect function until completed function be in loop
    mqttConnect();
  }
 // delay(100);
}
void mqttConnect() { // try to connect to ibm device specific server
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); 
    Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
  }
}


void PublishData(float t, float h, float dp, float bp, float ba, float bt, float dst, int value1, String distance) { // t, h, dp, bp, ba, bt, dst, value1 as arguments
  if (!!!client.connected()) { // again try to reconnect
    Serial.print("Reconnecting client to ");
    Serial.println(server);
    while (!!!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
  }
  // adding all to one url
  String payload = "{\"d\":{\"temp\":";
  payload += t;
  payload +="," "\"humd\":";
  payload += h;
 /* payload +="," "\"dew\":";
  payload += dp;
  payload +="," "\"pressr\":";
  payload += bp;
  payload +="," "\"altitude\":";
  payload += ba;
  payload +="," "\"bmp_temp\":";
  payload += bt;
  payload +="," "\"sea_pressure\":";
  payload += dst;*/
  payload +="," "\"moist\":";
  payload += value1;
  payload +=",""\"distance\":";
  payload += distance;
  payload +="}}";
  // printing all payload
  Serial.print("Sending payload: ");
  Serial.println(payload);
  
  if(client.publish(topic, (char*) payload.c_str())) {
    Serial.println("Publish ok");
  } 
  else {
    Serial.println("Publish failed");
  }
  //delay(1000);
}
