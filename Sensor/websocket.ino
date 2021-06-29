void webSocketServerEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
        if (debug) Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
        {
            IPAddress ip = webSocketServer.remoteIP(num);
            if (debug) Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
            String msg = "{\"status\":\"connected\",\"firmware\":" + (String)FIRMWARE_VERSION + ",\"hardware\":";
            #if defined(HW_BETA)
              msg += "\"BETA\",";
            #elif defined(HW_DIY_FULL)
              msg += "\"DIY_FULL\",";
            #else
              msg += "\"DIY_BASIC\",";
            #endif
            msg += "\"cal\":{\"calibrationEnable\":" + (String)calibration + ",\"offsetEn\":" + (String)offsetOn + ",\"mirrorX\":" + (String)mirrorX + ",\"mirrorY\":" + (String)mirrorY + ",\"rotation\":" + (String)rotation + "},";
            #if defined(HW_BETA)
              msg += "\"ir\":{\"framePeriod\":" + (String)IRsensor.getFramePeriod() + ",\"exposure\":" + (String)IRsensor.getExposureTime() + ",\"gain\":" + (String)IRsensor.getGain() + ",\"brightness\":" + (String)IRsensor.getPixelBrightnessThreshold() + ",\"noise\":" + (String)IRsensor.getPixelNoiseTreshold() + ",\"minArea\":" + (String)IRsensor.getMinAreaThreshold() + ",\"maxArea\":" + (String)IRsensor.getMaxAreaThreshold() + ",\"points\":" + (String)IRsensor.getObjectNumberSetting() + "}";
            #else
              msg += "\"ir\":{\"framePeriod\":" + (String)framePeriod + ",\"gain\":" + (String)IRsensor.getGain() + ",\"brightness\":" + (String)IRsensor.getPixelBrightnessThreshold() + "}";
            #endif
            
            msg += "}";
            // send message to client
            webSocketServer.sendTXT(num, msg);
        }
        break;
    case WStype_TEXT:
        {
          if (debug) Serial.printf("[%u] get Text: %s\n", num, payload);
          String pl = "";
          for (int i=0; i<length; i++) pl += char(payload[i]);
          pl += ' ';
          analyzeMessage(pl);
          
         // if (pl.indexOf("STOP")>=0) stopCal = true;
         // else analyzeData(pl);
        }
        break;
    case WStype_BIN:
        if (debug) Serial.printf("[%u] get binary length: %u\n", num, length);
       // hexdump(payload, length);

        // send message to client
        // webSocketServer.sendBIN(num, payload, length);
        break;
    case WStype_ERROR:      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
    }
}

/*
 * Ping Foundry to make sure ws connection is alive
 */
void wsPing(uint8_t source) {
  if (millis()-pingTimer > PING_PERIOD) {
    pingTimer = millis();
    String msg = "{\"status\":\"ping\",\"source\":\"";
    if (source == PING_MAIN_LOOP) msg += "mainLoop";
    else if (source == PING_CAL) msg += "calibration";
    msg+="\",\"battery\":{\"voltage\":"+(String)vBat+",\"charging\":"+(String)chargeState+"}}";
    webSocketServer.broadcastTXT(msg);
    if (debug) Serial.println(msg);
  }
}
