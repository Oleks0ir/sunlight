
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

inline String BoolToString(bool b)
{
  return b ? "true" : "false";
}