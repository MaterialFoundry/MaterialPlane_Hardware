
uint8_t checkSerial() {
  return analyzeMessage(Serial.readStringUntil('\n'));
}

/**
 * Analyze the msg received over the serial port
 */
uint8_t analyzeMessage(String msg) {
  if (debug) Serial.println("Received message: " + msg);
  String recArray[6] = {"","","","","",""};                                            //Stores the read string as individual words
  uint8_t counter = 0;                                                                  //Word counter
  for (int i=0; i<(msg.length()-1); i++) {                                         //Check each individual character in the received string
    if (msg[i] == ' ')                                       //If the character is a space, increment the word counter
      counter++;
    else if (msg[i] == '\"') {
      i++;
      for (i; i<(msg.length()-1); i++){
        if (msg[i] == '\"') break;
        else recArray[counter] += msg[i];
      }
    }
    else                                                                            //If the character is no space, add the character to the current word
      recArray[counter] += msg[i];
  }
  
  if (calibrationRunning) {
    if (recArray[0] == "CAL" && recArray[1] == "CANCEL") calibrationProcedure = CALIBRATION_CANCEL;
    else if (recArray[0] == "CAL" && recArray[1] == "NEXT") calibrationProcedure = CALIBRATION_NEXT;
    return 0;
  }

  if (recArray[0] == "TEST") {
    Serial.println("OK");
  }

  else if (recArray[0] == "HELP") printHelp();

  else if (recArray[0] == "RESTART") {
    Serial.println("Restarting sensor");
    ESP.restart();
  }

  else if (recArray[0] == "GET") {
    
    if (recArray[1] == "STATUS") printStatus();

    else if (recArray[1] == "SETTINGS") updateSetting();

    else if (recArray[1] == "DEBUG") {
      Serial.print("Debug: ");
      if (debug) Serial.println("Enabled");
      else Serial.println("Disabled");
    }

    else if (recArray[1] == "SERIALOUT") {
      Serial.print("Serial Output: ");
      if (serialOutput) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    
    else if (recArray[1] == "WIFI"){
      if (recArray[2] == "SSID") {
        Serial.print("WiFi SSID: ");
        if (ssidString.length() > 0) Serial.println("\"" + ssidString + "\"");
        else Serial.println("not configured");
      }
      else if (recArray[2] == "CONNECTED") {
        Serial.print("WiFi Connected: ");
        if (WiFi.isConnected()) Serial.println("True");
        else Serial.println("False");
      }
      else if (recArray[2] == "IP") {
        Serial.print("WiFi IP Address: ");
        if (WiFi.isConnected()) Serial.println(WiFi.localIP());
        else Serial.println("WiFi not connected");
      }
      else if (recArray[2] = "NAME") {
        Serial.println("Device Name: " + nameString);
      }
      else Serial.println("Error: Invalid value");
    }
    
    else if (recArray[1] == "WS") {
      if (recArray[2] == "MODE") {
        Serial.print("Websocket mode: ");
        if (wsMode == WS_MODE_OFF) Serial.println("Off");
        else if (wsMode == WS_MODE_CLIENT) Serial.println("Client");
        else if (wsMode == WS_MODE_SERVER) Serial.println("Server");
      }
      else if (recArray[2] == "PORT") {
        if (wsMode == WS_MODE_OFF) Serial.println("Error: Websocket is off");
        else Serial.println("Websocket port: " + (String)wsPort);
      }
      else if (recArray[2] == "IP") {
        if (wsMode != WS_MODE_CLIENT) Serial.println("Error: Websocket is not set as client");
        else Serial.println("Websocket IP: " + (String)wsIP);
      }
      else if (recArray[2] == "RUNNING") {
        if (wsMode == WS_MODE_OFF) Serial.println("Error: Websocket is off");
        else {
          Serial.print("Websocket running: ");
          if (wsConnected) Serial.println("True");
          else Serial.println("False");
        }
      }
      else if (recArray[2] == "CLIENTS") {
        if (wsMode == WS_MODE_SERVER) {
          Serial.println("Connected clients: " + (String)webSocketServer.connectedClients());
          for (int i=0; i<webSocketServer.connectedClients(); i++){
            Serial.print("Client " + (String)i + ":\t");
            Serial.println(webSocketServer.remoteIP(i));
          }
        }
        else Serial.println("Error: Websocket is not set as server");
      }
      else Serial.println("Error: Invalid value");
    }
    
    else if (recArray[1] == "IR") {
      if (recArray[2] == "AVERAGE") Serial.println("Average Count: " + (String)averageCount);
      else if (recArray[2] == "GAIN") Serial.println("Gain: " + (String)IRsensor.getGain());
      else if (recArray[2] == "BRIGHTNESS") Serial.println("Brightness Threshold: " + (String)IRsensor.getPixelBrightnessThreshold());
      
      #if defined(HW_BETA)
        else if (recArray[2] == "FRAMEPERIOD") Serial.println("IR Frame Period: " + (String)IRsensor.getFramePeriod() + "ms (Frame Rate: " + (String)(1000/IRsensor.getFramePeriod()) + "Hz)");
        else if (recArray[2] == "EXPOSURE") Serial.println("Exposure Time: " + (String)IRsensor.getExposureTime() + "ms");
        else if (recArray[2] == "NOISE") Serial.println("Noise Threshold: " + (String)IRsensor.getPixelNoiseTreshold());
        else if (recArray[2] == "MINAREA") Serial.println("Min Area Threshold: " + (String)IRsensor.getMinAreaThreshold());
        else if (recArray[2] == "MAXAREA") Serial.println("Max Area Threshold: " + (String)IRsensor.getMaxAreaThreshold());
        else if (recArray[2] == "POINTS") Serial.println("Number of IR points: " + (String)IRsensor.getObjectNumberSetting()); 
        else Serial.println("Error: Invalid value");
      #else
        else if (recArray[2] == "FRAMEPERIOD") Serial.println("IR Frame Period: " + (String)framePeriod + "ms (Frame Rate: " + (String)(1000/framePeriod) + "Hz)");
      #endif
    }

    else if (recArray[1] == "CAL") {
      if (recArray[2] == "CALIBRATION") {
        Serial.print("Calibration: ");
        if (calibration) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "OFFSET") {
        Serial.print("Offset Calibration: ");
        if (offsetOn) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "MIRRORX") {
        Serial.print("Mirror X: ");
        if (mirrorX) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "MIRRORY") {
        Serial.print("Mirror Y: ");
        if (mirrorY) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "ROTATION") {
        Serial.print("Rotation: ");
        if (rotation) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "OFFSETX") {
        Serial.println("Offset X: " + (String)getEepromCalOffsetX());
      }
      else if (recArray[2] == "OFFSETY") {
        Serial.println("Offset Y: " + (String)getEepromCalOffsetY());
      }
      else Serial.println("Error: Invalid value");
    }
    else Serial.println("Error: Invalid value");
  }
  
  else if (recArray[0] == "SET"){
    if (recArray[1] == "DEBUG") {
      if (recArray[2] == "1" || recArray[2] == "TRUE") debug = true;
      else debug = false;
      setEepromDebug(debug);
      Serial.print("Debug set to: ");
      if (debug) Serial.println("Enabled");
      else Serial.println("Disabled");
    }

    else if (recArray[1] == "DEFAULT") {
      firstBoot();
      Serial.println("Settings have been reset to the default values, restarting sensor");
      ESP.restart();
    }
    
    else if (recArray[1] == "SERIALOUT") {
      if (recArray[2] == "1" || recArray[2] == "TRUE") serialOutput = true;
      else serialOutput = false;
      setEepromSerialOutput(serialOutput);
      Serial.print("Serial Output set to: ");
      if (serialOutput) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    
    else if (recArray[1] == "WIFI"){
      if (recArray[2] == "SSID"){
        char ssid[recArray[3].length()];
        char password[recArray[4].length()];
        
        recArray[3].toCharArray(ssid,recArray[3].length()+1);
        recArray[4].toCharArray(password,recArray[4].length()+1);
        
        setEepromSSID(ssid, recArray[3].length());
        setEepromPassword(password, recArray[4].length());
        
        connectWifi(ssid, password, recArray[3].length(), recArray[4].length());
      }
      else if (recArray[2] = "NAME") {
        char deviceName[recArray[3].length()];
        recArray[3].toCharArray(deviceName,recArray[3].length()+1);
        if (recArray[3].length() == 0) {
          Serial.println("Error: Invalid value, name may not be empty");
          return true;
        }
        for (int i=0; i<recArray[3].length(); i++) {
          if (deviceName[i] == ' ') {
            Serial.println("Error: Invalid value, name may not contain empty spaces");
            return true;
          }
        }
        setEepromDeviceName(deviceName, recArray[3].length());
        nameString = deviceName;
        configureDNS(nameString);
        Serial.println("Device name set to: " + nameString);
      }
      else Serial.println("Error: Invalid value");
    }
    
    else if (recArray[1] == "WS"){
      if (recArray[2] == "PORT"){
        uint16_t port = recArray[3].toInt();
        setEepromWsPort(port);
        Serial.println("Websocket port set to: " + (String)port);
        if (wsMode == WS_MODE_SERVER) {
          Serial.println("New port will be set after a restart");
        }
      }
      else if (recArray[2] == "MODE") {
        int wsModeOld = wsMode;
        uint8_t mode = recArray[3].toInt();
        if (recArray[3]  == "OFF") wsMode = WS_MODE_OFF;
        else if (recArray[3]  == "SERVER") wsMode = WS_MODE_SERVER;
        else if (recArray[3]  == "CLIENT") wsMode = WS_MODE_CLIENT;
        else {
          Serial.println("Error: Invalid input");
          return true;
        }
        Serial.print("Websocket mode set to: ");
        if (wsMode == WS_MODE_OFF) {
          Serial.println("Off");
          if (wsModeOld == WS_MODE_SERVER) {
            webSocketServer.close();
            Serial.println("Websocket server stopped");
          }
        }
        else if (wsMode == WS_MODE_CLIENT) {
          Serial.println("Client");
          if (wsModeOld == WS_MODE_SERVER) {
            webSocketServer.close();
            Serial.println("Websocket server stopped");
          }
        }
        else if (wsMode == WS_MODE_SERVER) {
          Serial.println("Server");
          if (wsModeOld != WS_MODE_SERVER) {
            webSocketServer.begin();
            Serial.println("Websocket server started on port: " + (String)wsPort + "\n");
          }
        }
        setEepromWsMode(wsMode); 
      }
      else if (recArray[2] == "IP"){
        wsIP = recArray[3];
      }
      else Serial.println("Error: Invalid value");
    }
    else if (recArray[1] == "IR"){
      if (recArray[2] == "AVERAGE"){
        averageCount = recArray[3].toInt();
        setEepromAvg(averageCount);
        delay(100);
        Serial.println("IR average count set to: " + (String)averageCount);
      }
      else if (recArray[2] == "GAIN"){
        IRsensor.setGain(recArray[3].toFloat());
        setEepromIrGain(recArray[3].toFloat());
        delay(100);
        Serial.println("IR gain set to: " + (String)IRsensor.getGain());
      }
      else if (recArray[2] == "BRIGHTNESS") {
        uint8_t data = recArray[3].toInt();
        IRsensor.setPixelBrightnessThreshold(data);
        setEepromIrBrightness(data);
        delay(100);
        Serial.println("IR brightness treshold set to: " + (String)IRsensor.getPixelBrightnessThreshold());
      }
      else if (recArray[2] == "FRAMEPERIOD") {
        #if defined(HW_BETA)
          IRsensor.setFramePeriod(recArray[3].toFloat());
        #else
          framePeriod = recArray[3].toFloat();
        #endif
        setEepromIrFramePeriod(recArray[3].toFloat());
        delay(100);
        Serial.println("IR frame period set to: " + (String)recArray[3] + "ms");
      }
      #if defined(HW_BETA)
        else if (recArray[2] == "EXPOSURE") {
          IRsensor.setExposureTime(recArray[3].toFloat());
          setEepromIrExposure(recArray[3].toFloat());
          delay(100);
          Serial.println("IR exposure set to: " + (String)IRsensor.getExposureTime() + "ms");
        }
        else if (recArray[2] == "NOISE") {
          uint8_t data = recArray[3].toInt();
          IRsensor.setPixelNoiseTreshold(data);
          setEepromIrNoise(data);
          delay(100);
          Serial.println("IR noise treshold set to: " + (String)IRsensor.getPixelNoiseTreshold());
        }
        else if (recArray[2] == "MINAREA") {
          uint8_t data = recArray[3].toInt();
          IRsensor.setMinAreaThreshold(data); 
          setEepromIrMinArea(data);
          delay(100);
          Serial.println("IR minimum area set to: " + (String)IRsensor.getMinAreaThreshold());
        }
        else if (recArray[2] == "MAXAREA") {
          uint16_t data = recArray[3].toInt();
          IRsensor.setMaxAreaThreshold(data);
          setEepromIrMaxArea(data);
          delay(100);
          Serial.println("IR maximum area set to: " + (String)IRsensor.getMaxAreaThreshold());
        }
        else if (recArray[2] == "POINTS") {
          uint8_t data = recArray[3].toInt();
          if (data > MAX_IR_POINTS) data = MAX_IR_POINTS;
          IRsensor.setObjectNumberSetting(data);
          setEepromIrPoints(data);
          maxIRpoints = data;
          delay(100);
          Serial.println("IR maximum points set to: " + (String)IRsensor.getObjectNumberSetting());
        }
        else if (recArray[2] == "AUTOEXPOSE") {
          autoExposureProcedure = EXP_START;
        }
      #endif
      else Serial.println("Error: Invalid value");
    }
    else if (recArray[1] == "CAL"){
      if (recArray[2] == "CALIBRATION"){
        if (recArray[3] == "1" || recArray[3] == "TRUE") calibration = true;
        else calibration = false;
        setEepromCalCalibration(calibration);
        Serial.print("Calibration set to: ");
        if (calibration) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "OFFSET"){
        if (recArray[3] == "1" || recArray[3] == "TRUE") offsetOn = true;
        else offsetOn = false;
        setEepromCalOffset(offsetOn);
        Serial.print("Offset set to: ");
        if (offsetOn) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "MIRRORX"){
        if (recArray[3] == "1" || recArray[3] == "TRUE") mirrorX = true;
        else mirrorX = false;
        setEepromCalMirrorX(mirrorX);
        Serial.print("Mirror X set to: ");
        if (mirrorX) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "MIRRORY"){
        if (recArray[3] == "1" || recArray[3] == "TRUE") mirrorY = true;
        else mirrorY = false;
        setEepromCalMirrorY(mirrorY);
        Serial.print("Mirror Y set to: ");
        if (mirrorY) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "ROTATION"){
        if (recArray[3] == "1" || recArray[3] == "TRUE") rotation = true;
        else rotation = false;
        setEepromCalRotation(rotation);
        Serial.print("Rotation set to: ");
        if (rotation) Serial.println("Enabled");
        else Serial.println("Disabled");
      }
      else if (recArray[2] == "OFFSETX") {
        offsetX = recArray[3].toInt();
        setEepromCalOffsetX(offsetX);
        Serial.println("Offset X set to: " + (String)offsetX);
      }
      else if (recArray[2] == "OFFSETY") {
        offsetY = recArray[3].toInt();
        setEepromCalOffsetY(offsetY);
        Serial.println("Offset Y set to: " + (String)offsetY);
      }
      else Serial.println("Error: Invalid value");
    }
    else Serial.println("Error: Invalid value");

    updateSetting();
  }
  else if (recArray[0] == "SCAN" && recArray[1] == "WIFI") {
    Serial.println("Scanning for WIFI stations. Please wait.");
    int n = WiFi.scanNetworks();
    String wsMessage = "{\"status\":\"wifiStations\",\"data\":[";
    if (n==0)
      Serial.println("No stations found"); 
    else {
      Serial.println((String)n + " stations found");
      Serial.println("Nr\tSSID\t\t\tRSSI\tAuthentication Mode");
      for (int i = 0; i < n; ++i) {
        String authMode = "";
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) authMode = "Open";
        else if (WiFi.encryptionType(i) == WIFI_AUTH_WEP) authMode = "WEP";
        else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA_PSK) authMode = "WPA-PSK";
        else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA2_PSK) authMode = "WPA2-PSK";
        else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA_WPA2_PSK) authMode = "WPA-WPA2-PSK";
        else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA2_ENTERPRISE) authMode = "WPA2-Enterprise";
        //else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA3_PSK) authMode = "WPA3-PSK";
        //else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA2_WPA3_PSK) authMode = "WPA2-WPA3-PSK";
        //else if (WiFi.encryptionType(i) == WIFI_AUTH_WAPI_PSK) authMode = "WAPI-PSK";
        else authMode = "Unknown";
        Serial.println((String)(i + 1) + "\t" + (String)WiFi.SSID(i) + "\t\t\t" + (String)WiFi.RSSI(i) + "dBm\t" + authMode);
        if (i > 0) wsMessage += ",";
        wsMessage += "{\"ssid\":\"" + (String)WiFi.SSID(i) + "\",\"rssi\":" + (String)WiFi.RSSI(i) + ",\"authMode\":\"" + authMode + "\"}";
      }
    }
    wsMessage += "]}";
    if (wsMode != WS_MODE_OFF) webSocketServer.broadcastTXT(wsMessage);
  }
  else if (recArray[0] == "PERFORM" && recArray[1] == "CALIBRATION"){
    calibrationProcedure = CALIBRATION_STARTING;
  }
  else if (recArray[0] == "CAL" && recArray[1] == "CANCEL"){
    return CAL_CANCEL;
  }
  else if (recArray[0] == "CAL" && recArray[1] == "NEXT"){
    return CAL_NEXT;
  }
  else Serial.println("Error: Invalid value");
  return true;
}

void printHelp(){
  String msg = "-----------------------------------------------------------------------------\nHELP MENU\nBelow is a list of commands.\nWhen it states \"SET/GET\" it means that you can either get the setting or set the setting by using either \"SET\" or \"GET\"\nArguments are indicated by square brackets and are only relevant for \"SET\" commands.\n\nCommands:\n";
  msg += " HELP\n";
  msg += " RESTART\n";
  msg += " GET STATUS\n";
  msg += " SET DEFAULT\n";
  msg += " GET SETTINGS\n\n";
  
  msg += " SET/GET DEBUG [enable]\t\t\t- TRUE/FALSE\n";
  msg += " SET/GET SERIALOUT [enable]\t\t- TRUE/FALSE\n\n";
  
  msg += " SCAN WIFI\n";
  msg += " SET/GET WIFI SSID [name] [password]\t- SSID name, SSID password\n";
  msg += " GET WIFI CONNECTED\n";
  msg += " GET WIFI IP\n";
  msg += " SET/GET WIFI NAME [name]\t\t- device name\n\n";
  
  msg += " SET/GET WS MODE [mode]\t\t\t- OFF/CLIENT/SERVER\n";
  msg += " SET/GET WS PORT [port]\t\t\t- 16bit integer\n";
  msg += " SET/GET WS IP [ip]\t\t\t- XXX.XXX.XXX.XXX\n";
  msg += " GET WS RUNNING\n";
  msg += " GET WS CLIENTS\n\n";

  msg += " SET/GET IR AVERAGE [average count]\t- 8bit integer\n";
  msg += " SET/GET IR FRAMEPERIOD [frameperiod]\t- float (ms)\n";
  msg += " SET/GET IR EXPOSURE [exposure]\t\t- float (ms)\n";
  msg += " SET/GET IR GAIN [gain]\t\t\t- float\n";
  msg += " SET/GET IR BRIGHTNESS [brightness]\t- 8bit integer\n";
  msg += " SET/GET IR NOISE [noise]\t\t- 8bit integer\n";
  msg += " SET/GET IR MINAREA [area]\t\t- 8bit integer\n";
  msg += " SET/GET IR MAXAREA [area]\t\t- 16bit integer\n";
  msg += " SET/GET IR POINTS [points]\t\t- 0 to 16, integer\n\n";
  
  msg += " PERFORM CALIBRATION [method]\t\t- SINGLE/MULTI\n";
  msg += " PERFORM OFFSET [method]\t\t- SINGLE/MULTI\n";
  msg += " SET/GET CAL CALIBRATION [enable]\t- TRUE/FALSE\n";
  msg += " SET/GET CAL OFFSET [enable]\t\t- TRUE/FALSE\n";
  msg += " SET/GET CAL MIRRORX [enable]\t\t- TRUE/FALSE\n";
  msg += " SET/GET CAL MIRRORY [enable]\t\t- TRUE/FALSE\n";
  msg += " SET/GET CAL ROTATION [rotation]\t- TRUE/FALSE\n";
  msg += " SET/GET CAL OFFSETX [offset]\t\t- 16bit integer\n";
  msg += " SET/GET IR OFFSETY [offset]\t\t- 16bit integer\n";
  
  msg += " \n";
  msg += "-----------------------------------------------------------------------------\n\n";
  Serial.println(msg);
}

void printStatus() {
  Serial.println("-----------------------------------------------------------------------------");
  Serial.println("STATUS\n");
  Serial.println("GENERAL");
  Serial.print("Hardware Version:\t");
  #if defined(HW_DIY_BASIC)
    Serial.println("DIY Basic");
  #elif defined(HW_DIY_FULL)
    Serial.println("DIY Full");
  #else
    Serial.println("Beta");
  #endif
  Serial.println("Firmware Version:\tv" + (String)FIRMWARE_VERSION);
  Serial.print("Debugging:\t\t");
  if (debug) Serial.println("Enabled");
  else Serial.println("Disabled");
  Serial.print("Serial Output:\t\t");
  if (serialOutput) Serial.println("Enabled");
  else Serial.println("Disabled");

  Serial.println("\nBATTERY");
  Serial.println("Charging State:\t\t" + (String)chargeState);
  Serial.println("Voltage:\t\t" + (String)vBat + "V");

  Serial.print("\nWIFI\nDevice Name:\t\t\"" + nameString);
  Serial.print("\"\nSSID:\t\t\t");
  if (ssidString.length() > 0) Serial.println("\"" + ssidString + "\"");
  else Serial.println("not configured");
  Serial.print("Connected:\t\t");
  if (WiFi.isConnected()) {
    Serial.print("True\nIP Address:\t\t");
    Serial.println(WiFi.localIP());
  }
  else
    Serial.println("False");
  
  Serial.print("\nWEBSOCKET\nMode:\t\t\t");
  if (wsMode == WS_MODE_OFF) Serial.println("Off");
  else if (wsMode == WS_MODE_SERVER) Serial.println("Server");
  else if (wsMode == WS_MODE_CLIENT) Serial.println("Client");
  
  if (wsMode != WS_MODE_OFF) Serial.println("Port:\t\t\t" + (String)wsPort);
  
  if (wsMode == WS_MODE_SERVER) {
    Serial.println("Connected clients:\t" + (String)webSocketServer.connectedClients());
    for (int i=0; i<webSocketServer.connectedClients(); i++){
      Serial.print("\tClient " + (String)i + ":\t");
      Serial.println(webSocketServer.remoteIP(i));
    }
  }
  else if (wsMode == WS_MODE_CLIENT) {
    Serial.print("Connected:\t\t");
    if (wsConnected) {
      Serial.println("True");
      Serial.println("Server IP Address:\t");
    }
    else Serial.println("False");
  }

  Serial.println("\nIR TRACKER");
  Serial.println("Average Count:\t\t" + (String)averageCount);
  #if defined(HW_BETA)
    Serial.println("Frame Period:\t\t" + (String)IRsensor.getFramePeriod() + "ms (Frame Rate: " + (String)(1000/IRsensor.getFramePeriod()) + "Hz)");
    Serial.println("Exposure Time:\t\t" + (String)IRsensor.getExposureTime() + "ms");
    Serial.println("Gain:\t\t\t" + (String)IRsensor.getGain());
    Serial.println("Brightness Threshold:\t" + (String)IRsensor.getPixelBrightnessThreshold());
    Serial.println("Noise Threshold:\t" + (String)IRsensor.getPixelNoiseTreshold());
    Serial.println("Min Area Threshold:\t" + (String)IRsensor.getMinAreaThreshold());
    Serial.println("Max Area Threshold:\t" + (String)IRsensor.getMaxAreaThreshold());
    Serial.println("Number of IR points:\t" + (String)IRsensor.getObjectNumberSetting());
  #else
    Serial.println("Frame Period:\t\t" + (String)framePeriod + "ms (Frame Rate: " + (String)(1000/framePeriod) + "Hz)");
    Serial.println("Gain:\t\t\t" + (String)IRsensor.getGain());
    Serial.println("Brightness Threshold:\t" + (String)IRsensor.getPixelBrightnessThreshold());
  #endif

  Serial.println("\nCALIBRATION");
  Serial.print("Calibration:\t\t");
  if (calibration) Serial.println("Enabled");
  else Serial.println("Disabled");
  Serial.print("Offset Calibration:\t");
  if (offsetOn) Serial.println("Enabled");
  else Serial.println("Disabled");
  Serial.print("Mirror X:\t\t");
  if (mirrorX) Serial.println("Enabled");
  else Serial.println("Disabled");
  Serial.print("Mirror Y:\t\t");
  if (mirrorY) Serial.println("Enabled");
  else Serial.println("Disabled");
  Serial.print("Rotation:\t\t");
  if (rotation) Serial.println("Enabled");
  else Serial.println("Disabled");
  Serial.println("Offset X:\t\t" + (String)offsetX);
  Serial.println("Offset Y:\t\t" + (String)offsetY);
  Serial.println("-----------------------------------------------------------------------------");
}

void updateSetting(){
  String msg = "{\"status\":\"update\",\"firmware\":\"" + (String)FIRMWARE_VERSION + "\",\"hardware\":";
  #if defined(HW_BETA)
    msg += "\"Beta\",";
  #elif defined(HW_DIY_FULL)
    msg += "\"DIY Full\",";
  #else
    msg += "\"DIY Basic\",";
  #endif
  String websocketMode;
  if (wsMode == WS_MODE_OFF) websocketMode = "Off";
  else if (wsMode == WS_MODE_SERVER) websocketMode = "Server";
  else if (wsMode == WS_MODE_CLIENT) websocketMode = "Client";
  String wsClients = "";
  if (wsMode == WS_MODE_SERVER) {
    for (int i=0; i<webSocketServer.connectedClients(); i++) {
      if (i > 0) wsClients += ",";
      wsClients += "\"";
      wsClients += webSocketServer.remoteIP(i).toString().c_str();
      wsClients += "\"";
    }
  }
  msg += "\"sett\":{\"debug\":" + (String)debug + ",\"serialOut\":" + (String)serialOutput + "},";
  msg += "\"network\":{\"ssid\":\"" + ssidString + "\",\"ipAddress\":\"" + WiFi.localIP().toString().c_str() + "\",\"name\":\"" + nameString + "\",\"connected\":" + (String)WiFi.isConnected() + ",\"wsMode\":\"" + websocketMode + "\",\"wsPort\":" + (String)wsPort + ",\"wsClients\":[" +wsClients + "]},";
  msg += "\"cal\":{\"calibrationEnable\":" + (String)calibration + ",\"offsetEn\":" + (String)offsetOn + ",\"mirrorX\":" + (String)mirrorX + ",\"mirrorY\":" + (String)mirrorY + ",\"rotation\":" + (String)rotation + ",\"offsetX\":" + (String)offsetX + ",\"offsetY\":" + (String)offsetY + "},";
  #if defined(HW_BETA)
    msg += "\"ir\":{\"averageCount\":" + (String)averageCount + ",\"framePeriod\":" + (String)IRsensor.getFramePeriod() + ",\"exposure\":" + (String)IRsensor.getExposureTime() + ",\"gain\":" + (String)IRsensor.getGain() + ",\"brightness\":" + (String)IRsensor.getPixelBrightnessThreshold() + ",\"noise\":" + (String)IRsensor.getPixelNoiseTreshold() + ",\"minArea\":" + (String)IRsensor.getMinAreaThreshold() + ",\"maxArea\":" + (String)IRsensor.getMaxAreaThreshold() + ",\"points\":" + (String)IRsensor.getObjectNumberSetting() + "}";
  #else
    msg += "\"ir\":{\"averageCount\":" + (String)averageCount + ",\"framePeriod\":" + (String)framePeriod + ",\"gain\":" + (String)IRsensor.getGain() + ",\"brightness\":" + (String)IRsensor.getPixelBrightnessThreshold() + "}";
  #endif
  
  msg += "}";
  // send message to client
  if (wsMode != WS_MODE_OFF) webSocketServer.broadcastTXT(msg);
  if (serialOutput) Serial.println(msg);
  //Serial.print("webserver client: ");
  //Serial.println(webServer.client());
}

void sendIRcode(char* rcvGroup, uint32_t result) {
  if (debug) Serial.println("IR rec: " + (String)rcvGroup + '\t' + (String)result);
  String msg = "{\"status\":\"IRcode\",\"data\":{\"protocol\":\"" + (String)rcvGroup + "\",\"code\":" + (String)result + "}}";

  if (wsMode != WS_MODE_OFF) webSocketServer.broadcastTXT(msg);
  if (serialOutput) Serial.println(msg);
}
