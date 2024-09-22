/*
This file gives an overwiew over the simple extraction of a JsonFile stored internally into a global memory of a program. 
This code makes it possible to read all the possible JsonData written in a standart way of config.json
Uses:
  >LittleFS
  >ArdunoJson
*/

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include "FS.h"
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true

const char* fileName = "/config.json"; 
JsonDocument config;

void setup() {
  // Initialisation of the main functions
  Serial.begin(115200);
  Serial.println("Testing begone");
  LittleFS.begin();

  String TestOut;

  config = JsonConfig(); //extracts a config file and creates an internal JsonDocument Object which is accecable from everywhere
  listNetworks();
  listClients();
  /*serializeJsonPretty(config, TestOut);
  Serial.println(TestOut);*/
}

void loop() {
  // put your main code here, to run repeatedly:

}

void listNetworks(){
  JsonArray networksArr = config["network"].as<JsonArray>();
  JsonObject network;
  int networkSize = networksArr.size();
  
  Serial.println("Exploring networks from config: ");

  for (int i=0; i<networkSize; i++) {
    network = networksArr[i].as<JsonObject>();

    Serial.print("< ID: ");
    Serial.print(network["ID"].as<int>());  // Get and print ID as an integer
  
    Serial.print("   SSID: ");
    Serial.print(network["SSID"].as<const char*>());  // Get and print SSID as a string
  
    Serial.print("   PASSWORD: ");
    Serial.print(network["PASSWORD"].as<const char*>());

    Serial.println(" >");
  }
}

void listClients(){
  JsonArray networksArr = config["clients"].as<JsonArray>();
  JsonObject network;
  int networkSize = networksArr.size();
  
  Serial.println("Exploring clients from config: ");

  for (int i=0; i<networkSize; i++) {
    network = networksArr[i].as<JsonObject>();

    Serial.print("< HOST: ");
    Serial.print(network["HOST"].as<const char*>());  // Get and print ID as an integer
  
    Serial.print("   PASSWORD: ");
    Serial.print(network["PASSWORD"].as<const char*>());  // Get and print SSID as a string
  
    Serial.print("   RIGHTS: ");
    Serial.print(network["RIGHTS"].as<String>());

    Serial.println(" >");
  }

}

JsonDocument JsonConfig(){
  File file = LittleFS.open(fileName, "r");
  String JsonSerial = "";
  JsonDocument FullJson;

  if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
    }
  Serial.println("- read from file:");
    while(file.available()){
      JsonSerial += file.readString();
    }

  deserializeJson(FullJson, JsonSerial);
  return FullJson;

}
