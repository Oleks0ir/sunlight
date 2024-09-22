/*TODO:
  [] Reconnection timer!
  [◙] DEEPSLEEP after datacall!
  []  
  [◙] Failure blinker


√ = Alt + 251
◙ = Alt + 10
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

#include "LittleFS.h"
#include <ArduinoJson.h>
#include "OLED.h"

OLED display(12,14);

AsyncWebServer server(80);

const char* ssid = "Magical device";
const char* password = "groob123";
/*
const char* ssid = "Area51";
const char* password = "DasIstGeheim321!";
*/
const char* PARAM_MESSAGE = "message";
const char* PARAM_HOST = "host";

const char* configPath  = "/config.json";

String previous_message = "None"; 
/*
String hostname;
int port;
*/
struct config {
  int MEASURE_STEP;
  int NETWORK_STEP:
  int ALLOW[3];
} config;

struct network{
  const char* ssid;
  const char* password;
} network;

struct client{
  int MEASURE_STEP;
  int NETWORK_STEP;
  int allow[3];
}

void notFound(AsyncWebServerRequest *request) {
  Serial.println("Gotcha");
    request->send(404, "text/plain", "Not found");
}

void failureBlink(){
  while (1==1){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      Serial.println("Gol");
  }
}

void loadConfiguration() {
    //Starting the LittleFS system:
    if(!LittleFS.begin()){
      Serial.println("LittleFiles.begin() failed");
      return;
    }

  //opening and checking the config file
    File file = LittleFS.open(configPath, "r");
    
    if(!file){
      Serial.println("Failed to open file for reading");
      return;
    }

  // Allocate a temporary JsonDocument
  JsonDocument doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);

  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config

  strlcpy(hostname,                  // <- destination
          doc["hostname"],  // <- source
          sizeof(hostname));         // <- destination's capacity
 
  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}


void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);

  //loading of the configuration

    display.begin();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("\nWiFi Failed!");
      failureBlink();
      return;
    }

    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "test");
    });

    server.on("/admin", HTTP_GET, [](AsyncWebServerRequest* request){
      request->send(200, "text/plain", "welcome, admin! [NONSPEC]");
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request){
      

      request->send(200, "text/plain", "data");
    });

    server.on("/update", HTTP_POST, [](AsyncWebServerRequest* request){
      String message, host;
      
      if (request->hasParam(PARAM_MESSAGE, true)) {
          message = request->getParam(PARAM_MESSAGE, true)->value();
          host = request->getParam(PARAM_HOST, true)->value();
          
      } else {
          message = "ERROR";
      }
      
      request->send(200, "application/json", "{\"new_message\":\"" + message + "\",\"old_message\":\"" + previous_message + "\", \"host\":\"" + host + "\"}");
      previous_message = message;
    });

    server.onNotFound(notFound);

    server.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
}
