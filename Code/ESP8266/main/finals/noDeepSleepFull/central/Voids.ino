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

void failureBlink(){

  Serial.println("Failure");

}

void finishConnectionSession(){
  Serial.println("finishConnectionSession()");
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