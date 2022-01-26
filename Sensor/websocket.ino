bool initialSend = true;

void webSocketServerEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
        if (debug) Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
        {
            IPAddress ip = webSocketServer.remoteIP(num);
            if (debug) Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
            if (initialSend) {
              delay(100);
              initialSend = false;
            }
            updateSettings();
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
