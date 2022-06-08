unsigned long IRtimeout = 0;
uint8_t IRpointsOld = 0;
bool validPointsOld2 = false;
unsigned long timer = 0;
unsigned long IRtimer = 0;
unsigned long invalidPointCounter = 0;
bool wasValid = false;
//unsigned long readCount = 0;

void readIR() {
  getCal();
#if defined(HW_BETA)

  if (millis() - IRtimeout >= 1000) {
    bool productId = IRsensor.checkProductId();
    if (productId == false) {
      //Serial.println("Sensor reset " + (String)readCount);
      IRsensor.initialize();
      delay(10);
      initializeEepromIRsensor();
    }
    IRtimeout = millis();
  }
  
  if (exposureDone) {
    //Serial.println("Free stack irSensorTaskLoop: " + (String)uxTaskGetStackHighWaterMark(NULL));
    exposureDone = false;

#else
  if (millis() - IRtimer > IRsensor.getFramePeriod()) {
    IRtimer = millis();

#endif

    bool newPoints = IRsensor.getOutput(1);
    uint8_t IRpoints = IRsensor.detectedPoints;
    //printIrPoints(IRpoints);

    //If no valid points were detected
    if (IRpoints == 0)
      invalidPointCounter++;
    else
      invalidPointCounter = 0;

    if (newPoints && IRpoints > 0 && autoExposureProcedure == EXP_STOPPED) {
      validPointsOld2 = sendIRpoints(IRpoints);
      IRpointsOld = IRpoints;
    }
    else if (invalidPointCounter < dropDelay && wasValid) {
      validPointsOld2 = sendIRpoints(IRpointsOld, true);
    }
    else if (invalidPointCounter >= dropDelay  && wasValid && autoExposureProcedure == EXP_STOPPED) {
      irAddress = 0;
      irMode = 129;
      validPointsOld2 = sendIRpoints(IRpoints);
    }
      
    wasValid = invalidPointCounter >= dropDelay ? false : true;
    
    autoExpose();

    //readCount++;
  }
}

void printIrPoints(uint8_t IRpoints) {
  Serial.print("points: ");
  Serial.print(IRpoints);
  Serial.print("\tvalid: ");
  Serial.print(IRsensor.irPoints[0].valid);
  Serial.print("\tx: ");
  Serial.print(IRsensor.irPoints[0].x);
  Serial.print("\ty: ");
  Serial.print(IRsensor.irPoints[0].y);
  Serial.print("\txRaw: ");
  Serial.print(IRsensor.irPoints[0].xRaw);
  Serial.print("\tyRaw: ");
  Serial.print(IRsensor.irPoints[0].yRaw);
  Serial.println();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


float coordinates[16][2], maxBrightness[16];
uint16_t area[16];
uint8_t validPoints = 0;
uint8_t pingCounter = 0;
uint8_t validPointsOld = 0;
uint8_t invalidPointCount = 0;
uint8_t averageCounter;
float averageCoordinates[16][2];
float averageBrightness[16];
bool printData = false;
String calMode;

int16_t cal_lastCoordinates[4][2];                //stores the last valid coordinates
int16_t cal_localCalCoordinates[4][2];         //stores the calibration points
uint8_t cal_count = 0;                          //stores the calibration cound
bool mirrorX_old, mirrorY_old, rotation_old, calibration_old, offset_old = false;
float scaleX_old, scaleY_old = 1;

void getCal() {

  if (calibrationProcedure == CALIBRATION_INACTIVE) return;

  else if (calibrationProcedure == CALIBRATION_STARTING) {
    calibrationProcedure = CALIBRATION_ACTIVE;
    calibrationRunning = true;

    Serial.println("Starting calibration procedure. Move the base to the correct point and send \"CAL NEXT\" to proceed to the next point. \"CAL CANCEL\" cancels the calibration procedure.");

    if (calibrationMode == CALIBRATION_SINGLE) calMode = "single";
    else if (calibrationMode == CALIBRATION_OFFSET) calMode = "offset";
    else if (calibrationMode == CALIBRATION_MULTI) calMode = "multi";
    else return;
    String msg = "{\"status\":\"calibration\",\"mode\":\"" + calMode + "\",\"state\":\"starting\"}";
    if (wsMode != WS_MODE_OFF) broadcastWs(msg);
    if (serialOutput) Serial.println(msg);

    cal_count = 0;
    for (int i = 0; i < 4; i++) {
      cal_lastCoordinates[i][0] = 0;
      cal_lastCoordinates[i][1] = 0;
      cal_localCalCoordinates[i][0] = 0;
      cal_localCalCoordinates[i][1] = 0;
    }

    mirrorX_old = IRsensor.getMirrorX();
    mirrorY_old = IRsensor.getMirrorY();
    rotation_old = IRsensor.getRotation();
    calibration_old = IRsensor.getCalibrationEnable();
    offset_old = IRsensor.getCalibrationOffsetEnable();
    scaleX_old = IRsensor.getScaleX();
    scaleY_old = IRsensor.getScaleY();
    IRsensor.setMirrorX(false);
    IRsensor.setMirrorY(false);
    IRsensor.setRotation(false);
    if (calibrationMode != CALIBRATION_OFFSET) IRsensor.setCalibrationEnable(false);
    IRsensor.setCalibrationOffsetEnable(false);
    IRsensor.setScaleX(1);
    IRsensor.setScaleY(1);
  }

  else if (calibrationProcedure == CALIBRATION_ACTIVE) {
    //Read the IR sensor and check if valid IR points were detected. If so, store them in the lastCoordinates array

    if (IRsensor.detectedPoints > 0) {
      for (int i = 0; i < 4; i++) {
        cal_lastCoordinates[i][0] = (int16_t)IRsensor.irPoints[i].x;
        cal_lastCoordinates[i][1] = (int16_t)IRsensor.irPoints[i].y;
      }
    }
    //Serial.println((String)IRsensor.detectedPoints + '\t' + (String)cal_lastCoordinates[0][0] + '\t' + (String)cal_lastCoordinates[0][1]);
    delay(25);
  }

  else if (calibrationProcedure == CALIBRATION_CANCEL) {
    Serial.println("Calibration cancelled");
    String msg = "{\"status\":\"calibration\",\"mode\":\"" + calMode + "\",\"state\":\"cancelled\"}";
    if (wsMode != WS_MODE_OFF) broadcastWs(msg);
    if (serialOutput) Serial.println(msg);
    calibrationProcedure = CALIBRATION_INACTIVE;
    calibrationRunning = false;
    IRsensor.setMirrorX(mirrorX_old);
    IRsensor.setMirrorY(mirrorY_old);
    IRsensor.setRotation(rotation_old);
    IRsensor.setCalibrationEnable(calibration_old);
    IRsensor.setCalibrationOffsetEnable(offset_old);
    IRsensor.setScaleX(scaleX_old);
    IRsensor.setScaleY(scaleY_old);
    return;
  }

  //If the message was "CAL NEXT" store the last valid coordinates in the local calibration coordinate array and increment the count
  else if (calibrationProcedure == CALIBRATION_NEXT) {
    String msg = "";
    if (calibrationMode != CALIBRATION_MULTI) {
      calibrationProcedure = CALIBRATION_ACTIVE;
      cal_localCalCoordinates[cal_count][0] = cal_lastCoordinates[0][0];
      cal_localCalCoordinates[cal_count][1] = cal_lastCoordinates[0][1];

      msg = "{\"status\":\"calibration\",\"mode\":\"" + calMode + "\",\"state\":" + (String)(cal_count + 1) + ",\"x\":" + (String)cal_lastCoordinates[0][0] + ",\"y\":" + (String)cal_lastCoordinates[0][1] + "}";
      cal_count++;
    }
    else {
      if (IRsensor.detectedPoints >= 4) {
        msg = "{\"status\":\"calibration\",\"mode\":\"" + calMode + "\",\"state\":4,\"points\":[";
        uint8_t point = 0;
        for (int i = 0; i < 16; i++) {
          if (IRsensor.irPoints[i].maxBrightness == 0) continue;
          cal_localCalCoordinates[point][0] = (int16_t)IRsensor.irPoints[i].x;
          cal_localCalCoordinates[point][1] = (int16_t)IRsensor.irPoints[i].y;
          msg += "{\"point\":" + (String)point + ",\"x\":" + (String)IRsensor.irPoints[i].x + ",\"y\":" + (String)IRsensor.irPoints[i].y + '}';
          if (point < 3) msg += ',';
          point++;
          if (point > 3) break;
        }
        msg += "]}";
        cal_count = 4;
      }
    }

    if (wsMode != WS_MODE_OFF) broadcastWs(msg);
    if (serialOutput) Serial.println(msg);
  }

  if (cal_count == 4) {
    Serial.println("\nCalibration done");
    String msg = "{\"status\":\"calibration\",\"mode\":\"" + calMode + "\",\"state\":\"done\"}";
    if (wsMode != WS_MODE_OFF) broadcastWs(msg);
    if (serialOutput) Serial.println(msg);

    if (calibrationMode == CALIBRATION_OFFSET) {
      for (int i = 0; i < 4; i++) {
        IRsensor.setCalibrationOffsetArray(i, 0, cal_localCalCoordinates[i][0]);
        IRsensor.setCalibrationOffsetArray(i, 1, cal_localCalCoordinates[i][1]);
      }
      IRsensor.setCalibrationOffsetEnable(true);
      setEepromCalOffset(true);
      IRsensor.setCalibrationEnable(calibration_old);

      //Order the calibration points and calculate the homography array
      IRsensor.orderCalibrationOffsetArray();

      //Store the calibration points in the EEPROM
      storeCalOffset();
    }
    else {
      for (int i = 0; i < 4; i++) {
        IRsensor.setCalibrationArray(i, 0, cal_localCalCoordinates[i][0]);
        IRsensor.setCalibrationArray(i, 1, cal_localCalCoordinates[i][1]);
      }
      IRsensor.setCalibrationOffsetEnable(offset_old);
      IRsensor.setCalibrationEnable(true);
      setEepromCalCalibration(true);

      //Order the calibration points and calculate the homography array
      IRsensor.orderCalibrationArray();

      //Store the calibration points in the EEPROM
      storeCal();
    }

    IRsensor.setMirrorX(mirrorX_old);
    IRsensor.setMirrorY(mirrorY_old);
    IRsensor.setRotation(rotation_old);
    IRsensor.setScaleX(scaleX_old);
    IRsensor.setScaleY(scaleY_old);
    calibrationProcedure = CALIBRATION_INACTIVE;
    calibrationRunning = false;
    updateSettings();
  }
}



#if defined(HW_BETA)
  float autoExposeExposure = 0.1;
  uint8_t autoExposureMinBrightness = 30;
#else
  uint8_t autoExposureMinBrightness = 3;
#endif

uint8_t autoExposureStartCounter = 0;
float autoExposeGain = 1;

void autoExpose() {

  if (autoExposureProcedure == EXP_STOPPED) return;

  if (autoExposureProcedure == EXP_START) {
    Serial.println("Starting auto exposure\nMake sure you have 1 base or the pen in view of the sensor with its IR LED on.");
    #if defined(HW_BETA)
      autoExposeExposure = 0.1;
      autoExposureMinBrightness = 30;
      IRsensor.setPixelNoiseTreshold(0);
      IRsensor.setGain(autoExposeGain);
      IRsensor.setExposureTime(autoExposeExposure);
    #else
      autoExposureMinBrightness = 3;
    #endif
    
    autoExposeGain = 1;
    autoExposureStartCounter = 0;
    IRsensor.setFramePeriod(25);
    IRsensor.setPixelBrightnessThreshold(autoExposureMinBrightness);
    IRsensor.setAverageCount(1);
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
    for (int i = 0; i < 16; i++) {
      if (IRsensor.irPoints[i].maxBrightness > brightestBrightness) {
        brightestPoint = i;
        brightestBrightness = IRsensor.irPoints[i].maxBrightness;
      }
    }

    #if defined(HW_BETA)
    if (brightestBrightness >= 200) {
    #else
    if (brightestBrightness >= 10) {
    #endif
      autoExposureProcedure = EXP_SETMINBRIGHTNESS;
      autoExposureMinBrightness = 0;
      return;
    }

    #if defined(HW_BETA)
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
      Serial.println("autoexpose. E: " + (String)autoExposeExposure + "\tG: " + (String)autoExposeGain + "\tB: " + (String)brightestBrightness + "\tnum: " + (String)IRsensor.detectedPoints);
    #else
        autoExposeGain += 0.05;
        IRsensor.setGain(autoExposeGain);
        if (autoExposeGain > 7.8) {
          autoExposureProcedure = EXP_SETMINBRIGHTNESS;
          autoExposureMinBrightness = 0;
        }
        Serial.println("autoexpose. G: " + (String)autoExposeGain + "\tB: " + (String)brightestBrightness + "\tnum: " + (String)IRsensor.detectedPoints);
    #endif
  }

  if (autoExposureProcedure == EXP_SETMINBRIGHTNESS) {
  
    //Get the brightest and second brightest point
    uint8_t brightestPoint = 0;
    uint8_t secondBrightestPoint = 0;
    uint8_t brightestBrightness = 0;
    uint8_t secondBrightestBrightness = 0;
    uint8_t points = 0;
    #if defined(HW_BETA)
    for (int i = 0; i < 16; i++) {
    #else
    for (int i = 0; i < 4; i++) {
    #endif
      if (IRsensor.irPoints[i].maxBrightness > 0) points++;

      if (IRsensor.irPoints[i].maxBrightness > brightestBrightness) {
        brightestPoint = i;
        brightestBrightness = IRsensor.irPoints[i].maxBrightness;
      }
      else if (IRsensor.irPoints[i].maxBrightness > secondBrightestBrightness) {
        secondBrightestPoint = i;
        secondBrightestBrightness = IRsensor.irPoints[i].maxBrightness;
      }
    }

    if (secondBrightestBrightness > 0) {
      autoExposureMinBrightness = 0.5 * (brightestBrightness - secondBrightestBrightness);
    }
    else
    #if defined(HW_BETA)
      autoExposureMinBrightness = 75;
    #else
      autoExposureMinBrightness = 5;
    #endif

    #if defined(HW_BETA)
      float framePeriod = IRsensor.getExposureTime() + 2.7;
      uint8_t avg = (1 / (0.001 * framePeriod)) / 60;
      framePeriod = 1000 / (avg * 60);
      if (framePeriod < IRsensor.getExposureTime() + 2.7) framePeriod = IRsensor.getExposureTime() + 2.7;
      if (framePeriod < 5) framePeriod = 5;
      else if (framePeriod > 100) framePeriod = 100;
    #else
      float framePeriod = 15;
      uint8_t avg = 1;
    #endif

    String msg = "{\"status\":\"Auto Exposure Done\"}";
    // send message to client
    if (wsMode != WS_MODE_OFF) broadcastWs(msg);
    if (serialOutput) Serial.println(msg);

    IRsensor.setPixelBrightnessThreshold(autoExposureMinBrightness);
    IRsensor.setFramePeriod(framePeriod);
    IRsensor.setAverageCount(avg);
    setEepromAvg(avg);
    setEepromIrGain(IRsensor.getGain());
    setEepromIrBrightness(autoExposureMinBrightness);
    setEepromIrFramePeriod(framePeriod);
    #if defined(HW_BETA)
      setEepromIrExposure(IRsensor.getExposureTime());
      setEepromIrNoise(0);
    #endif
    
    updateSettings();

    autoExposureProcedure = EXP_STOPPED;
    return;
  }

}
