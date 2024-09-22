//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char* ssid = "Magical device";
const char* password = "groob123";

const char* PARAM_MESSAGE = "message";

#define pot_pin A0

String pot_value = "Value";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {

    Serial.begin(115200);
    pinMode(pot_pin, INPUT);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed!\n");
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

  // Define the route and handler
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request){
    readpot();
    String jsonResponse = "{\"pot_value\":\"" + pot_value + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

    server.onNotFound(notFound);

    server.begin();
}

String readpot(){
    pot_value = String(analogRead(pot_pin));
    Serial.println(pot_value);
    return pot_value;
}

void loop() {
}