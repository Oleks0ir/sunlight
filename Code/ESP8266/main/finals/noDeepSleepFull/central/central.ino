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