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

#define Vin A0
#define pin1 4
#define pin2 5

int collected_data[4]={0,0,0,0};


AsyncWebServer server(80);
/*
const char* ssid = "Magical device";
const char* password = "groob123";
*/
const char* ssid = "Labor2";
const char* password = "DasIstGeheim321!";

const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request) {
  Serial.println("Gotcha");
    request->send(404, "text/plain", "Not found");
}

void alertFailure(){
  while (1==1){
      digitalWrite(2, HIGH);
      delay(200);
      digitalWrite(2, LOW);
      delay(200);
      Serial.println("Gol");
  }
}

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    
    
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("\n!WiFi Failed!");
      alertFailure();
      return;
    }

    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    server.on("/admin", HTTP_GET, [](AsyncWebServerRequest* request){
      request->send(200, "text/plain", "welcome, admin! [NONSPEC]");
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request){
      
      updateData();      
      
      String jsonResponse = "{\"value1\":\"" + String(collected_data[0]) + "\",";
      jsonResponse += "\"value2\":\"" + String(collected_data[1]) + "\",";
      jsonResponse += "\"value3\":\"" + String(collected_data[2]) + "\",";
      jsonResponse += "\"value4\":\"" + String(collected_data[3]) + "\"}";

      request->send(200, "application/json", jsonResponse);
/*
      delay(20)
    
      if (enableDeepsleep){
        Serial.println("going Deepsleep");
        ESP.deepSleep(50e6);
      }
      else{
        enableDeepsleep = true;
      }
      */
    });

    server.onNotFound(notFound);

    server.begin();
}

void setMultiplexer(int adress = 0){
  int adress_in_binary[2];

  if(adress%2 != 0){
    adress_in_binary[1] = 1;
  }
  if ((adress-1)/2 == 1) {
    adress_in_binary[0] = 1;
  }

  digitalWrite(pin1, adress_in_binary[0]);
  digitalWrite(pin2, adress_in_binary[1]);
}

void updateData(){
  int Voltage = 0;
  for (int i=0; i<=3; i++) {
    setMultiplexer(i);
    Voltage = analogRead(Vin);
    collected_data[i] = Voltage;
    Serial.print(String(i)+" > "+ String(Voltage)+" | ");
  }
  Serial.print("\n######\n");
  setMultiplexer(0);
}






void loop() {
  // put your main code here, to run repeatedly:
}
