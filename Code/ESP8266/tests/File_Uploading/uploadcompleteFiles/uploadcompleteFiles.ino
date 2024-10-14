/*
 This code allows to rewrite existing files on ESP8266 LittleFS
  Protocol:
    1.Open Gate with ID of last log file you will be replacing OR -1 for config.json. /log/logInfo.json is being rewritten every time.
    2.Dump Gate with filename and one line at a time.
    3.Close gate (Get request with no Params)


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

bool FilegateOpen = false;

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

  server.on("/openGate", HTTP_POST,  [](AsyncWebServerRequest *request){
    int status = 401;

    Serial.print("\n === USER HAS REQUESTED GATE OPENING ==> ");
    if (request->hasParam("data", true)) {
      status = 200;
      FilegateOpen = true;
      Serial.print("GRANTED\n");
      String req=request->getParam("data", true)->value();
      wipeFiles(req.toInt());
    }else{Serial.print("DENIED");}

    request->send(status);
  });
  
  server.on("/dunkGate", HTTP_POST,  [](AsyncWebServerRequest *request){
    int status = 401;

    Serial.print("\n === USER DUNKS GATE ==> ");

    if (request->hasParam("filename", true) && request->hasParam("line", true) && FilegateOpen) {
      status = 200;
      Serial.print("GRANTED\n");

      writeToFile(request->getParam("filename", true)->value(), request->getParam("line", true)->value());

    } else if(FilegateOpen){Serial.print("DENIED ON PARAM INCOMPLETE\n");
    } else {Serial.print("DENIED ON CLOSED GATE\n");}

    request->send(status);
  });

  server.on("/closeGate", HTTP_GET, [](AsyncWebServerRequest *request){
    FilegateOpen = false;
    request->send(200);
  });
  

  server.onNotFound(notFound);

  server.begin();
}

void writeToFile(String filename, String line){
  String path = "/log/"+filename;
  File file;
  bool fileExists = LittleFS.exists(path);

  if(fileExists){
    file = LittleFS.open(path, "a");
    Serial.print("\n Appending file with line: \""+line+"\" \n Result -> ");
    
    if (file.print(line)) {
      Serial.println("Message appended");
    } else {
      Serial.println("Append failed");
    }  
  } else{
    file = LittleFS.open(path, "w");
    Serial.print("\n<> Creating file with line: \""+line+"\" \n Result -> ");

    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }
    if (file.print(line)) {
      Serial.println("File written");
    } else {
      Serial.println("Write failed");
    }

  }
  file.close();

}

void wipeFiles(int upTo){
  Serial.println("WIPING FILES");
  String wipePath;
  
  if (upTo>=0){
  for(int i = 0; i<=upTo; i++){
    wipePath = "/log/"+lastRespLogList[i]+".log";
    if (LittleFS.remove(wipePath)) {
      Serial.println(" - "+wipePath+" deleted");
      continue;
    } else {
      Serial.println("###"+wipePath+" delete failed");
    }
  }

  if (LittleFS.remove("/log/logInfo.json")) {
      Serial.println(" - /log/logInfo.json deleted");
    } else {
      Serial.println("###/log/logInfo.json delete failed");
    }
  }else if(upTo = -1){
    if (LittleFS.remove("/config.json")) {
      Serial.println(" - /config.json deleted");
    } else {
      Serial.println("###/config.json delete failed");
    }
  } else{
    Serial.println("FILE OUT OF BOUNDS");
  }


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