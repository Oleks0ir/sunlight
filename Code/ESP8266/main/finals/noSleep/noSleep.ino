/*
noSleep - version of the code that does not include processes based on timers or sleep.
Thus does not include logging, information saving, last-time-updated, measure/sleep-step related processes

TODO:
  [◄] add user verification at /pullconfig
  [◄] add config.json rewrite at /updateconfig
  [√] research file transmittion to ESP


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

#include <Wire.h>
#include <SPI.h>

#include <I2C_RTC.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define FORMAT_LITTLEFS_IF_FAILED true

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define Switch 14 //D5  MOSFET control pin (V_MES on Scheme) 
// Switches between Open Curcuit an ~1kOm Resistor 

#define Testled 12

unsigned long previousMillis = 0;  // Store last time LED was updated
const long blinkInterval = 1000;  

#include <OneWire.h>
#include <DallasTemperature.h>
#define Mult1 5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(80);
static DS3231 RTC;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;  
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

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

bool networkSuccessefull = true;

int stoppedOnNetwork = 0;
int lastLog = 0;
int WAIT_FOR_CONNECTION; //not implemented in noSleep
bool requestFlag = false;
bool allowCallback = false;
bool FilegateOpen = false;
bool ledState = LOW;    // Variable to store LED state

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
  

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.clearDisplay();

  LittleFS.begin();
  WiFi.mode(WIFI_STA);
  RTC.begin();
  sensors.begin();
  pinMode(Switch, OUTPUT);
  pinMode(Testled, OUTPUT);

  digitalWrite(Switch, LOW);
  digitalWrite(Testled, LOW);

  config = JsonConfig();

  display.println("Connection start...");
  display.display();

  connectToNetwork();

  if (networkSuccessefull){
    
    Serial.println("\nFinished connection");
    setupServer();
    Serial.println("\nFinished server setup");

    Serial.println("ESP ready to work!");
    
    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());
    display.println("IP Adress: ");
    display.println(WiFi.localIP());
  }
  else{
    display.println("Connection Failed. Try reconnecting");
  }

  display.display();
}



void loop() {
 // Non-blocking LED Blink
  unsigned long currentMillis = millis();  // Get current time
  if (currentMillis - previousMillis >= config["statics"][0]["timers"][0]["MEASURE_STEP"].as<int>()*1000) {
    previousMillis = currentMillis;       // Update the last time the LED was toggled
    ledState = !ledState;                 // Toggle LED state
    updateLog();
    if(ledState){
      digitalWrite(Testled, HIGH);   
    }    // Set LED to new state
    else{
      digitalWrite(Testled, LOW);   
    }
    Serial.println(ledState ? "LED ON" : "LED OFF");
  }
  yield();
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

void finishConnectionSession(){
  //Serial.println("finishConnectionSession()");
  display.println("No networks found");
  networkSuccessefull = false;
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

      display.print(network["SSID"].as<const char*>());

      display.println("<[X]");

      Serial.println("  <== WiFi Failed!");
      networkSuccessefull = false;
      failureBlink();
      continue;
    }
    else {
      Serial.print(network["SSID"].as<const char*>());
      Serial.print(" : ");
      Serial.print(network["PASSWORD"].as<const char*>());

      Serial.print("  <== Wifi Succeeded");

      display.print(network["SSID"].as<const char*>());

      display.println("<[S]");
      networkSuccessefull = true;

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

          Serial.println((user.id));
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
    Serial.print("\n=== USER CLOSED GATE ===");
    FilegateOpen = false;
    request->send(200);
  });
  
  server.on("/updateconfig", HTTP_POST, [](AsyncWebServerRequest *request){

    request->send(200, "text/plain", "none");
  });  

  server.on("/forceReadVoltage", HTTP_GET, [](AsyncWebServerRequest *request){
    String V[2] = {readVoc(), readV_lowRes()};

    String response = "[" + V[0] + "," + V[1] + "]";

    request->send(200, "application/json", "{\"voltage\":["+response+"]}");
  });

  server.on("/forceReadTemp", HTTP_GET, [](AsyncWebServerRequest *request){

    String response = "[" +Get_hexTemp()+"]";

    request->send(200, "application/json", "{\"temperature\":["+response+"]}");
  });

  server.on("/forceUpdateLog", HTTP_GET, [](AsyncWebServerRequest *request){
    
    String resp = updateLog();

    request->send(200, String(), resp);
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

String updateLog(){
  String line;

  line = String(RTC.getYear())+"-"+String(RTC.getMonth())+"-"+String(RTC.getDay())+" "+String(RTC.getHours())+":"+String(RTC.getMinutes())+":"+String(RTC.getSeconds());
  line += " " + String(readVoc()) + "|" + String(readV_lowRes());
  line += " " + String(Get_hexTemp()) + "\n";

  Serial.print("\n line created. Writing line: \n");
  
  writeToFile("logDay.log", line);
  
  return line;
}

String readVoc(){
  unsigned int value;
  digitalWrite(Switch, LOW);
  delay(100);
  value = analogRead(A0);
  Serial.print("\nVoltage OC  = " + String(value, HEX) + " | " + String(value*3.22265/1000) + "V\n");
  //Serial.println((int)(value*100));
  return String((int)(value), HEX);
}

String readV_lowRes(){
  unsigned int value;
  digitalWrite(Switch, HIGH);
  delay(5);
  value = analogRead(A0);
  Serial.print("\nVoltage LRES = " + String(value, HEX) + " | " + String(value*3.22265) + " mV\n");
  //Serial.println((int)(value*100));
  return String((int)(value), HEX);
}

String Get_hexTemp(){
  /*sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float value = temperatureC;
  Serial.println((int)(value*100));
  return String((int)(value*100), HEX);*/
  return "96c";
}