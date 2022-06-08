void connectWifi(const char* ssid, const char* password, uint8_t ssidLength, uint8_t passwordLength) {
  Serial.println("\nStarting network configuration");
  ssidString = "";
  for (int i=0; i<ssidLength; i++) ssidString += ssid[i];
  
  if (WiFi.isConnected()) {
    Serial.println("Disconnecting from: \"" + (String)WiFi.SSID() + "\"");
  }

  if (ssidLength == 0) Serial.println("SSID not configured, not connecting to WiFi");
  else {
    Serial.print("Attempting to connect to \"" + (String)ssid + "\". Please wait");
    WiFi.disconnect();  
    WiFi.mode(WIFI_OFF); 
    WiFi.mode(WIFI_AP);
    WiFi.begin(ssid,password);

    int counter = 0;
    while(WiFi.status() != WL_CONNECTED) {
      counter++;
      if (counter >= WIFI_TIMEOUT*2) {
        Serial.println("\nConnection failed");
        break;
      }
      delay(500);
      Serial.print(".");
    }
  }
 
  if (WiFi.status() == WL_CONNECTED) {
    String ipAddress = WiFi.localIP().toString().c_str();
    Serial.println("\nWiFi connected with IP address: " + ipAddress + ", using device name: " + nameString);
    configureDNS(nameString);
  }
  else {
    Serial.println("Starting access point");
    //Start access point
    WiFi.disconnect();  
    WiFi.mode(WIFI_OFF); 
    WiFi.mode(WIFI_AP);
    char name[nameString.length()];
    stringToChar(nameString, name);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(name);
    dnsServer.start(53, "*", apIP);
    IPAddress IP = WiFi.softAPIP();
    String ipAddress = IP.toString().c_str();
    Serial.println("Started WiFi access point on IP address: " + ipAddress + ", using device name: " + nameString);
  }

  //Start websocket server
  wsMode = WS_MODE_SERVER;  //for now, force server mode
  if (wsMode == WS_MODE_SERVER) {
    webSocketServer.begin();
    Serial.println("Websocket server started on port: " + (String)wsPort);
  }
  
  //Connect to websocket server
  else if (wsMode == WS_MODE_CLIENT) {
    
  }
  
  webSocketServer.onEvent(webSocketServerEvent);
}

void configureDNS(String name) {
  char hostName[name.length()];
  name.toCharArray(hostName,name.length()+1);

  if (!MDNS.begin(hostName)) {
      Serial.println("Error setting up MDNS responder!");
      while(1){
          delay(1000);
      }
  }
}
