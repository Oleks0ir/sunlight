/*
noSleep - version of the code that does not include processes based on timers or sleep.
Thus does not include logging, information saving, last-time-updated, measure/sleep-step related processes

TODO:
  [◄] add user verification at /pullconfig
  [◄] research file transmittion to ESP
  [◄] add config.json rewrite at /updateconfig
  [◄] add collectLogData(){...}
  [◄] add organizeNewData(){...}

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

const char* logResponse = "/log/logResponse.log";

int Year, Month, Day, Hour, Minute, Second;
tmElements_t tm;

JsonDocument config;

int stoppedOnNetwork = 0;
int WAIT_FOR_CONNECTION; //not implemented in noSleep
bool requestFlag = false;
bool allowCallback = false;

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
  File file = LittleFS.open(logInfo, "r");
  String JsonSerial = "";
  JsonDocument FullJson;

  if(!file || file.isDirectory()){
        Serial.println("- failed to open logInfo.json for reading");
    }
  Serial.print("- reading logInfo ==>");
    while(file.available()){
      JsonSerial += file.readString();
    }

  Serial.println(" Done");

  deserializeJson(FullJson, JsonSerial);
  return FullJson;

}

bool TimeDiff(String a_str, String b_str){
// result false when a earlyer then b, true when a later then b
  char a_char[a_str.length()];
  a_str.toCharArray(a_char, a_str.length());
  char b_char[b_str.length()];
  b_str.toCharArray(b_char, b_str.length());

  createElements(a_char);
  unsigned long a = makeTime(tm);
  createElements(b_char);
  unsigned long b = makeTime(tm);

  if(a-b<0){
    return false;
  }else{
    return true;
  }
}

int selectFiles(String lastTimeVisitString){
  int lastRespLog = 0;
  JsonDocument logInfoJson = logInfoToJson();

  for(lastRespLog; lastRespLog<4; lastRespLog++){
    if(TimeDiff(lastTimeVisitString, logInfoJson[lastRespLog].as<String>())){
      break;
    }
  }

    return lastRespLog;
  
}

tmElements_t createElements(const char* str){
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
  server.on("/echopost", HTTP_POST, [](AsyncWebServerRequest *request){
    String message;

    if (request->hasParam(PARAM.PARAM_MESSAGE, true)) {
          message = request->getParam(PARAM.PARAM_MESSAGE, true)->value();
          
      } else {
          message = "ERROR";
      }

    request->send(200, "application/json", "{\"test\":\""+message+"\"}");
  });

  //CALLBACK FUNCTION
  server.on("/callback", HTTP_POST, [](AsyncWebServerRequest *request){
    int STATUS_CODE = 200;
    String id_Buffer, responseStr;
    Serial.print("\n New Client connected: -Param:");

    if (request->hasParam(PARAM.id, true)) {
          Serial.print(" -Confirmed. Writing PARAM >> ");
          id_Buffer = request->getParam(PARAM.id, true)->value();
          Serial.print("+id Buffer");
          user.par_name = request->getParam(PARAM.par_name, true)->value();
          Serial.print(" +par_name");
          user.par_password = request->getParam(PARAM.par_password, true)->value();
          Serial.print(" +par_password");
          user.id = id_Buffer.toInt();
          Serial.println(" @id");

          Serial.println(user.id);
          Serial.println(user.par_name);
          Serial.println(user.par_password);

          if (!checkUser()) {
            Serial.println("== Client ACCESS DENIED ==");
            STATUS_CODE = 409;
          }
      } else {
        Serial.println("== CLIENT PARAM REFUSED ==");
        for (int i=0; i<6; i++) {
          user.RIGHTS[i]=false;
          }
        STATUS_CODE = 401;
      }
    for (int i=0; i<5; i++) {
      responseStr= responseStr + BoolToString(user.RIGHTS[i]) + ",";
    }
    responseStr= responseStr+ BoolToString(user.RIGHTS[5]);

    request->send(STATUS_CODE, "application/json", String("{\"RIGHTS\":[")+responseStr+String("]}"));
  });

  server.on("/setcallback", HTTP_POST, [](AsyncWebServerRequest *request){
    bool message = true;
    int status = 200;

    if (request->hasParam(PARAM.CALLBACK, true) and allowCallback) {
          requestFlag = true;
          prepeareLog(request->getParam(PARAM.CALLBACK, true)->value());    
      } else {
          message = false;
          status = 401;
      }

    request->send(status, "application/json", "{\"success\":\""+BoolToString(message)+"\"}");
  });  
/*
  server.on("/setcallback", HTTP_POST, [](AsyncWebServerRequest *request){
    bool message = true;
    int status = 200;

    if (request->hasParam(PARAM.CALLBACK, true) and allowCallback) {
          requestFlag = true;
      } else {
          message = false;
          status = 401;
      }

    request->send(status, "application/json", "{\"success\":\""+BoolToString(message)+"\"}");
  });  
*/
  server.on("/getdata", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, logResponse, String(), true);
  }); 

  server.on("/pullconfig", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam(PARAM.id, true)){
      //--------------------------------------------------------------------------------------------------
      request->send(LittleFS, filepar_name, String(), true);
    }
  }); 

  server.on("/updateconfig", HTTP_POST, [](AsyncWebServerRequest *request){

    request->send(200, "text/plain", "none");
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
    Serial.println("== USER idENT FAILURE ==");
    return false;
  }
}

void composeResponse(int lastRespLog){
  const char* LogFiles[] = {logDay, logWeek, logMonth, logYear};
  String lines;
  File file;
  
  for(int i = 0; i<=lastRespLog; i++){
    file = LittleFS.open(LogFiles[i], "r");

    while(file.available()){
      lines = file.readStringUntil('\n');
      file.close();
      file = LittleFS.open(logResponse, "a");
      file.println(lines);
      file.close();
      file = LittleFS.open(LogFiles[i], "r");
    }
    file.close();
    
  }
}

void prepeareLog(String lastTimeVisit){
  int selectedFiles = selectFiles(lastTimeVisit);
  composeResponse(selectedFiles);
    
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
