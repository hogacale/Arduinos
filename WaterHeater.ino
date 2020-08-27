//This program uses a google script which looks at a calendar to determine whether the hill chapel will
//be in use. If the calendar says that it will the script returns yes and if it will not it returns no.
//If the google script gives a yes the D1 port which is connected to the water heater will be turned on.

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
//needed for library (Wifi Manager)
#include <Hash.h>
#include "HTTPSRedirect.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#ifndef STASSID

const int heatPin = 5; // Trigger Pin of Ultrasonic Sensor D1

#define STASSID "Mark-PC_Mobile";
#define STAPSK "2qms-2jfl-bjzr";
//#define _stackSize (6748/4) 
#endif

HTTPSRedirect* client = nullptr;

const char* ssid = STASSID;
const char* password = STAPSK;

char serverName[] = "script.google.com";
const int httpsPort = 443;

char deviceName[40];

const unsigned long milliDay = 86400000;
//const unsigned long milliDay = 40000;
unsigned long curTime = 0;

void setup() {
  Serial.begin(115200); // Starting Serial Terminal
  delay(1000);
  
  setupWifi();

  pinMode(heatPin, OUTPUT);

  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

//  if (sendPost() == true) {
//    digitalWrite(heatPin, HIGH);
//    curTime = millis();
//    Serial.println("Water heater on");
//  }

}

void loop() {
//  Serial.println(millis());
  //  runs the main code every 12 hours and wont rerun it if the heater is on it waits an extra 24 hours
  if (millis() >= (milliDay / 4) && (millis()) >= (curTime + milliDay)) {
    if (sendPost() == true) {
      digitalWrite(heatPin, HIGH);
      curTime = millis();
      Serial.println("Water heater on");
    }
    if((millis()) >= (curTime + milliDay)){
      digitalWrite(heatPin,LOW); 
      Serial.println("Water heater off");
    }
  }
//  This restarts the device every week
  if(millis() >= (milliDay * 7) && millis() >= (curTime + milliDay)){
//    Serial.println("Time: " + cTime);
    ESP.restart();
  }
}

boolean sendPost() {
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
    return false;
  }
  
  String url = String("/macros/s/AKfycbzIUa-ZIFJdzprkYuy87v0HFryxv32c2JrTU8iz31oG6HvGhiA/exec");
  
  client ->GET(url, serverName);
  String result = client ->getResponseBody();
  Serial.println("The server replied with " + result);
  result.trim();
  if (result.equals("yes")) {
    return true;
  }
  else {
    return false;
  }
  //  Serial.println(String("Distance ") + cm +  " recorded");
}


//=============================================================================
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
