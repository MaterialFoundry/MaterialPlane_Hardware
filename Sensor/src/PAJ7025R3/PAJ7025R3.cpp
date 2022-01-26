#include "PAJ7025R3.h"
#include "Arduino.h"
#include <SPI.h>
#include "../Homography/homography.h"

homography cal;
homography offset;

PAJ7025R3::PAJ7025R3(uint8_t cs){
  _cs = cs;
  SPI.begin();
  pinMode(_cs,OUTPUT); //set chip select pin
  digitalWrite(_cs,HIGH);
  SPI.beginTransaction(SPISettings(14000000, LSBFIRST, SPI_MODE3));
}

void PAJ7025R3::initialize(){
  powerOn(true);
  delay(1);
  if (checkProductId() == false){
    Serial.println("Error connecting to sensor, retrying");
    delay(500);
    initialize();
    return;
  }
  //else
  //  Serial.println("Connected to sensor");

  setFrameSubstraction(0);
  setResolutionScale(4095,4095);
  setObjectLabelingMode(1);
  setBarOrientationRatio(3);
  setExposureSignal(true);
}

uint32_t PAJ7025R3::readRegister(uint8_t reg, uint8_t bytesToRead){
  uint32_t val = 0;
  digitalWrite(_cs,LOW);
  if (bytesToRead == 1) SPI.transfer(0x80);
  else SPI.transfer(0x81);
  SPI.transfer(reg);
  for (int i=0; i<bytesToRead; i++)
    val |= SPI.transfer(0x00) << (8*i);
  digitalWrite(_cs,HIGH);
  return val;
}

/*
 * Write to the sensor register
 */
void PAJ7025R3::writeRegister(uint8_t reg, uint32_t val, uint8_t bytesToWrite){
  digitalWrite(_cs,LOW);
  if (bytesToWrite == 1) SPI.transfer(0x00);
  else SPI.transfer(0x01);
  SPI.transfer(reg);
  //SPI.transfer(val);
  for (int i=0; i<bytesToWrite; i++)
    SPI.transfer((val>>(i*8)) & 0xFF);
  digitalWrite(_cs,HIGH);
}

void PAJ7025R3::switchBank(uint8_t bank){
  writeRegister(Bank_Select, bank); //switch to register bank
}

/*
 * Set the power state of the sensor
 */
void PAJ7025R3::powerOn(bool en){
  switchBank(0x00); //switch to register bank 0x00
  if (en)
    writeRegister(Cmd_Manual_PowerControl,0x05); //cmd_manual_powercontrol_power && cmd_manual_powercontrol_sensor On = 1
  else 
    writeRegister(Cmd_Manual_PowerControl,0x00); //cmd_manual_powercontrol_power && cmd_manual_powercontrol_sensor On = 0
  writeRegister(Cmd_Manual_PowerControl_Update_Req,0x00); //make the cmd_manual_powercontrol setting effective
  writeRegister(Cmd_Manual_PowerControl_Update_Req,0x01); //make the cmd_manual_powercontrol setting effective
}

/*
 * Reset the sensor
 */
void PAJ7025R3::resetSensor(){
  switchBank(0x00);
  writeRegister(Cmd_Global_RESETN,0x00); //cmd_global_resetn
}

bool PAJ7025R3::getFrameSubstration(){
  switchBank(0x00);
  return readRegister(Cmd_FrameSubstraction_On);
}

void PAJ7025R3::setFrameSubstraction(bool val){
  switchBank(0x00);
  writeRegister(Cmd_FrameSubstraction_On,val);
}

/*
 * Get the frame period of the sensor
 */
float PAJ7025R3::getFramePeriod(){
  switchBank(0x0C);
  framePeriod = readRegister(Cmd_frame_period, 3);
  return 0.0001*framePeriod;
}

void PAJ7025R3::setFramePeriod(float period){
  if (period > 100) period = 100;
  if (period < 5) period = 5;
  framePeriod = 10000*period;
  switchBank(0x0C);
  writeRegister(Cmd_frame_period,framePeriod, 3);
}

float PAJ7025R3::getExposureTime(){
  switchBank(0x01);
  exposureTime = readRegister(B_expo_R, 2);
  return exposureTime * 0.0002;
}

void PAJ7025R3::setExposureTime(float val){
  float maxExposureTime = (float)framePeriod*0.0001-2.7;
  if (val > maxExposureTime) val = maxExposureTime;
  if (val > 13) val = 13;
  if (val < 0.02) val = 0.02;
  
  exposureTime = val*5000;

  switchBank(0x0C);
  writeRegister(B_expo,(uint16_t)exposureTime,2);
  switchBank(0x01);
  writeRegister(Bank1_Sync_Update_Flag,0x01);
}

float PAJ7025R3::getGain(){
  switchBank(0x01);
  gain = (1 + 0.0625*readRegister(B_global_R));
  uint8_t H = readRegister(B_ggh_r);
  if (H == 0x02) gain *= 2;
  else if (H == 0x03) gain *= 4;
  return gain;
}

void PAJ7025R3::setGain(float gainTemp){
  if (gainTemp < 1) gainTemp = 1;
  if (gainTemp > 8) gainTemp = 8;
  uint8_t _B_global, _B_ggh;
  if (gainTemp < 2) {
    _B_global = (gainTemp - 1)/0.0625;
    _B_ggh = 0x00;
  }
  else if (gainTemp < 4) {
    _B_global = (gainTemp - 2)/0.125;
    _B_ggh = 0x02;
  }
  else {
    _B_global = (gainTemp - 4)/0.25;
    _B_ggh = 0x03;
  }
  
  gain = (1 + 0.0625*readRegister(B_global_R));
  if (_B_ggh == 0x02) gain *= 2;
  else if (_B_ggh == 0x03) gain *= 4;
 
  switchBank(0x0C);
  writeRegister(B_global, _B_global);
  writeRegister(B_ggh, _B_ggh);
  switchBank(0x01);
  writeRegister(0x01,0x01);
}

uint8_t PAJ7025R3::getPixelBrightnessThreshold(){
  switchBank(0x0C);
  return readRegister(Cmd_thd);
}

void PAJ7025R3::setPixelBrightnessThreshold(uint8_t value){
  switchBank(0x0C);
  writeRegister(Cmd_thd,value);
}

uint8_t PAJ7025R3::getPixelNoiseTreshold(){
  switchBank(0x00);
  return readRegister(Cmd_nthd);
}

void PAJ7025R3::setPixelNoiseTreshold(uint8_t value){
  switchBank(0x00);
  writeRegister(Cmd_nthd,value);
}

uint16_t PAJ7025R3::getMaxAreaThreshold(){
  switchBank(0x00);
  return readRegister(Cmd_oahb, 2);
}

void PAJ7025R3::setMaxAreaThreshold(uint16_t val){
  switchBank(0x00);
  writeRegister(Cmd_oahb,val, 2);
}

uint8_t PAJ7025R3::getMinAreaThreshold(){
  switchBank(0x0C);
  return readRegister(Cmd_oalb);
}

void PAJ7025R3::setMinAreaThreshold(uint8_t val){
  switchBank(0x0C);
  writeRegister(Cmd_oalb,val);
}

uint16_t PAJ7025R3::getXResolutionScale(){
  switchBank(0x0C);
  return readRegister(Cmd_scale_resolution_x, 2);
}

uint16_t PAJ7025R3::getYResolutionScale(){
  switchBank(0x0C);
  return readRegister(Cmd_scale_resolution_y, 2);
}

void PAJ7025R3::setResolutionScale(uint16_t x, uint16_t y){
  if (x>4095) x = 4095;
  if (y>4095) y = 4095;
  switchBank(0x0C);
  writeRegister(Cmd_scale_resolution_x, x, 2);
  writeRegister(Cmd_scale_resolution_y, y, 2);
}

bool PAJ7025R3::getObjectLabelingMode(){
  switchBank(0x00);
  return readRegister(Cmd_dsp_operation_mode);
}

void PAJ7025R3::setObjectLabelingMode(bool val){
  switchBank(0x00);
  writeRegister(Cmd_dsp_operation_mode,val);
}

uint8_t PAJ7025R3::getObjectNumberSetting(){
  switchBank(0x00);
  objectNumber = readRegister(Cmd_max_objects_num);
  return objectNumber;
}

void PAJ7025R3::setObjectNumberSetting(uint8_t val){
  if (val > 16) val = 16;
  switchBank(0x00);
  writeRegister(Cmd_max_objects_num,val);
  objectNumber = val;
}

uint8_t PAJ7025R3::getBarOrientationRatio(){
  switchBank(0x00);
  return readRegister(Cmd_orientation_ratio);
}

void PAJ7025R3::setBarOrientationRatio(uint8_t val){
  switchBank(0x00);
  writeRegister(Cmd_orientation_ratio,val);
}

bool PAJ7025R3::getVsync(){
  switchBank(0x0C);
  uint8_t Vsync = readRegister(Cmd_IOMode_GPIO_08);
  if (Vsync == 0x0B) return true;
  else return false;
}

void PAJ7025R3::setVsync(bool enable){
  switchBank(0x0C);
  if (enable) writeRegister(Cmd_IOMode_GPIO_08,0x0B);
  else writeRegister(Cmd_IOMode_GPIO_08,0x02);
}

bool PAJ7025R3::getExposureSignal(){
  return false;
}

void PAJ7025R3::setExposureSignal(bool enable){
  switchBank(0x0C);
  if (enable) writeRegister(Cmd_IOMode_GPIO_13,0x08); //assign LED_SIDE to G13
  else writeRegister(Cmd_IOMode_GPIO_13,0x00);
  switchBank(0x00);
  if (enable) writeRegister(Cmd_OtherGPO_13,0x2C); //G13 output Exposure Signal
  else writeRegister(Cmd_OtherGPO_13,0xDC);
  writeRegister(Bank0_Sync_Updated_Flag,0x01); //Bank0 sync update flag
}

bool PAJ7025R3::checkProductId(){
  switchBank(0x00); //switch to register bank 0x00
  //if (readRegister(Product_ID, 2) == 0x7025) return true;
  //else return false;
  uint8_t val1 = readRegister(0x03);
  uint8_t val2 = readRegister(0x02);
  if (val1 == 0x70 && val2 == 0x25) return true;
  else return false;
}

bool PAJ7025R3::getOutput(uint8_t format){
 // unsigned long timer = micros();
  if (format > 4) format = 4;
  outputFormat = format;

  uint16_t num,bank;
  
  if (format == 1){
    bank = 0x05;
    num = 256;
  }
  else if (format == 2){
    bank = 0x09;
    num = 96;
   
  }
  else if (format == 3){
    bank = 0x0A;
    num = 144;
    
  }
  else if (format == 4){
    bank = 0x0B;
    num = 208;

  }
  else return false;

  switchBank(bank);
  
  digitalWrite(_cs,LOW);
  SPI.transfer(0x81);
  SPI.transfer(0x00);
  for (int i=0; i<256; i++){
    if (i<num) outputBuffer[i] = SPI.transfer(0x00);
    else outputBuffer[i] = 0;
  }
  digitalWrite(_cs,HIGH);
  
  detectedPoints = 0;
  _avgCounter++;
  bool newPoints = false;

  for (int i=0; i<objectNumber; i++){
    getMeasuredPoint(i);
  }

  if (_avgCounter >= _avgCount) newPoints = true;

  if (detectedPoints == 0 || _avgCounter >= _avgCount) {
    _avgCounter = 0;
  }

  return newPoints;
}

void PAJ7025R3::getMeasuredPoint(uint8_t point){
  uint16_t area,xCoord,yCoord;
  uint8_t avgBrightness,maxBrightness,range,radius,boundaryLeft,boundaryRight,boundaryUp,boundaryDown,aspectRatio,Vx,Vy;
  
  if (outputFormat == 1){
    area = outputBuffer[point*16] | outputBuffer[point*16+1]<<8;
    xCoord = outputBuffer[point*16+2] | outputBuffer[point*16+3]<<8;
    yCoord = outputBuffer[point*16+4] | outputBuffer[point*16+5]<<8;
    avgBrightness = outputBuffer[point*16+6];
    maxBrightness = outputBuffer[point*16+7];
    range = outputBuffer[point*16+8]>>4;
    radius = outputBuffer[point*16+8]&0x0F;
    boundaryLeft = outputBuffer[point*16+9];
    boundaryRight = outputBuffer[point*16+10];
    boundaryUp = outputBuffer[point*16+11];
    boundaryDown = outputBuffer[point*16+12];
    aspectRatio = outputBuffer[point*16+13];
    Vx = outputBuffer[point*16+14];
    Vy = outputBuffer[point*16+15];
  }
  else if (outputFormat == 2){
    area = outputBuffer[point*6] | outputBuffer[point*6+1]<<8;
    xCoord = outputBuffer[point*6+2] | outputBuffer[point*6+3]<<8;
    yCoord = outputBuffer[point*6+4] | outputBuffer[point*6+5]<<8;
  }
  else if (outputFormat == 3){
    area = outputBuffer[point*9] | outputBuffer[point*9+1]<<8;
    xCoord = outputBuffer[point*9+2] | outputBuffer[point*9+3]<<8;
    yCoord = outputBuffer[point*9+4] | outputBuffer[point*9+5]<<8;
    avgBrightness = outputBuffer[point*9+6];
    maxBrightness = outputBuffer[point*9+7];
    range = outputBuffer[point*9+8]>>4;
    radius = outputBuffer[point*9+8]&0x0F;
  }
  else if (outputFormat == 4){
    area = outputBuffer[point*13] | outputBuffer[point*13+1]<<8;
    xCoord = outputBuffer[point*13+2] | outputBuffer[point*13+3]<<8;
    yCoord = outputBuffer[point*13+4] | outputBuffer[point*13+5]<<8;
    boundaryLeft = outputBuffer[point*13+6];
    boundaryRight = outputBuffer[point*13+7];
    boundaryUp = outputBuffer[point*13+8];
    boundaryDown = outputBuffer[point*13+9];
    aspectRatio = outputBuffer[point*13+10];
    Vx = outputBuffer[point*13+11];
    Vy = outputBuffer[point*13+12];
  }

  //point not detected
  if (maxBrightness == 0) {
    if (irPoints[point].invalidCount < 255) irPoints[point].invalidCount++;
    if (irPoints[point].invalidCount > 5) {
      irPoints[point].valid = false;
      irPoints[point].x = 0;
      irPoints[point].y = 0;
      irPoints[point].avgBrightness = 0;
      irPoints[point].maxBrightness = 0;
      return;
    }
  }
  else {
    irPoints[point].valid = true;
    irPoints[point].invalidCount = 0;

    _avgPoints[point].x += xCoord;
    _avgPoints[point].y += yCoord;
    _avgPoints[point].avgBrightness += avgBrightness;
    _avgPoints[point].maxBrightness += maxBrightness;

    if (_avgCounter >= _avgCount) {
      double x = _avgPoints[point].x/_avgCounter;
      double y = _avgPoints[point].y/_avgCounter;

      //If calibration is enabled, perform homography transform
      if (_calEn) {
        cal.calculateCoordinates(x, y);
        x = cal.getX();
        y = cal.getY();

        //If calibration offset is enabled, perform homography transform
        if (_calOffsetEn) {
          offset.calculateCoordinates(x, y);
          x = offset.getX();
          y = offset.getY();
        }
      }

      //If rotation is enabled, flip x and y
      if (_rotation) {
        double xTemp = x;
        x = y;
        y = xTemp;
      }

      //If mirrorX is enabled
      if (_mirrorX) x = 4095 - x;

      //If mirrorY is enabled
      if (_mirrorY) y = 4095 - y;

      //Add offset
      x += _offsetX;
      y += _offsetY;

      //Scale coordinates
      x = (x-2048)*_scaleX+2048;
      y = (y-2048)*_scaleY+2048;

      //Store the values
      irPoints[point].x = x;
      irPoints[point].y = y;
      irPoints[point].avgBrightness = _avgPoints[point].avgBrightness/_avgCounter;
      irPoints[point].maxBrightness = _avgPoints[point].maxBrightness/_avgCounter;

      //Reset _avgPoints
      _avgPoints[point].x = 0;
      _avgPoints[point].y = 0;
      _avgPoints[point].avgBrightness = 0;
      _avgPoints[point].maxBrightness = 0;
    }
  }
  
  detectedPoints++;
 
  //Store all the values
  irPoints[point].xRaw = xCoord;
  irPoints[point].yRaw = yCoord;
  irPoints[point].avgBrightnessRaw = avgBrightness;
  irPoints[point].maxBrightnessRaw = maxBrightness;
  irPoints[point].area = area;
  irPoints[point].range = range;
  irPoints[point].radius = radius;
  irPoints[point].boundaryLeft = boundaryLeft;
  irPoints[point].boundaryRight = boundaryRight;
  irPoints[point].boundaryUp = boundaryUp;
  irPoints[point].boundaryDown = boundaryDown;
  irPoints[point].aspectRatio = aspectRatio;
  irPoints[point].Vx = Vx;
  irPoints[point].Vy = Vy;
}




void PAJ7025R3::setAverageCount(uint8_t averageCount) {
  if (averageCount < 1) averageCount = 1;
  _avgCount = averageCount;
}

uint8_t PAJ7025R3::getAverageCount() {
  return _avgCount;
}

void PAJ7025R3::setMirrorX(bool en){
  _mirrorX = en;
}
bool PAJ7025R3::getMirrorX(){
  return _mirrorX;
}
void PAJ7025R3::setMirrorY(bool en){
  _mirrorY = en;
}
bool PAJ7025R3::getMirrorY(){
  return _mirrorY;
}
void PAJ7025R3::setRotation(bool en){
  _rotation = en;
}
bool PAJ7025R3::getRotation(){
  return _rotation;
}
void PAJ7025R3::setOffset(int16_t x, int16_t y){
  _offsetX = x;
  _offsetY = y;
}
void PAJ7025R3::setOffsetX(int16_t x){
  _offsetX = x;
}
void PAJ7025R3::setOffsetY(int16_t y){
  _offsetY = y;
}
int16_t PAJ7025R3::getOffsetX(){
  return _offsetX;
}
int16_t PAJ7025R3::getOffsetY(){
  return _offsetY;
}

void PAJ7025R3::setScale(float scaleX, float scaleY) {
  _scaleX = scaleX;
  _scaleY = scaleY;
}
void PAJ7025R3::setScaleX(float scale) {
  _scaleX = scale;
}
void PAJ7025R3::setScaleY(float scale) {
  _scaleY = scale;
}
float PAJ7025R3::getScaleX() {
  return _scaleX;
}
float PAJ7025R3::getScaleY() {
  return _scaleY;
}





void PAJ7025R3::setCalibrationEnable(bool en){
  _calEn = en;
}
bool PAJ7025R3::getCalibrationEnable(){
  return _calEn;
}
int PAJ7025R3::getCalibrationArray(int point, int axis) {
  return cal.getCalArray(point, axis);
}
void PAJ7025R3::setCalibrationArray(int point,int axis, int value) {
  cal.setCalArray(point, axis, value);
}

void PAJ7025R3::orderCalibrationArray() {
  cal.orderCalArray();
}

void PAJ7025R3::calculateHomographyMatrix() {
  cal.calculateHomographyMatrix();
}



void PAJ7025R3::setCalibrationOffsetEnable(bool en){
  _calOffsetEn = en;
}
bool PAJ7025R3::getCalibrationOffsetEnable(){
  return _calOffsetEn;
}
int PAJ7025R3::getCalibrationOffsetArray(int point, int axis) {
  return offset.getCalArray(point, axis);
}
void PAJ7025R3::setCalibrationOffsetArray(int point,int axis, int value) {
  offset.setCalArray(point, axis, value);
}

void PAJ7025R3::orderCalibrationOffsetArray() {
  offset.orderCalArray();
}

void PAJ7025R3::calculateOffsetHomographyMatrix() {
  offset.calculateHomographyMatrix();
}