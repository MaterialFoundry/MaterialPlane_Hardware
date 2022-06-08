void initialization() {
  
  //Start the serial port
  Serial.begin(250000);
  delay(100);
  Serial.println("\n-----------------------------------------------------------------------------\nStarting initialization");
  
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

    #if defined(INVERT_LEDS)
      GPIO.func_out_sel_cfg[LEDL_R].inv_sel = 1;
      GPIO.func_out_sel_cfg[LEDL_G].inv_sel = 1;
      GPIO.func_out_sel_cfg[LEDR_R].inv_sel = 1;
      GPIO.func_out_sel_cfg[LEDR_G].inv_sel = 1;
    #endif
    
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

    getBatteryVoltage();
    
  #endif

  SPIFFS.begin(); 
 
  //Start and setup EEPROM
  EEPROM.begin(512);
  startupEeprom();
  
 
  //Disable bluetooth radio
  btStop();

  //Start WiFi
  char ssid[ssidString.length()+1];
  stringToChar(ssidString, ssid);
  char password[passwordString.length()+1];
  stringToChar(passwordString, password);
  connectWifi(ssid, password, ssidString.length(), passwordString.length());

  initializeWebserver();
 
/*
  xTaskCreatePinnedToCore(  
    irSensorLoop,    //function
    "irSensorTask",  //name
    5000,       //stack size
    NULL,       //parameter to pass
    1,          //task priority
    &irSensorTask,        //task handle
    1           //task core
  );
  */
  xTaskCreatePinnedToCore(  
    pingLoop,    //function
    "pingTask",  //name
    3000,       //stack size
    NULL,       //parameter to pass
    1,          //task priority
    &pingTask,        //task handle
    1           //task core
  );
  xTaskCreatePinnedToCore(  
    core0Loop,    //function
    "core0Task",  //name
    5000,       //stack size
    NULL,       //parameter to pass
    1,          //task priority
    &core0Task,        //task handle
    0           //task core
  );
 /* xTaskCreatePinnedToCore(  
    comLoop,    //function
    "comTask",  //name
    6000,       //stack size
    NULL,       //parameter to pass
    2,          //task priority
    &comTask,        //task handle
    0           //task core
  );*/

  IRsensor.initialize();
  initializeEepromIRsensor();
  
  printStatus();
  
  Serial.println("\nDone intializing\n-----------------------------------------------------------------------------\n");
  
}
