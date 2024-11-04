/*
 {TEXT}
TODO:
  [◄] {TASK}

  [◄] = alt +17
  [√] = alt + 251
*/

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include <TimeLib.h>

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include "FS.h"
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true


AsyncWebServer server(80);

const char* filepar_name = "/config.json"; 

const char* logDay = "/log/logDay.log";
const char* logWeek = "/log/logWeek.log";
const char* logMonth = "/log/logMonth.log";
const char* logYear = "/log/logYear.log";
const char* logInfo = "/log/logInfo.json";

const char* logResponse = "/log/logResp.log";

int Year, Month, Day, Hour, Minute, Second;
tmElements_t tm;

JsonDocument config;

int stoppedOnNetwork = 0;
int lastLog = 0;
int WAIT_FOR_CONNECTION; //not implemented in noSleep
bool requestFlag = false;
bool allowCallback = false;

String lastRespLogList[4] = {
  "logDay",
  "logWeek",
  "logMonth",
  "logYear"
};

struct{
  const char* PARAM_MESSAGE = "message";
  const char* PARAM_HOST = "host";
  const char* id = "ID";
  const char* par_name = "HOST";
  const char* par_password = "PASSWORD";
  const char* CALLBACK = "lastTimeVisit";
  const char* datatime = "datatime";
  const char* changesMade= "changesMade";
  const char* newConfig= "newConfig";
  const char* currentTime= "currentTime";
}PARAM;

struct{
  int id;
  String par_name;
  String par_password;
  bool RIGHTS[6];
}user;

void notFound(AsyncWebServerRequest *request) {
  Serial.println("Gotcha");
    request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);

  LittleFS.begin();
  WiFi.mode(WIFI_STA);

  config = JsonConfig();

  connectToNetwork();
  Serial.println("\nFinished connection");
  setupServer();
  Serial.println("\nFinished server setup");

  Serial.println("ESP ready to work!");
  
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

}


void setupServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "test");
  });

  server.on("/uploadConfig", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200);
  });
  

  server.onNotFound(notFound);

  server.begin();
}



JsonDocument JsonConfig(){
  File file = LittleFS.open(filepar_name, "r");
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

JsonDocument logInfoToJson(){
  Serial.print("\n => Accesing logInfo ");
  
  File file = LittleFS.open(logInfo, "r");
  String JsonSerial = "";
  JsonDocument FullJson;

  if(!file || file.isDirectory()){
        Serial.println("- failed to open logInfo.json for reading");
    }
  Serial.print(" - reading logInfo ==>");
    while(file.available()){
      JsonSerial += file.readString();
    }

  Serial.println(" Done");

  deserializeJson(FullJson, JsonSerial);
  return FullJson;

}

void connectToNetwork(){
  JsonArray networksArr = config["network"].as<JsonArray>();
  JsonObject network;
  int networkSize = networksArr.size();
  
  if (stoppedOnNetwork!=666) {
    for (int i=stoppedOnNetwork; i < networkSize; i++) {
    network = networksArr[i].as<JsonObject>();

    WiFi.begin(network["SSID"].as<const char*>(), network["PASSWORD"].as<const char*>());
    
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {

      Serial.print(network["SSID"].as<const char*>());
      Serial.print(" : ");
      Serial.print(network["PASSWORD"].as<const char*>());

      Serial.println("  <== WiFi Failed!");
      failureBlink();
      continue;
    }
    else {
      Serial.print(network["SSID"].as<const char*>());
      Serial.print(" : ");
      Serial.print(network["PASSWORD"].as<const char*>());

      Serial.print("  <== Wifi Succeeded");

      if (i<networkSize-1) {
        stoppedOnNetwork = i;
      }
      else{
        stoppedOnNetwork=666;
      }

      break;
    }
  }
  }
  else{
    Serial.println("List finished");
    stoppedOnNetwork = 0;
    finishConnectionSession();
  }

}

bool checkUser(){
  JsonObject client = config["clients"][user.id].as<JsonObject>();
  Serial.println("LOG>> Rights");
  if (user.par_name == client["HOST"].as<const char*>() and user.par_password == client["PASSWORD"].as<const char*>()) {
    for (int i = 0; i < 6; i++) {
      user.RIGHTS[i] = client["RIGHTS"][i];

      Serial.print("\n>>>"+String(i));
      Serial.print(client["RIGHTS"][i].as<String>());
    }
    return true;
  }
  else{
    Serial.println("\n == USER idENT FAILURE ==");
    return false;
  }
}


void failureBlink(){

  Serial.println("FAILURE ON CONNECTING");

}

void finishConnectionSession(){
  Serial.println("finishConnectionSession()");
}

inline String BoolToString(bool b)
{
  return b ? "true" : "false";
}



void loop() {
  // put your main code here, to run repeatedly:

}