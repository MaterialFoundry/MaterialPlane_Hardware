void initialization() {
  
  //Start the serial port
  Serial.begin(250000);
  delay(100);
  Serial.println("\n-----------------------------------------------------------------------------\nStarting initialization\n");

  IRsensor.initialize();

  //Start and setup EEPROM
  EEPROM.begin(512);
  startupEeprom();
 
  //Disable bluetooth radio
  btStop();

  //Start WiFi
  if (ssidString.length() > 0) {
    char ssid[ssidString.length()];
    char password[passwordString.length()];
    
    ssidString.toCharArray(ssid,ssidString.length()+1);
    passwordString.toCharArray(password,passwordString.length()+1);
  
    connectWifi(ssid, password, ssidString.length(), passwordString.length());
  }
  else {
    Serial.println("SSID not configured, not connecting to WiFi");
  }

  //Start websocket server
  wsMode = WS_MODE_SERVER;  //for now, force server mode
  if (wsMode == WS_MODE_SERVER && WiFi.status() == WL_CONNECTED) {
    webSocketServer.begin();
    Serial.println("Websocket server started on port: " + (String)wsPort + "\n");
  }
  
  //Connect to websocket server
  else if (wsMode == WS_MODE_CLIENT) {
    
  }
  
  webSocketServer.onEvent(webSocketServerEvent);
  
  initializeEepromIRsensor();
  
  #if defined(HW_DIY_BASIC) 

  #elif defined(HW_DIY_FULL)
    //LEDs
    pinMode(LEDL_R,OUTPUT);
    digitalWrite(LEDL_R,LOW);
    pinMode(LEDL_G,OUTPUT);
    digitalWrite(LEDL_G,HIGH);
    pinMode(LEDR_R,OUTPUT);
    digitalWrite(LEDR_R,LOW);
    pinMode(LEDR_G,OUTPUT);
    digitalWrite(LEDR_G,LOW);

    //Disable on-board led
    tp.DotStar_SetPower(false);

    setRightLED(false);
  #elif defined(HW_BETA)
    pinMode(EXPOSURE,INPUT);
    
    //Start ID sensor
    IDsensor.start(IR_IN);

    //Attach interrupt to exposure pin
    attachInterrupt(EXPOSURE, pajInterruptHandler, FALLING);

    //Battery voltage monitor pins
    pinMode(VBAT_EN,OUTPUT);
    pinMode(VBAT_SENSE,INPUT);
    pinMode(VBAT_STAT,INPUT_PULLUP);
  
    //Batter monitoring pins
    pinMode(USB_ACTIVE,INPUT);
    
    //LEDs
    pinMode(LEDL_R,OUTPUT);
    digitalWrite(LEDL_R,LOW);
    pinMode(LEDL_G,OUTPUT);
    digitalWrite(LEDL_G,HIGH);
    pinMode(LEDR_R,OUTPUT);
    digitalWrite(LEDR_R,LOW);
    pinMode(LEDR_G,OUTPUT);
    digitalWrite(LEDR_G,LOW);

    setRightLED(false);
  #endif
  
  printStatus();
  
  Serial.println("\nDone intializing\n-----------------------------------------------------------------------------\n");
}
