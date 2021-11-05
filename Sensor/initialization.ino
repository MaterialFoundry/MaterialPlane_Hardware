void initialization() {
  
  //Start the serial port
  Serial.begin(250000);
  delay(100);
  Serial.println("\n-----------------------------------------------------------------------------\nStarting initialization\n");
  
  #if defined(HW_DIY_BASIC) 

  #elif defined(HW_DIY_FULL)
    //Configure LED channels
    ledcSetup(LEDL_R_CH, 5000, 12);
    ledcSetup(LEDL_G_CH, 5000, 12);
    ledcSetup(LEDR_R_CH, 5000, 12);
    ledcSetup(LEDR_G_CH, 5000, 12);
    ledcAttachPin(LEDL_R, LEDL_R_CH);
    ledcAttachPin(LEDL_G, LEDL_G_CH);
    ledcAttachPin(LEDR_R, LEDR_R_CH);
    ledcAttachPin(LEDR_G, LEDR_G_CH);

    //Switch LEDs off
    ledcWrite(LEDL_R_CH, 0);
    ledcWrite(LEDL_G_CH, 0);
    ledcWrite(LEDR_R_CH, 0);
    ledcWrite(LEDR_G_CH, 0);
    
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

    //Charage enable pin
    pinMode(CHARGE_EN,OUTPUT);
    //Enable charging
    digitalWrite(CHARGE_EN,LOW);
  
    //Battery monitoring pins
    pinMode(USB_ACTIVE,INPUT);
    
    //LEDs
    //Configure LED channels
    ledcSetup(LEDL_R_CH, 5000, 8);
    ledcSetup(LEDL_G_CH, 5000, 8);
    ledcSetup(LEDR_R_CH, 5000, 8);
    ledcSetup(LEDR_G_CH, 5000, 8);
    ledcAttachPin(LEDL_R, LEDL_R_CH);
    ledcAttachPin(LEDL_G, LEDL_G_CH);
    ledcAttachPin(LEDR_R, LEDR_R_CH);
    ledcAttachPin(LEDR_G, LEDR_G_CH);

    //Switch LEDs off
    ledcWrite(LEDL_R_CH, 0);
    ledcWrite(LEDL_G_CH, 0);
    ledcWrite(LEDR_R_CH, 0);
    ledcWrite(LEDR_G_CH, 0);

    setRightLED(false);
  #endif

  SPIFFS.begin(); 
  
  IRsensor.initialize();

  //Start and setup EEPROM
  EEPROM.begin(512);
  startupEeprom();
  initializeEepromIRsensor();
 
  //Disable bluetooth radio
  btStop();

  //Start WiFi
  char ssid[ssidString.length()];
  stringToChar(ssidString, ssid);
  char password[passwordString.length()];
  stringToChar(passwordString, password);
  connectWifi(ssid, password, ssidString.length(), passwordString.length());
  
  

  webServer.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(webServer.uri()))                  // send it if it exists
     webServer.send(404, "text/html", getEmptyIndex()); // otherwise, respond with a 404 (Not Found) error
  });

  webServer.begin();
  
  printStatus();
  
  Serial.println("\nDone intializing\n-----------------------------------------------------------------------------\n");
}
