/*
This code makes possible to test an ESP which is connected to an DS3231 RTC

Functions:
  fetch Time:
    returns actual current time on RTC
  
  current_time:
    requires User to send {"currentTime": "YYYY-MM-DD HH:MM:SS"}
    updates RTC time to time of user

TODO:
  [◄] add finishConnectionSession (noSleep relevant)

  [◄] = alt +17
  [√] = alt + 251
*/

#include <Wire.h>
#include <I2C_RTC.h>

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

static DS3231 RTC;

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

  RTC.begin();

  config = JsonConfig();

  connectToNetwork();
  Serial.println("\nFinished connection");
  setupServer();
  Serial.println("\nFinished server setup");

  Serial.println("ESP ready to work!");
  
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  // put your main code here, to run repeatedly:

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

bool TimeDiff(String a_str, String b_str) {
  // result false when a earlier than b, true when a later than b
  char a_char[a_str.length() + 1];  // Allocate space for null terminator
  a_str.toCharArray(a_char, sizeof(a_char));
  char b_char[b_str.length() + 1];  // Allocate space for null terminator
  b_str.toCharArray(b_char, sizeof(b_char));

  tmElements_t tm_a = createElements(a_char);
  unsigned long a = makeTime(tm_a);
  
  tmElements_t tm_b = createElements(b_char);
  unsigned long b = makeTime(tm_b);

  return a > b;  // Return true if a is later than b
}

int selectFiles(String lastTimeVisitString){
  
  int lastRespLog = -1;
  JsonDocument logInfoJson_n = logInfoToJson();
    Serial.print("\n => Data selection");
  
  for(lastRespLog; lastRespLog<4; lastRespLog++){
    if(TimeDiff(lastTimeVisitString, logInfoJson_n[lastRespLogList[lastRespLog]][1]["timespan"].as<String>())){
      
      break;
    }
  }
    Serial.print(" - selected from > ");
    Serial.print(lastRespLog);

    return lastRespLog;
  
}

tmElements_t createElements(const char* str) {
  sscanf(str, "%d-%d-%d %d:%d:%d", &Year, &Month, &Day, &Hour, &Minute, &Second);

  tm.Year = CalendarYrToTm(Year);
  tm.Month = Month;
  tm.Day = Day;
  tm.Hour = Hour;
  tm.Minute = Minute;
  tm.Second = Second;

  return tm;
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

void setupServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "test");
  });

  server.on("/current_time", HTTP_POST, [](AsyncWebServerRequest *request){
    
    int status = 200;
    Serial.print("\n === USER REQUESTED TIME UPDATE ==> ");

    if (request->hasParam(PARAM.currentTime, true)) {

      Serial.print("ALLOWED");

      String Time_str = request->getParam(PARAM.currentTime, true)->value();

      char Time_char[Time_str.length() + 1];  // Allocate space for null terminator
      Time_str.toCharArray(Time_char, sizeof(Time_char));


      updateTime(Time_char);
          
    } else {
      status = 401;
    }

    request->send(status);
  });

  server.on("/fetchTime", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = "";
    response = String(RTC.getYear())+"-"+String(RTC.getMonth())+"-"+String(RTC.getDay())+" "+String(RTC.getHours())+":"+String(RTC.getMinutes())+":"+String(RTC.getSeconds());
    request->send(200, "text/plain", response);
  });

  server.onNotFound(notFound);

  server.begin();
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



int searchlastlog(String lastTimeVisit){
  Serial.print("\n+++ FILE PREPARATION");
  Serial.print("\n  => LTV =");
  Serial.print(lastTimeVisit);
  
  int selectedFiles = selectFiles(lastTimeVisit);
  
  Serial.print("\n+++ FILES FOUND");
  return selectedFiles;
}

void failureBlink(){

  Serial.println("Failure");

}

void finishConnectionSession(){
  Serial.println("finishConnectionSession()");
}

inline String BoolToString(bool b)
{
  return b ? "true" : "false";
}

void updateTime(const char* StringTime){
  Serial.print("\n User Time: ");
  Serial.print(StringTime);
  tmElements_t Time = createElements(StringTime);
  
  RTC.setDate(Time.Day, Time.Month,Time.Year+1970);
  RTC.setTime(Time.Hour,Time.Minute,Time.Second);

  Serial.print("\n Time updated: ");
  printTime();

}

void printTime(){
  Serial.print(" ");
  Serial.print(RTC.getYear());
  Serial.print("-");
  Serial.print(RTC.getMonth());
  Serial.print("-");
  Serial.print(RTC.getDay());

  Serial.print(" ");

  Serial.print(RTC.getHours());
  Serial.print(":");
  Serial.print(RTC.getMinutes());
  Serial.print(":");
  Serial.print(RTC.getSeconds());
  Serial.println(" ");
}
