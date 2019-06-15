#include <ESP8266WiFi.h>
#include "DHT.h"
#include "secrets.h"
#include "ThingSpeak.h"

#define DHTPIN D4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN D1
#define LDR_PIN A0

// WIFI settings
const char* ssid     = "HN_011";
// WIFI password is stored in secrets.h

// Thingspeak settings
String HOST = "api.thingspeak.com";
String PORT = "80";
String AP = "SmartWeatherStation";
String PASS = "";
WiFiClient  client;


const long A = 1000;     //Resistencia en oscuridad en KΩ
const int B = 15;        //Resistencia a la luz (10 Lux) en KΩ
const int Rc = 10;       //Resistencia calibracion en KΩ

void setupWifi()
{
  Serial.println("Configuring WIFI...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(200);
    Serial.print('.');
  }

  Serial.println("OK");
}

void setup()
{
  Serial.begin(9600);
  pinMode(LED_PIN,OUTPUT);
  pinMode(LDR_PIN, INPUT);

  dht.begin();
  setupWifi();
  ThingSpeak.begin(client);
}

void keepAlive()
{
  digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                   // wait for a second
  digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                   // wait for a second
  
}

float getTemperature()
{
  float t = 0;
  t = dht.readTemperature();
  if (isnan(t)){
    Serial.println("ERROR reading Temperature sensor");
    return -999;
  }

  //float hic = dht.computeHeatIndex(t, h, false);

  return t;
}

float getHumidity()
{
  float h = 0;
  h = dht.readHumidity();
  if (isnan(h)){
    Serial.println("ERROR reading Humidity sensor");
    return -999;
  }
  return h;
}

int getLight()
{
  //int ldr = analogRead(LDR_PIN);
  /*int light = 0;
  Serial.println(ldr);
  if ( ldr == 1024) {
    light = ((long)ldr*A*10)/((long)B*Rc*(0,1));
  }else{
    Serial.println((long)ldr*A*10);
    Serial.println((long)B*Rc*(1024-ldr));
    light = ((long)ldr*A*10)/((long)B*Rc*(1024-ldr));
  }
  Serial.print("Light=");
  Serial.println(light);*/

  return analogRead(LDR_PIN);
}

void sendToThingspeak(float t, float h, int is_raining, int light)
{
  Serial.println("Sending data to Thingspeak...");
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, is_raining);
  ThingSpeak.setField(4, light);

  ThingSpeak.setStatus("OK");

  int response = ThingSpeak.writeFields(THINGSPEAK_CHANNEL, THINGSPEAK_WRITE_APIKEY);
  if(response == 200){
    Serial.println("Channel updated successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(response));
  }
}

void loop() 
{
  keepAlive();
  float t = getTemperature();
  float h = getHumidity();
  int light = getLight();
  
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  //Serial.print("Heat index: ");
  //Serial.print(hic);
  //Serial.print(" *C ");
  Serial.print("Light: ");
  Serial.print(light);
  Serial.print("\n");

  if (t == -999 || h == -999)
  {
    delay(5000); //Waiting for DHT to start
  }else{
    sendToThingspeak(t,h,0,light);
    delay(30000);
  }

  
}
