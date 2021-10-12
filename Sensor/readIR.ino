

float coordinates[16][2], maxBrightness[16];
uint16_t area[16];
uint8_t validPoints = 0;
uint8_t pingCounter = 0;
uint8_t idCounter = 0;
uint8_t validPointsOld = 0;
char msgBuffer[MSG_LENGTH];
uint8_t invalidPointCount = 0;
bool validPointsArray[16];

uint8_t averageCounter;
float averageCoordinates[16][2];
float averageBrightness[16];

bool printData = false;

void readIR(){
  //Get the coordinates
  uint8_t IRstatus = getCoordinates();

  //If no valid points were detected
  if (IRstatus == 0) {
    irAddress = 0;
    irMode = 0;
    if (idCounter > 0) idCounter--;
  }
  if (IRstatus == 1)
    return;

  if (printData || (validPoints == 0 && validPointsOld != 0)) {
    printData = false;
    averageCounter = 0;
    printMsg();
  }
}

uint8_t getCoordinates(){
  //Get data from the sensor
  IRsensor.getOutput(1);

  //Check if max brightness of all points is 0 (HW_BETA) or y coordinate is lower than 5452 (other HW). In which case no IR point was detected.
  uint8_t pointsMeasured = 0;
  
  for (int i=0; i<maxIRpoints; i++) {
    #if defined(HW_BETA)
      if (IRsensor.pointStorage[i][4] > 0) 
    #else
      if (IRsensor.pointStorage[i][2] < 5452) 
    #endif
      pointsMeasured++;
  }
      
  if (pointsMeasured == 0) {
    if (invalidPointCount < 255) invalidPointCount++;
    if (invalidPointCount > 5){
      validPoints = 0;
      return 0;
    }
    return 2;
  }

  #if defined(HW_BETA)
    if (irMode == 0) {
      idCounter++;
      if (idCounter > 20) idCounter = 20;
      else if (idCounter < 5) return 2;
    }
  #endif
  
  invalidPointCount = 0;
  validPoints = pointsMeasured;

  averageCounter++;
  
  //Calculate values for all points
  for (int i=0; i<maxIRpoints; i++){
    float xCoord = IRsensor.pointStorage[i][1];  
    float yCoord = IRsensor.pointStorage[i][2];  
    uint16_t brightness = IRsensor.pointStorage[i][4];

    if (debug) Serial.print("Point " + (String)i + ": X: " + (String)xCoord + "\tY: " + (String)yCoord + "\tBrightness: " + (String)brightness);

    if (brightness == 0) {
      if (debug) Serial.println();
      continue;
    }

    //If calibration is enabled, perform homography transform
    if (calibration){
      cal.calculateCoordinates(xCoord,yCoord);
      xCoord = cal.getX();
      yCoord = cal.getY();
      if (debug) Serial.print("\tCX: " + (String)xCoord + "\tCY: " + (String)yCoord);
    }

    if (rotation){
      float xTemp = xCoord;
      xCoord = yCoord;
      yCoord = xTemp;
    }

    if (mirrorX)  xCoord = 4095-xCoord;
    if (mirrorY)  yCoord = 4095-yCoord;

    xCoord += offsetX;
    yCoord += offsetY;

    if (debug) Serial.println();

    averageCoordinates[i][0] += xCoord;  
    averageCoordinates[i][1] += yCoord; 
    averageBrightness[i] += brightness;
    
    if (averageCounter >= averageCount) {
      coordinates[i][0] = averageCoordinates[i][0] / averageCounter;  //x coordinate
      coordinates[i][1] = averageCoordinates[i][1] / averageCounter;  //y coordinate
      maxBrightness[i] = averageBrightness[i] / averageCounter;
    
      averageCoordinates[i][0] = 0;
      averageCoordinates[i][1] = 0;
      averageBrightness[i] = 0;
      printData = true;
    }
  }
  if (debug) Serial.println("Valid Points: " + (String)validPoints + "\n");
  return 2;
}

void printMsg(){
  bool dataStarted = false;
  
  if (validPoints == 0 && validPointsOld == 0 && debug == false) return;
  
  //Clear the message buffer
  sprintf(msgBuffer,"");
  
  sprintf(msgBuffer+strlen(msgBuffer),"{\"status\":\"IR data\",\"points\":%d,\"data\":[",validPoints);
  
  //Check each valid IR point
  for (int i=0; i<maxIRpoints; i++){
    
    #if defined(HW_BETA)
      if (IRsensor.pointStorage[i][3] > 0) {
    #else
      if (IRsensor.pointStorage[i][2] < 5452) {
    #endif
      validPointsArray[i] = true;
      //print the data for that IR point
      if (dataStarted) sprintf(msgBuffer+strlen(msgBuffer),",");
      sprintf(msgBuffer+strlen(msgBuffer),"{\"point\":%d,\"x\":%.2f,\"y\":%.2f,\"maxBrightness\":%.2f,\"id\":%d,\"command\":%d",i,coordinates[i][0],coordinates[i][1],maxBrightness[i],irAddress,irMode);
      #if defined(HW_BETA)
        if (calOpen) sprintf(msgBuffer+strlen(msgBuffer),",\"avgBrightness\":%d,\"area\":%d,\"radius\":%d",IRsensor.pointStorage[i][3],IRsensor.pointStorage[i][0],IRsensor.pointStorage[i][6]);
      #endif
      sprintf(msgBuffer+strlen(msgBuffer),"}");
      dataStarted = true;
    }
    else if (validPointsArray[i] == true) {
      validPointsArray[i] = false;
      if (dataStarted) sprintf(msgBuffer+strlen(msgBuffer),",");
      sprintf(msgBuffer+strlen(msgBuffer),"{\"point\":%d,\"id\":%d,\"command\":129}",i,irAddress);
      dataStarted = true;
    }
  }
  sprintf(msgBuffer+strlen(msgBuffer),"]}");
  validPointsOld = validPoints;
  if (serialOutput) Serial.println(msgBuffer);

  webSocketServer.broadcastTXT(msgBuffer);
}

#define CALIBRATION_CANCEL  4
#define CALIBRATION_NEXT    3
#define CALIBRATION_ACTIVE  2
#define CALIBRATION_STARTING  1
#define CALIBRATION_INACTIVE  0

uint16_t cal_lastCoordinates[2];                //stores the last valid coordinates
  uint16_t cal_localCalCoordinates[4][2];         //stores the calibration points
  uint8_t cal_count = 0;                          //stores the calibration cound
  bool mirrorX_old = false;
  bool mirrorY_old = false;
  bool rotation_old = false;
  bool calibration_old = false;

void getCal(){
  if (calibrationProcedure == CALIBRATION_INACTIVE) return;
  
  else if (calibrationProcedure == CALIBRATION_STARTING) {
    calibrationProcedure = CALIBRATION_ACTIVE;
    calibrationRunning = true;
    
    Serial.println("Starting calibration procedure. Move the base to the correct point and send \"CAL NEXT\" to proceed to the next point. \"CAL CANCEL\" cancels the calibration procedure.");
    
    String msg = "{\"status\":\"calibration\",\"state\":\"starting\"}";
    if (wsMode != WS_MODE_OFF) webSocketServer.broadcastTXT(msg);
    if (serialOutput) Serial.println(msg);
    cal_count = 0;
    for (int i=0; i<4; i++) {
      cal_localCalCoordinates[i][0];
      cal_localCalCoordinates[i][1];
    }
    cal_lastCoordinates[0] = 0;
    cal_lastCoordinates[1] = 0;

    mirrorX_old = mirrorX;
    mirrorY_old = mirrorY;
    rotation_old = rotation;
    calibration_old = calibration;
    mirrorX = false;
    mirrorY = false;
    rotation = false;
    calibration = false;
  }
  else if (calibrationProcedure == CALIBRATION_ACTIVE) {
    //Read the IR sensor and check if valid IR points were detected. If so, store them in the lastCoordinates array
    #if defined(HW_BETA)
      if (IRsensor.pointStorage[0][3] > 0) {
    #else
      if (IRsensor.pointStorage[0][1] < 5452) {
    #endif
        cal_lastCoordinates[0] = coordinates[0][0];
        cal_lastCoordinates[1] = coordinates[0][1];
      }
    delay(25);
  }
  else if (calibrationProcedure == CALIBRATION_CANCEL) {
      Serial.println("Calibration cancelled");
      String msg = "{\"status\":\"calibration\",\"state\":\"cancelled\"}";
      if (wsMode != WS_MODE_OFF) webSocketServer.broadcastTXT(msg);
      if (serialOutput) Serial.println(msg);
      calibrationProcedure = CALIBRATION_INACTIVE;
      calibrationRunning = false;
      calibration = calibration_old;
      mirrorX = mirrorX_old;
      mirrorY = mirrorY_old;
      rotation = rotation_old;
      return;
  }

  //If the message was "CAL NEXT" store the last valid coordinates in the local calibration coordinate array and increment the count
  else if (calibrationProcedure == CALIBRATION_NEXT){
    calibrationProcedure = CALIBRATION_ACTIVE;
    cal_localCalCoordinates[cal_count][0] = cal_lastCoordinates[0];
    cal_localCalCoordinates[cal_count][1] = cal_lastCoordinates[1];

    String msg = "{\"status\":\"calibration\",\"state\":" + (String)(cal_count+1) + ",\"x\":" + (String)cal_lastCoordinates[0] + ",\"y\":" + (String)cal_lastCoordinates[1] + "}";
    if (wsMode != WS_MODE_OFF) webSocketServer.broadcastTXT(msg);
    if (serialOutput) Serial.println(msg);
    cal_count++;
  }

  if (cal_count == 4) {
    Serial.println("\nCalibration done");
    String msg = "{\"status\":\"calibration\",\"state\":\"done\"}";
    if (wsMode != WS_MODE_OFF) webSocketServer.broadcastTXT(msg);
    if (serialOutput) Serial.println(msg);
    for (int i=0; i<4; i++){
      cal.setCalArray(i, 0, cal_localCalCoordinates[i][0]);
      cal.setCalArray(i, 1, cal_localCalCoordinates[i][1]);
      Serial.println((String)i + "\tX: " + (String)cal_localCalCoordinates[i][0] + "\tY: " + (String)cal_localCalCoordinates[i][1]);
    }

    //Order the calibration points and calculate the homography array
    cal.orderCalArray();

    //Store the calibration points in the EEPROM
    storeCal();

    mirrorX = mirrorX_old;
    mirrorY = mirrorY_old;
    rotation = rotation_old;
    calibration = true;
    setEepromCalCalibration(calibration);
    calibrationProcedure = CALIBRATION_INACTIVE;
    calibrationRunning = false;
  }
}

#define EXP_STOPPED           0
#define EXP_PRESTART          1
#define EXP_START             2
#define EXP_SETEXP            3
#define EXP_SETMINBRIGHTNESS  4

uint8_t autoExposureProcedure = EXP_STOPPED;
uint8_t autoExposureStartCounter = 0;
float autoExposeExposure = 0.1;
float autoExposeGain = 1;
uint8_t autoExposureMinBrightness = 30;

void autoExpose() {
  #if defined(HW_DIY_BASIC) 

  #elif defined(HW_DIY_FULL)

  #elif defined(HW_BETA)

    if (autoExposureProcedure == EXP_STOPPED) return;
  
    if (autoExposureProcedure == EXP_START) {
      Serial.println("Starting auto exposure\nMake sure you have 1 base or the pen in view of the sensor with its IR LED on.");
      autoExposeExposure = 0.1;
      autoExposeGain = 1;
      autoExposureStartCounter = 0;
      IRsensor.setFramePeriod(25);
      IRsensor.setPixelBrightnessThreshold(autoExposureMinBrightness);
      IRsensor.setPixelNoiseTreshold(0);
      IRsensor.setGain(autoExposeGain);
      setEepromAvg(0);
      IRsensor.setExposureTime(autoExposeExposure);
      autoExposureProcedure = EXP_PRESTART;
    }
    if (autoExposureProcedure == EXP_PRESTART) {
      autoExposureStartCounter++;
      if (autoExposureStartCounter > 10) autoExposureProcedure = EXP_SETEXP;
    }
    if (autoExposureProcedure == EXP_SETEXP) {
      
      //Get the brightest point
      uint8_t brightestPoint = 0;
      uint8_t brightestBrightness = 0;
      for (int i=0; i<16; i++) {
        if (IRsensor.pointStorage[i][4] > brightestBrightness) {
          brightestPoint = i;
          brightestBrightness = IRsensor.pointStorage[i][4];
        }
      }
      
      if (brightestBrightness >= 200) {
        autoExposureProcedure = EXP_SETMINBRIGHTNESS;
        autoExposureMinBrightness = 0;
        return;
      }
      autoExposeExposure += 0.25;
      if (autoExposeExposure > 13) {
        autoExposeGain += 1;
        IRsensor.setGain(autoExposeGain);
        autoExposeExposure = 5;
        if (autoExposeGain > 8) {
          autoExposureProcedure = EXP_STOPPED;
          return;
        }
      }
      IRsensor.setExposureTime(autoExposeExposure);
    }
    if (autoExposureProcedure == EXP_SETMINBRIGHTNESS) {
      //Get the brightest and second brightest point
      uint8_t brightestPoint = 0;
      uint8_t secondBrightestPoint = 0;
      uint8_t brightestBrightness = 0;
      uint8_t secondBrightestBrightness = 0;
      uint8_t points = 0;
      for (int i=0; i<16; i++) {
        if (IRsensor.pointStorage[i][4] > 0) points++;
      
        if (IRsensor.pointStorage[i][4] > brightestBrightness) {
          brightestPoint = i;
          brightestBrightness = IRsensor.pointStorage[i][4];
        }
        else if (IRsensor.pointStorage[i][4] > secondBrightestBrightness) {
          secondBrightestPoint = i;
          secondBrightestBrightness = IRsensor.pointStorage[i][4];
        }
      }
  
      if (secondBrightestBrightness > 0) {
        autoExposureMinBrightness = 0.5*(brightestBrightness - secondBrightestBrightness);
      }
      else
        autoExposureMinBrightness = 25;
  
      float framePeriod = IRsensor.getExposureTime() + 2.7;
      if (framePeriod < 5) framePeriod = 5;
      else if (framePeriod > 100) framePeriod = 100;
      
      String msg = "{\"status\":\"Auto Exposure Done\"}";
      // send message to client
      if (wsMode != WS_MODE_OFF) webSocketServer.broadcastTXT(msg);
      if (serialOutput) Serial.println(msg);
  
      IRsensor.setPixelBrightnessThreshold(autoExposureMinBrightness);
      IRsensor.setFramePeriod(framePeriod);
  
      setEepromAvg(0);
      setEepromIrGain(IRsensor.getGain());
      setEepromIrBrightness(autoExposureMinBrightness);
      setEepromIrFramePeriod(framePeriod);
      setEepromIrExposure(IRsensor.getExposureTime());
      setEepromIrNoise(0);
      
      updateSetting();
      
      autoExposureProcedure = EXP_STOPPED;
      return;
    }
    
    #endif
}
  
