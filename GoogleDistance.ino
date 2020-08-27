#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
//needed for library (Wifi Manager)
#include <Hash.h>
#include "HTTPSRedirect.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#ifndef STASSID

const int trigPin = 5; // Trigger Pin of Ultrasonic Sensor D1
const int echoPin = 4; // Echo Pin of Ultrasonic Sensor D2


/*
  #define STASSID "Mark-PC_Mobile"
  #define STAPSK  "2qms-2jfl-bjzr"
*/
#define STASSID "phantomranch";
#define STAPSK "theranch";
#endif

HTTPSRedirect* client = nullptr;

const char* ssid = STASSID;
const char* password = STAPSK;

char serverName[] = "script.google.com";
const int httpsPort = 443;

char deviceName[40];

const unsigned long milliDay = 86400000/2;


void setup() {
  Serial.begin(115200); // Starting Serial Terminal
  Serial.println("Code Started");
  delay(1000);
  setupWifi();


  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  Serial.println("Starting measurement");

  long duration, inches, cm;
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  



  duration = pulseIn(echoPin, HIGH);
  cm = microsecondsToCentimeters(duration);
  Serial.println("Distance is " + cm);
//  delay(10000);
  sendPost(cm);
}

void loop() {
  //  Restarts esp daily
//    long duration, inches, cm;
//    digitalWrite(trigPin, LOW);
//  delayMicroseconds(2);
//  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(10);
//  digitalWrite(trigPin, LOW);
//
//  duration = pulseIn(echoPin, HIGH);
//  cm = microsecondsToCentimeters(duration);
//  
//  delay(2000);
//  Serial.println(cm);
  
  if (millis() >= milliDay) {
    ESP.restart();
  }

}

void sendPost(float cm) {
  //Setup the https redirect

  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(serverName, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(serverName);
    Serial.println("Exiting...");
    return;
  }
  String hash = getHash();
  Serial.println(hash);
  String url = String("/macros/s/AKfycbzNYkUzQMsbD9WnKEZg2hQ6Ql4Y6cV2h9Fm_Vw1vqKMp9-Ukg/exec?distance=");
  url.concat(String(cm) + "&key=" + hash);
  Serial.println(url);
  client ->GET(url, serverName);
  Serial.println(String("Distance ") + cm +  " recorded");
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

String getHash() {
  return sha1(WiFi.macAddress() + "::" + ESP.getChipId());
}


bool setupWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Prepping Wifi Config of ");
    Serial.println(deviceName);
    WiFiManagerParameter custom_deviceName("Device Name", "Device name", deviceName, 40);

    char chipId[30];
    snprintf(chipId, 30, "ESP%i", ESP.getChipId());
    Serial.println(chipId);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();

    //add all your parameters here
    wifiManager.addParameter(&custom_deviceName);

    //set custom ip for portal
    //    wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //    if (!wifiManager.startConfigPortal(chipId ,"configure")) {
    //      Serial.println("failed to connect and hit timeout");
    //      delay(3000);
    //      //reset and try again, or maybe put it to deep sleep
    //      ESP.reset();
    //      delay(5000);
    //    }
    wifiManager.autoConnect(chipId, "password");


    //read updated parameters
    strcpy(deviceName, custom_deviceName.getValue());

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    return true;
  }
  else {
    return true;
  }
}
