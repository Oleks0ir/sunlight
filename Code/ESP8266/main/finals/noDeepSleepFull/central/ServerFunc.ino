void notFound(AsyncWebServerRequest *request) {
  Serial.println("Gotcha");
    request->send(404, "text/plain", "Not found");
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
    FilegateOpen = false;
    request->send(200);
  });
  
  server.on("/updateconfig", HTTP_POST, [](AsyncWebServerRequest *request){

    request->send(200, "text/plain", "none");
  });  


  server.onNotFound(notFound);

  server.begin();
}