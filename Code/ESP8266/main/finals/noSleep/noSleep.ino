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
            Serial.println("\n== Client ACCESS DENIED ==");
            STATUS_CODE = 409;
          }
          else{
            Serial.println("\n== Client ACCESS GRANTED ==");
            allowCallback = true;
          }
      } else {
        Serial.println("\n== CLIENT PARAM REFUSED ==");
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

    Serial.print("\n= USER REQUESTED CALLBACK ==>  ");

    if (request->hasParam(PARAM.CALLBACK, true) and allowCallback) {
          Serial.println("GRANTED");
          requestFlag = true;
          lastLog = searchlastlog(request->getParam(PARAM.CALLBACK, true)->value());
          Serial.print("\n== CALLBACK SESSION ENDED ==");   
      
      } else if (request->hasParam(PARAM.CALLBACK, true) != true){
         Serial.println("DENIED");
          lastLog = -1;
          status = 401;
        Serial.print("PARAM failure");
      }
      else if (!allowCallback){
        Serial.println("DENIED");
          lastLog = -1;
          status = 401;
        Serial.print("CALLBACK not allowed");
      }
      else{
        Serial.println("DENIED");
          lastLog = -1;
          status = 401;
        Serial.print("UNKNOWN FAILURE");
      }

    request->send(status, "application/json", "{\"lastLog\":\""+String(lastLog)+"\"}");
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
  server.on("/getDay", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.print("\n == USER REQUESTED LOG DATA DAY==> ");
    if (lastLog >=0 && requestFlag){
      Serial.println("GRANTED and SENT");
      request->send(LittleFS, logDay, String(), true);
    }else if(!requestFlag){
      Serial.println("DENIED: CALLBACK REQ DENIED");
      request->send(401, "text/plain", "NO REQUEST SUCCEEDED");
    }else if(lastLog==-1){
      Serial.println("DENIED: LOG DENIAL");
      request->send(401, "text/plain", "LOG DENIAL");
    } else {
      Serial.println("DENIED: UNKMOWN FAILURE");
      request->send(401, "text/plain", "UNKNOWN ERROR");
    }
  });

  server.on("/getWeek", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.print("\n == USER REQUESTED LOG DATA WEEK==> ");
    if (lastLog >=1 && requestFlag){
      Serial.println("GRANTED and SENT");
      request->send(LittleFS, logWeek, String(), true);
    }else if(!requestFlag){
      Serial.println("DENIED: CALLBACK REQ DENIED");
      request->send(401, "text/plain", "NO REQUEST SUCCEEDED");
    }else if(lastLog<1){
      Serial.println("DENIED: LOG DENIAL");
      request->send(401, "text/plain", "LOG EXCEEDED");
    } else {
      Serial.println("DENIED: UNKMOWN FAILURE");
      request->send(401, "text/plain", "UNKNOWN ERROR");
    }
  }); 
  
  server.on("/getMonth", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.print("\n == USER REQUESTED LOG DATA MONTH==> ");
    if (lastLog >=2 && requestFlag){
      Serial.println("GRANTED and SENT");
      request->send(LittleFS, logMonth, String(), true);
    }else if(!requestFlag){
      Serial.println("DENIED: CALLBACK REQ DENIED");
      request->send(401, "text/plain", "NO REQUEST SUCCEEDED");
    }else if(lastLog<2){
      Serial.println("DENIED: LOG DENIAL");
      request->send(401, "text/plain", "LOG EXCEEDED");
    }else {
      Serial.println("DENIED: UNKMOWN FAILURE");
      request->send(401, "text/plain", "UNKNOWN ERROR");
    }
  }); 
  
  server.on("/getYear", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.print("\n == USER REQUESTED LOG DATA YEAR==> ");
    if (lastLog >=3 && requestFlag){
      Serial.println("GRANTED and SENT");
      request->send(LittleFS, logYear, String(), true);
    }else if(!requestFlag){
      Serial.println("DENIED: CALLBACK REQ DENIED");
      request->send(401, "text/plain", "NO REQUEST SUCCEEDED");
    }else if(lastLog<3){
      Serial.println("DENIED: LOG DENIAL");
      request->send(401, "text/plain", "LOG EXCEEDED");
    } else {
      Serial.println("DENIED: UNKMOWN FAILURE");
      request->send(401, "text/plain", "UNKNOWN ERROR");
    }
  }); 
  


  server.on("/pullconfig", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.print("\n == USER REQUESTED CONFIG ==> ");
    
    if(request->hasParam(PARAM.id, true)){
      Serial.println("GRANTED and SENT");
      //--------------------------------------------------------------------------------------------------
      request->send(LittleFS, filepar_name, String(), true);

    } else if(request->hasParam(PARAM.id, true)){
      Serial.println("DENIED \n PARAM Failure");
      //--------------------------------------------------------------------------------------------------
      request->send(401);
    } else {
      Serial.println("DENIED \n Unknown failure");
      //--------------------------------------------------------------------------------------------------
      request->send(401);
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
