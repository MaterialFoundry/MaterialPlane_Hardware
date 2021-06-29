

int16_t coordinates[16][2], area[16];
uint8_t averageBrightness[16], maxBrightness[16];
uint8_t validPoints = 0;
uint8_t pingCounter = 0;
uint8_t idCounter = 0;
uint8_t validPointsOld = 0;
char msgBuffer[MSG_LENGTH];
uint8_t invalidPointCount = 0;
bool validPointsArray[16];

void readIR(){
  //Get the coordinates
  bool IRstatus = getCoordinates();

  //If no valid points were detected
  if (IRstatus == false) {
    irAddress = 0;
    irMode = 0;
    if (idCounter > 0) idCounter--;
  }
  printMsg();
}

bool getCoordinates(){
  //Get data from the sensor
  IRsensor.getOutput(1);
  //for (int i=0; i<4; i++) Serial.print("\t" + (String)i + " " + (String)IRsensor.pointStorage[i][1] + " " + (String)IRsensor.pointStorage[i][2] + " " + (String)IRsensor.pointStorage[i][4]);
  //Serial.println();

  //Check if max brightness of all points is 0 (HW_BETA) or y coordinate is lower than 5452 (other HW). In which case no IR point was detected.
  uint8_t pointsMeasured = 0;
  #if defined(HW_BETA)
  #else
    if (maxIRpoints > 4) maxIRpoints = 4;
  #endif
  
  for (int i=0; i<maxIRpoints; i++) {
    #if defined(HW_BETA)
      if (IRsensor.pointStorage[i][4] > 0) 
    #else
      if (IRsensor.pointStorage[i][2] < 5452) 
    #endif
      pointsMeasured++;
  }
  //Serial.println("\tPoints: " + (String)pointsMeasured);
      
  if (pointsMeasured == 0) {
    if (invalidPointCount < 255) invalidPointCount++;
    if (invalidPointCount > 5){
      validPoints = 0;
      return false;
    }
    return true;
  }

  #if defined(HW_BETA)
    if (irMode == 0) {
      idCounter++;
      if (idCounter > 20) idCounter = 20;
      else if (idCounter < 5) return true;
    }
  #endif
  
  invalidPointCount = 0;
  validPoints = pointsMeasured;
  
  //Calculate values for all points
  for (int i=0; i<maxIRpoints; i++){
    coordinates[i][0] = IRsensor.pointStorage[i][1];  //x coordinate
    coordinates[i][1] = IRsensor.pointStorage[i][2];  //y coordinate
    maxBrightness[i] = IRsensor.pointStorage[i][4];
    
    if (debug) Serial.print("Point " + (String)i + ": X: " + (String)coordinates[i][0] + "\tY: " + (String)coordinates[i][1] + "\tBrightness: " + (String)maxBrightness[i]);

    if (maxBrightness[i] == 0) {
      if (debug) Serial.println();
      continue;
    }

    //If calibration is enabled, perform homography transform
    if (calibration){
      cal.calculateCoordinates(coordinates[i][0],coordinates[i][1]);
      coordinates[i][0] = cal.getX();
      coordinates[i][1] = cal.getY();
      if (debug) Serial.print("\tCX: " + (String)coordinates[i][0] + "\tCY: " + (String)coordinates[i][1]);
    }

    if (mirrorX)  coordinates[i][0] = 4095-coordinates[i][0];
    if (mirrorY)  coordinates[i][1] = 4095-coordinates[i][1];

    if (rotation){
      uint16_t coordinatesTemp[2] = {coordinates[i][0],coordinates[i][1]};
      coordinates[i][0] = coordinatesTemp[1];
      coordinates[i][1] = coordinatesTemp[0];
    }

    if (debug) Serial.println();
  }
  if (debug) Serial.println("Valid Points: " + (String)validPoints + "\n");
  return true;
}

void printMsg(){
  bool dataStarted = false;
  //Serial.println((String)validPoints + "\t" + (String)validPointsOld);
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
      sprintf(msgBuffer+strlen(msgBuffer),"{\"point\":%d,\"x\":%d,\"y\":%d,\"maxBrightness\":%d,\"id\":%d,\"command\":%d",i,coordinates[i][0],coordinates[i][1],maxBrightness[i],irAddress,irMode);
      #if defined(HW_BETA)
        if (calOpen) sprintf(msgBuffer+strlen(msgBuffer),",\"avgBrightness\":%d,\"area\":%d,\"radius\":%d",IRsensor.pointStorage[i][3],IRsensor.pointStorage[i][0],IRsensor.pointStorage[i][6]);
      #endif
      sprintf(msgBuffer+strlen(msgBuffer),"}");
      dataStarted = true;
      //Serial.print((String)i + ": X:" + (String)coordinates[i][0] + "\tY:" + (String)coordinates[i][1] + "\tB:" + (String)maxBrightness[i] + "\t\t");
    }
    else if (validPointsArray[i] == true) {
      validPointsArray[i] = false;
      if (dataStarted) sprintf(msgBuffer+strlen(msgBuffer),",");
      sprintf(msgBuffer+strlen(msgBuffer),"{\"point\":%d,\"id\":%d,\"command\":2}",i,irAddress);
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
    //Serial.println("\tPoint0: X: " + (String)cal_lastCoordinates[0] + "\tY: " + (String)cal_lastCoordinates[1] + "\tBrightness: " + (String)IRsensor.pointStorage[0][3]);
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
    //Serial.println("Calibration values point " + (String)(cal_count+1) + "/4\tX: " + (String)cal_lastCoordinates[0] + "\tY: " + (String)cal_lastCoordinates[1]);
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
