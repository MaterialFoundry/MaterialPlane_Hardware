/**
 * Check the serial port for new messages to analyze
 */
uint8_t checkSerial() {
  return analyzeMessage(Serial.readStringUntil('\n'));
}

/**
 * Analyze the received message
 */
uint8_t analyzeMessage(String msg) {
  if (debug) Serial.println("Received message: " + msg);
  String recArray[6] = {"","","","","",""};                                            //Stores the read string as individual words
  uint8_t counter = 0;                                                                  //Word counter
  
  //Check each individual character in the received string
  for (int i=0; i<(msg.length()-1); i++) {
    
    //If the character is a space, increment the word counter
    if (msg[i] == ' ')                                       
      counter++;

    //Else if the character is ", add all following characters together, until the next ".
    else if (msg[i] == '\"') {
      i++;
      for (i; i<(msg.length()-1); i++){
        if (msg[i] == '\"') break;
        else recArray[counter] += msg[i];
      }
    }

    //Else add the character to the current word
    else                                                                            
      recArray[counter] += msg[i];
  }

  //If the calibration procedure is running, allow "CAL CANCEL" and "CAL NEXT" to change the calibration procedure
  if (calibrationRunning) {
    if (recArray[0] == "CAL" && recArray[1] == "CANCEL") calibrationProcedure = CALIBRATION_CANCEL;
    else if (recArray[0] == "CAL" && recArray[1] == "NEXT") calibrationProcedure = CALIBRATION_NEXT;
    return 0;
  }

  if (recArray[0] == "HELP") printHelp();

  else if (recArray[0] == "RESTART") {
    Serial.println("Restarting sensor");
    ESP.restart();
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Get
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  else if (recArray[0] == "GET") {
    
    if      (recArray[1] == "STATUS")       printStatus();
    else if (recArray[1] == "VERSION") 
      #if defined(HW_DIY_BASIC)
        Serial.println("Hardware Version: DIY Basic\nFirmware Version: " + (String)FIRMWARE_VERSION);
      #elif defined(HW_DIY_FULL)
        Serial.println("Hardware Version: DIY Full\nFirmware Version: " + (String)FIRMWARE_VERSION);
      #else
        Serial.println("Hardware Version: Beta\nFirmware Version: " + (String)FIRMWARE_VERSION);
      #endif
    else if (recArray[1] == "SETTINGS")     updateSettings();
    else if (recArray[1] == "DEBUG")        Serial.println("Debug: " + (String)(debug ? "Enabled" : "Disabled"));
    else if (recArray[1] == "SERIALOUT")    Serial.println("Serial Output: " + (String)(serialOutput ? "Enabled" : "Disabled"));
    
    else if (recArray[1] == "WIFI"){
      if      (recArray[2] == "SSID")       Serial.println("WiFi SSID: " + (ssidString.length() > 0 ? '\"' + ssidString + '\"' : "Not Configured"));
      else if (recArray[2] == "CONNECTED")  Serial.println("WiFi Connected: " + (String)(WiFi.isConnected() ? "True" : "False"));
      else if (recArray[2] == "IP")         Serial.println("WiFi IP Address: " + (String)(WiFi.isConnected() ? WiFi.localIP().toString() : "WiFi not connected"));
      else if (recArray[2] = "NAME")        Serial.println("Device Name: \"" + nameString + '\"');
      else                                  Serial.println("Error: Invalid value");
    }
    
    else if (recArray[1] == "WS") {
      if      (recArray[2] == "MODE")       Serial.println("Websocket mode: " + (String)(wsMode == WS_MODE_OFF ? "Off" : (String)(wsMode == WS_MODE_CLIENT ? "Client" : (String)(wsMode == WS_MODE_SERVER ? "Server" : " "))));
      else if (recArray[2] == "PORT")       Serial.println((String)(wsMode == WS_MODE_OFF ? "Websocket is off" : "Websocket port: " + (String)wsPort));
      else if (recArray[2] == "IP")         Serial.println((String)(wsMode != WS_MODE_CLIENT ? "Websocket is not set as client" : "Websocket IP: " + (String)wsIP));
      else if (recArray[2] == "RUNNING")    Serial.println((String)(wsMode == WS_MODE_OFF ? "Websocket is off" : "Websocket running: " + (String)(wsConnected ? "True" : "False")));
      else if (recArray[2] == "CLIENTS") {
        if (wsMode == WS_MODE_SERVER) {
          String msg = "Connected clients: " + (String)webSocketServer.connectedClients() + '\n';
          for (int i=0; i<webSocketServer.connectedClients(); i++){
            msg += "Client " + (String)i + ":\t" + webSocketServer.remoteIP(i).toString() + '\n';
          }
          Serial.println(msg);
        }
        else Serial.println("Websocket is not set as server");
      }
      else Serial.println("Error: Invalid value");
    }
    
    else if (recArray[1] == "IR") {
      if      (recArray[2] == "AVERAGE")        Serial.println("Average Count: " + (String)IRsensor.getAverageCount());
      else if (recArray[2] == "GAIN")           Serial.println("Gain: " + (String)IRsensor.getGain());
      else if (recArray[2] == "BRIGHTNESS")     Serial.println("Brightness Threshold: " + (String)IRsensor.getPixelBrightnessThreshold());
      else if (recArray[2] == "DROPDELAY")      Serial.println("Drop Delay: " + (String)dropDelayTime + "ms");
      #if defined(HW_BETA)
        else if (recArray[2] == "FRAMEPERIOD")  Serial.println("IR Frame Period: " + (String)IRsensor.getFramePeriod() + "ms (Frame Rate: " + (String)(1000/IRsensor.getFramePeriod()) + "Hz)");
        else if (recArray[2] == "EXPOSURE")     Serial.println("Exposure Time: " + (String)IRsensor.getExposureTime() + "ms");
        else if (recArray[2] == "NOISE")        Serial.println("Noise Threshold: " + (String)IRsensor.getPixelNoiseTreshold());
        else if (recArray[2] == "MINAREA")      Serial.println("Min Area Threshold: " + (String)IRsensor.getMinAreaThreshold());
        else if (recArray[2] == "MAXAREA")      Serial.println("Max Area Threshold: " + (String)IRsensor.getMaxAreaThreshold());
        else if (recArray[2] == "POINTS")       Serial.println("Number of IR points: " + (String)IRsensor.getObjectNumberSetting()); 
        else                                    Serial.println("Error: Invalid value");
      #else
        else if (recArray[2] == "FRAMEPERIOD")  Serial.println("IR Frame Period: " + (String)IRsensor.getFramePeriod() + "ms (Frame Rate: " + (String)(1000/IRsensor.getFramePeriod()) + "Hz)");
      #endif
    }
    
    else if (recArray[1] == "CAL") {
      if      (recArray[2] == "CALIBRATION")    Serial.println("Calibration:\t\t" + (String)(IRsensor.getCalibrationEnable() ? "Enabled" : "Disabled"));
      else if (recArray[2] == "OFFSET")         Serial.println("Offset Calibration:\t" + (String)(IRsensor.getCalibrationOffsetEnable() ? "Enabled" : "Disabled"));
      else if (recArray[2] == "MIRRORX")        Serial.println("Mirror X:\t\t" + (String)(IRsensor.getMirrorX() ? "Enabled" : "Disabled"));
      else if (recArray[2] == "MIRRORY")        Serial.println("Mirror Y:\t\t" + (String)(IRsensor.getMirrorY() ? "Enabled" : "Disabled"));
      else if (recArray[2] == "ROTATION")       Serial.println("Rotation:\t\t" + (String)(IRsensor.getRotation() ? "Enabled" : "Disabled"));
      else if (recArray[2] == "OFFSETX")        Serial.println("Offset X: " + (String)getEepromCalOffsetX());
      else if (recArray[2] == "OFFSETY")        Serial.println("Offset Y: " + (String)getEepromCalOffsetY());
      else if (recArray[2] == "SCALEX")        Serial.println("SCALE X: " + (String)getEepromCalScaleX());
      else if (recArray[2] == "SCALEY")        Serial.println("SCALE Y: " + (String)getEepromCalScaleY());
      else                                      Serial.println("Error: Invalid value");
    }
    else                                        Serial.println("Error: Invalid value");
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Set
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  else if (recArray[0] == "SET"){
    if (recArray[1] == "DEBUG") {
      debug = (recArray[2] == "1" || recArray[2] == "TRUE");
      setEepromDebug(debug);
      Serial.println("Debug set to: " + (String)(debug ? "Enabled" : "Disabled"));

      #if defined(HW_BETA)
        IDsensor.enableDebug(true);
      #endif
    }

    else if (recArray[1] == "DEFAULT") {
      firstBoot();
      Serial.println("Settings have been reset to the default values, restarting sensor");
      ESP.restart();
    }
    
    else if (recArray[1] == "SERIALOUT") {
      serialOutput = (recArray[2] == "1" || recArray[2] == "TRUE");
      setEepromSerialOutput(serialOutput);
      Serial.println("Serial Output set to: " + (String)(serialOutput ? "Enabled" : "Disabled"));
    }
    
    else if (recArray[1] == "WIFI"){
      if (recArray[2] == "SSID"){
        char ssid[recArray[3].length()+1];
        char password[recArray[4].length()+1];
        
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
        setEepromWsPort(recArray[3].toInt());
        Serial.println("Websocket port set to: " + (String)recArray[3].toInt());
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
        uint8_t average = recArray[3].toInt();
        if (average < 1) average = 1;
        IRsensor.setAverageCount(average);
        setEepromAvg(average);
        delay(10);
        Serial.println("IR average count set to: " + (String)IRsensor.getAverageCount());
      }
      else if (recArray[2] == "GAIN"){
        Serial.println("Set gain: " + (String)recArray[3].toFloat());
        IRsensor.setGain(recArray[3].toFloat());
        setEepromIrGain(recArray[3].toFloat());
        delay(10);
        Serial.println("IR gain set to: " + (String)IRsensor.getGain());
      }
      else if (recArray[2] == "BRIGHTNESS") {
        IRsensor.setPixelBrightnessThreshold(recArray[3].toInt());
        setEepromIrBrightness(recArray[3].toInt());
        delay(10);
        Serial.println("IR brightness treshold set to: " + (String)IRsensor.getPixelBrightnessThreshold());
      }
      else if (recArray[2] == "FRAMEPERIOD") {
        float fPeriod = recArray[3].toFloat();
        //#if defined(HW_BETA)
        //  if (fPeriod < 25) fPeriod = 25;
        //#endif
        IRsensor.setFramePeriod(fPeriod);
        setEepromIrFramePeriod(fPeriod);
        dropDelay = dropDelayTime/fPeriod;
        delay(10);
        Serial.println("IR frame period set to: " + (String)fPeriod + "ms");
      }
      else if (recArray[2] == "DROPDELAY") {
        setEepromIrDropDelay(recArray[3].toInt());
        dropDelayTime = getEepromIrDropDelay();
        dropDelay = dropDelayTime/IRsensor.getFramePeriod();
        delay(10);
        Serial.println("Drop delay time set to: " + (String)dropDelayTime + "ms");
      }
      else if (recArray[2] == "AUTOEXPOSE") {
        autoExposureProcedure = EXP_START;
      }
      #if defined(HW_BETA)
        else if (recArray[2] == "EXPOSURE") {
          IRsensor.setExposureTime(recArray[3].toFloat());
          setEepromIrExposure(recArray[3].toFloat());
          delay(10);
          Serial.println("IR exposure set to: " + (String)IRsensor.getExposureTime() + "ms");
        }
        else if (recArray[2] == "NOISE") {
          IRsensor.setPixelNoiseTreshold(recArray[3].toInt());
          setEepromIrNoise(recArray[3].toInt());
          delay(10);
          Serial.println("IR noise treshold set to: " + (String)IRsensor.getPixelNoiseTreshold());
        }
        else if (recArray[2] == "MINAREA") {
          IRsensor.setMinAreaThreshold(recArray[3].toInt()); 
          setEepromIrMinArea(recArray[3].toInt());
          delay(10);
          Serial.println("IR minimum area set to: " + (String)IRsensor.getMinAreaThreshold());
        }
        else if (recArray[2] == "MAXAREA") {
          IRsensor.setMaxAreaThreshold(recArray[3].toInt());
          setEepromIrMaxArea(recArray[3].toInt());
          delay(10);
          Serial.println("IR maximum area set to: " + (String)IRsensor.getMaxAreaThreshold());
        }
        else if (recArray[2] == "POINTS") {
          uint8_t data = recArray[3].toInt();
          if (data > MAX_IR_POINTS) data = MAX_IR_POINTS;
          IRsensor.setObjectNumberSetting(data);
          setEepromIrPoints(data);
          delay(10);
          Serial.println("IR maximum points set to: " + (String)IRsensor.getObjectNumberSetting());
        }
      #endif
      else Serial.println("Error: Invalid value");
    }
    else if (recArray[1] == "CAL"){
      if (recArray[2] == "CALIBRATION"){
        bool calEn = (recArray[3] == "1" || recArray[3] == "TRUE");
        setEepromCalCalibration(calEn);
        IRsensor.setCalibrationEnable(calEn);
        Serial.println("Calibration set to: " + (String)(IRsensor.getCalibrationEnable() ? "Enabled" : "Disabled"));
      }
      else if (recArray[2] == "OFFSET"){
        bool offsetEn = (recArray[3] == "1" || recArray[3] == "TRUE");
        setEepromCalOffset(offsetEn);
        IRsensor.setCalibrationOffsetEnable(offsetEn);
        Serial.println("Offset Calibration set to: " + (String)(IRsensor.getCalibrationOffsetEnable() ? "Enabled" : "Disabled"));
      }
      else if (recArray[2] == "MIRRORX"){
        bool mirror = (recArray[3] == "1" || recArray[3] == "TRUE");
        setEepromCalMirrorX(mirror);
        IRsensor.setMirrorX(mirror);
        Serial.println("Mirror X set to: " + (String)(IRsensor.getMirrorX() ? "Enabled" : "Disabled"));
      }
      else if (recArray[2] == "MIRRORY"){
        bool mirror = (recArray[3] == "1" || recArray[3] == "TRUE");
        setEepromCalMirrorY(mirror);
        IRsensor.setMirrorY(mirror);
        Serial.println("Mirror Y set to: " + (String)(IRsensor.getMirrorY() ? "Enabled" : "Disabled"));
      }
      else if (recArray[2] == "ROTATION"){
        bool rotation = (recArray[3] == "1" || recArray[3] == "TRUE");
        setEepromCalRotation(rotation);
        IRsensor.setRotation(rotation);
        Serial.println("Rotation set to: " + (String)(IRsensor.getRotation() ? "Enabled" : "Disabled"));
      }
      else if (recArray[2] == "OFFSETX") {
        setEepromCalOffsetX(recArray[3].toInt());
        IRsensor.setOffsetX(recArray[3].toInt());
        Serial.println("Offset X set to: " + (String)IRsensor.getOffsetX());
      }
      else if (recArray[2] == "OFFSETY") {
        setEepromCalOffsetY(recArray[3].toInt());
        IRsensor.setOffsetY(recArray[3].toInt());
        Serial.println("Offset Y set to: " + (String)IRsensor.getOffsetY());
      }
      else if (recArray[2] == "SCALEX") {
          IRsensor.setScaleX(recArray[3].toFloat());
          setEepromCalScaleX(recArray[3].toFloat());
          Serial.println("Scale X set to: " + (String)IRsensor.getScaleX());
      }
      else if (recArray[2] == "SCALEY") {
          IRsensor.setScaleY(recArray[3].toFloat());
          setEepromCalScaleY(recArray[3].toFloat());
          Serial.println("Scale Y set to: " + (String)IRsensor.getScaleY());
      }
      else Serial.println("Error: Invalid value");
    }
    else Serial.println("Error: Invalid value");

    updateSettings();
  }
  else if (recArray[0] == "SCAN" && recArray[1] == "WIFI") {
    Serial.println("Scanning for WIFI stations. Please wait.");
    int n = WiFi.scanNetworks();
    String wsMsg = "{\"status\":\"wifiStations\",\"data\":[";
    String msg = "";
    if (n==0)
      msg = "No stations found"; 
    else {
      msg += (String)n + " stations found\nNr\tSSID\t\t\tRSSI\tAuthentication Mode\n";
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
        msg += (String)(i + 1) + "\t" + (String)WiFi.SSID(i) + "\t\t\t" + (String)WiFi.RSSI(i) + "dBm\t" + authMode + '\n';
        if (i > 0) wsMsg += ",";
        wsMsg += "{\"ssid\":\"" + (String)WiFi.SSID(i) + "\",\"rssi\":" + (String)WiFi.RSSI(i) + ",\"authMode\":\"" + authMode + "\"}";
      }
    }
    wsMsg += "]}";
    if (wsMode != WS_MODE_OFF) broadcastWs(wsMsg);
    if (serialOutput) Serial.println(wsMsg);
    Serial.println(msg);
  }
  else if (recArray[0] == "PERFORM" && recArray[1] == "CALIBRATION")  {
    String calMode = recArray[2];
    if (calMode == "SINGLE") calibrationMode = CALIBRATION_SINGLE;
    else if (calMode == "OFFSET") calibrationMode = CALIBRATION_OFFSET;
    else if (calMode == "MULTI") calibrationMode = CALIBRATION_MULTI;
    else return false;
    calibrationProcedure = CALIBRATION_STARTING;
  }
  else if (recArray[0] == "CAL" && recArray[1] == "CANCEL")           return CAL_CANCEL;
  else if (recArray[0] == "CAL" && recArray[1] == "NEXT")             return CAL_NEXT;
  else                                                                Serial.println("Error: Invalid value");
  return true;
}

/**
 * Print help menu
 */
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
  msg += " SET/GET IR DROPDELAY [drop delay time]\t\t- 0 to 2500, integer\n\n";
  
  msg += " PERFORM CALIBRATION [method]\t\t- SINGLE/MULTI\n";
  msg += " PERFORM OFFSET [method]\t\t- SINGLE/MULTI\n";
  msg += " SET/GET CAL CALIBRATION [enable]\t- TRUE/FALSE\n";
  msg += " SET/GET CAL OFFSET [enable]\t\t- TRUE/FALSE\n";
  msg += " SET/GET CAL MIRRORX [enable]\t\t- TRUE/FALSE\n";
  msg += " SET/GET CAL MIRRORY [enable]\t\t- TRUE/FALSE\n";
  msg += " SET/GET CAL ROTATION [rotation]\t- TRUE/FALSE\n";
  msg += " SET/GET CAL OFFSETX [offset]\t\t- 16bit integer\n";
  msg += " SET/GET CAL OFFSETY [offset]\t\t- 16bit integer\n";
  msg += " SET/GET CAL SCALEX [scale]\t\t- float\n";
  msg += " SET/GET CAL SCALEY [scale]\t\t- float\n";
  
  msg += " \n";
  msg += "-----------------------------------------------------------------------------\n\n";
  Serial.println(msg);
}

/**
 * Print status. Prints all settings
 */
void printStatus() {
  String msg = "-----------------------------------------------------------------------------\nSTATUS\n\n";
  Serial.print(msg);
  
  msg = "GENERAL\n";
  #if defined(HW_DIY_BASIC)
    msg += "Hardware Version:\tDIY Basic\n";
  #elif defined(HW_DIY_FULL)
    msg += "Hardware Version:\tDIY Full\n";
  #else
    msg += "Hardware Version:\tBeta\n";
  #endif
  msg += "Firmware Version:\tv" + (String)FIRMWARE_VERSION + '\n';
  msg += "Webserver Version:\tv" + webserverVersion + '\n';
  msg += "Debugging:\t\t" + (String)(debug ? "Enabled" : "Disabled") + '\n';
  msg += "Serial Output:\t\t" + (String)(serialOutput ? "Enabled" : "Disabled") + '\n';
  Serial.print(msg);
  
  msg = "\nBATTERY\n";
  msg += "USB Connected:\t\t" + (String)(usbActive ? "Connected" : "Disconnected") + '\n';
  msg += "Charging State:\t\t" + (String)chargeState + '\n';
  msg += "Voltage:\t\t" + (String)vBat + "V\n";
  Serial.print(msg);
  
  msg = "\nWIFI\n";
  msg += "Device Name:\t\t\"" + nameString + "\"\n";
  msg += "SSID:\t\t\t" + (ssidString.length() > 0 ? '\"' + ssidString + '\"' : "Not Configured") + '\n';
  msg += "Connected:\t\t" + (WiFi.isConnected() ? "True\nIP Address:\t\t" + WiFi.localIP().toString() : "False") + '\n';
  Serial.print(msg);
  
  msg = "\nWEBSOCKET\n";
  msg += "Mode:\t\t\t";
  if (wsMode == WS_MODE_OFF) msg += "Off\n";
  else if (wsMode == WS_MODE_SERVER) msg += "Server\n";
  else if (wsMode == WS_MODE_CLIENT) msg += "Client\n";
  msg += wsMode != WS_MODE_OFF ? "Port:\t\t\t" + (String)wsPort + '\n': "";
  if (wsMode == WS_MODE_SERVER) {
    msg += "Connected clients:\t" + (String)webSocketServer.connectedClients() + '\n';
    for (int i=0; i<webSocketServer.connectedClients(); i++)
      msg += "\tClient " + (String)i + ":\t" + webSocketServer.remoteIP(i).toString() + '\n';
  }
  Serial.print(msg);
  
  msg = "\nIR TRACKER\n";
  msg += "Average Count:\t\t" + (String)IRsensor.getAverageCount() + '\n';
  #if defined(HW_BETA)
    msg += "Frame Period:\t\t" + (String)IRsensor.getFramePeriod() + "ms (Frame Rate: " + (String)(1000/IRsensor.getFramePeriod()) + "Hz)" + '\n';
    msg += "Exposure Time:\t\t" + (String)IRsensor.getExposureTime() + "ms" + '\n';
    msg += "Gain:\t\t\t" + (String)IRsensor.getGain() + '\n';
    msg += "Brightness Threshold:\t" + (String)IRsensor.getPixelBrightnessThreshold() + '\n';
    msg += "Noise Threshold:\t" + (String)IRsensor.getPixelNoiseTreshold() + '\n';
    msg += "Min Area Threshold:\t" + (String)IRsensor.getMinAreaThreshold() + '\n';
    msg += "Max Area Threshold:\t" + (String)IRsensor.getMaxAreaThreshold() + '\n';
    msg += "Number of IR points:\t" + (String)IRsensor.getObjectNumberSetting() + '\n';
  #else
    msg += "Frame Period:\t\t" + (String)IRsensor.getFramePeriod() + "ms (Frame Rate: " + (String)(1000/IRsensor.getFramePeriod()) + "Hz)" + '\n';
    msg += "Gain:\t\t\t" + (String)IRsensor.getGain() + '\n';
    msg += "Brightness Threshold:\t" + (String)IRsensor.getPixelBrightnessThreshold() + '\n';
  #endif
  msg += "Drop Delay Time:\t" + (String)dropDelayTime + "ms\n";
  Serial.print(msg);
  
  msg = "\nCALIBRATION\n";
  msg += "Calibration:\t\t" + (String)(IRsensor.getCalibrationEnable() ? "Enabled" : "Disabled") + '\n';
  msg += "Offset Calibration:\t" + (String)(IRsensor.getCalibrationOffsetEnable() ? "Enabled" : "Disabled") + '\n';
  msg += "Mirror X:\t\t" + (String)(IRsensor.getMirrorX() ? "Enabled" : "Disabled") + '\n';
  msg += "Mirror Y:\t\t" + (String)(IRsensor.getMirrorY() ? "Enabled" : "Disabled") + '\n';
  msg += "Rotation:\t\t" + (String)(IRsensor.getRotation() ? "Enabled" : "Disabled") + '\n';
  msg += "Offset X:\t\t" + (String)IRsensor.getOffsetX() + '\n';
  msg += "Offset Y:\t\t" + (String)IRsensor.getOffsetY() + '\n';
  msg += "Scale X:\t\t" + (String)IRsensor.getScaleX() + '\n';
  msg += "Scale Y:\t\t" + (String)IRsensor.getScaleY() + '\n';
  Serial.print(msg);
  
  Serial.println("-----------------------------------------------------------------------------\n");
}

/**
 * Send all the settings in JSON format to websocket clients and/or serial port
 */
void updateSettings(){
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
  
  #if defined(HW_BETA)
    String msg = "{\"status\":\"update\",\"firmware\":\"" + (String)FIRMWARE_VERSION + "\",\"webserver\":\"" + webserverVersion + "\",\"hardware\":\"Beta\",";
  #elif defined(HW_DIY_FULL)
    String msg = "{\"status\":\"update\",\"firmware\":\"" + (String)FIRMWARE_VERSION + "\",\"webserver\":\"" + webserverVersion + "\",\"hardware\":\"DIY Full\",";
  #else
    String msg = "{\"status\":\"update\",\"firmware\":\"" + (String)FIRMWARE_VERSION + "\",\"webserver\":\"" + webserverVersion + "\",\"hardware\":\"DIY Basic\",";
  #endif
  
  msg += "\"sett\":{\"debug\":" + (String)debug + ",\"serialOut\":" + (String)serialOutput + "},";
  msg += "\"network\":{\"ssid\":\"" + ssidString + "\",\"ipAddress\":\"" + WiFi.localIP().toString().c_str() + "\",\"name\":\"" + nameString + "\",\"connected\":" + (String)WiFi.isConnected() + ",\"wsMode\":\"" + websocketMode + "\",\"wsPort\":" + (String)wsPort + ",\"wsClients\":[" +wsClients + "]},";
  msg += "\"cal\":{\"calibrationEnable\":" + (String)IRsensor.getCalibrationEnable() + ",\"offsetEnable\":" + (String)IRsensor.getCalibrationOffsetEnable() + ",\"mirrorX\":" + (String)IRsensor.getMirrorX() + ",\"mirrorY\":" + (String)IRsensor.getMirrorY() + ",\"rotation\":" + (String)IRsensor.getRotation() + ",\"offsetX\":" + (String)IRsensor.getOffsetX() + ",\"offsetY\":" + (String)IRsensor.getOffsetY() + ",\"scaleX\":" + (String)IRsensor.getScaleX() + ",\"scaleY\":" + (String)IRsensor.getScaleY() + "},";
  #if defined(HW_BETA)
    msg += "\"ir\":{\"averageCount\":" + (String)IRsensor.getAverageCount() + ",\"framePeriod\":" + (String)IRsensor.getFramePeriod() + ",\"exposure\":" + (String)IRsensor.getExposureTime() + ",\"gain\":" + (String)IRsensor.getGain() + ",\"brightness\":" + (String)IRsensor.getPixelBrightnessThreshold() + ",\"noise\":" + (String)IRsensor.getPixelNoiseTreshold() + ",\"minArea\":" + (String)IRsensor.getMinAreaThreshold() + ",\"maxArea\":" + (String)IRsensor.getMaxAreaThreshold() + ",\"points\":" + (String)IRsensor.getObjectNumberSetting() + ",\"dropDelay\":" + (String)dropDelayTime + "}";
  #else
    msg += "\"ir\":{\"averageCount\":" + (String)IRsensor.getAverageCount() + ",\"framePeriod\":" + (String)IRsensor.getFramePeriod() + ",\"gain\":" + (String)IRsensor.getGain() + ",\"brightness\":" + (String)IRsensor.getPixelBrightnessThreshold() + ",\"dropDelay\":" + (String)dropDelayTime + "}";
  #endif
  
  msg += "}";
  // send message to client
  if (wsMode != WS_MODE_OFF) broadcastWs(msg);
  if (serialOutput) Serial.println(msg);
}

/**
 * Send detected IR codes from an IR remote in JSON format to websocket clients and/or serial port
 */
void sendIRcode(char* rcvGroup, uint32_t result) {
  if (debug) Serial.println("IR rec: " + (String)rcvGroup + '\t' + (String)result);
  String msg = "{\"status\":\"IRcode\",\"data\":{\"protocol\":\"" + (String)rcvGroup + "\",\"code\":" + (String)result + "}}";

  if (wsMode != WS_MODE_OFF) broadcastWs(msg);
  if (serialOutput) Serial.println(msg);
}


/*
 * Ping Foundry to make sure ws connection is alive
 */
void pingLoop( void * parameter ) {
  while(1) {
    //Serial.print("Free stack pingLoop: ");
    //Serial.println(uxTaskGetStackHighWaterMark(NULL));
    String msg = "{\"status\":\"ping\",\"source\":\"";
    if (calibrationProcedure == CALIBRATION_INACTIVE) msg += "mainLoop";
    else msg += "calibration";

    msg += "\",\"battery\":{\"voltage\":" + (String)vBat + ",\"percentage\":" + (String)batPercentage + ",\"charging\":" + (String)chargeState + ",\"usbActive\":" + (String)usbActive + "}}";
    broadcastWs(msg);
    if (serialOutput) Serial.println(msg);
    delay(PING_PERIOD);
  }
}

/**
 * Send the detected IR points in JSON format to websocket clients and/or serial port
 */
bool validPointsArray[16];
char msgBuffer[MSG_LENGTH] = "";

bool sendIRpoints(uint8_t IRpoints, bool repeat=false){
  //char msgBuffer[MSG_LENGTH] = "";
  if (repeat == false) {
    sprintf(msgBuffer,"");
    sprintf(msgBuffer+strlen(msgBuffer),"{\"status\":\"IR data\",\"points\":%d,\"data\":[",IRpoints);
    bool msgStarted = false;
    //Check each valid IR point
    for (int i=0; i<IRsensor.getObjectNumberSetting(); i++){
        //If IR point is valid
        if (IRsensor.irPoints[i].valid > 0) {
          //print the data for that IR point
          validPointsArray[i] = true;
          if (msgStarted) sprintf(msgBuffer+strlen(msgBuffer),",");
          sprintf(msgBuffer+strlen(msgBuffer),"{\"point\":%d,\"x\":%.2f,\"y\":%.2f,\"maxBrightness\":%.2f,\"id\":%d,\"command\":%d",i,IRsensor.irPoints[i].x,IRsensor.irPoints[i].y,IRsensor.irPoints[i].maxBrightness,irAddress,irMode);
          #if defined(HW_BETA)
            if (calOpen) sprintf(msgBuffer+strlen(msgBuffer),",\"avgBrightness\":%.2f,\"area\":%d,\"radius\":%d",IRsensor.irPoints[i].avgBrightness,IRsensor.irPoints[i].area,IRsensor.irPoints[i].radius);
          #endif
          sprintf(msgBuffer+strlen(msgBuffer),"}");
          msgStarted = true;
        }
        else if (validPointsArray[i] == true) {
          validPointsArray[i] = false;
          if (msgStarted) sprintf(msgBuffer+strlen(msgBuffer),",");
          sprintf(msgBuffer+strlen(msgBuffer),"{\"point\":%d,\"id\":%d,\"command\":129}",i,irAddress);
          msgStarted = true;
        }
      }
      sprintf(msgBuffer+strlen(msgBuffer),"]}");
  }

  if (serialOutput) Serial.println(msgBuffer);
  broadcastWs(msgBuffer);
  return (IRpoints > 0);
}
