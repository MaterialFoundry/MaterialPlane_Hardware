#include "wiiCam.h"
#include "Arduino.h"
#include <Wire.h>
#include "../Homography/homography.h"

homography cal2;
homography offset2;

//I2C address
#define IR_ADDRESS 0x58

//I2C message size
#define MSGSIZE 40

wiiCam::wiiCam(uint8_t sda, uint8_t scl){
  _sda = sda;
  _scl = scl;
  Wire.begin(_sda,_scl,400000);
  for (int i=0; i<9; i++) sensitivityBlock1[i] = 0;
  for (int i=0; i<2; i++) sensitivityBlock2[i] = 0;
}

void wiiCam::initialize(){
  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(0x30); 
  Wire.write(0x01);
  Wire.endTransmission();
  delay(10);

  setSensitivity(4);
  
  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(0x33); 
  Wire.write(0x05);
  Wire.endTransmission();
  delay(10);
}


uint8_t wiiCam::readRegister(uint8_t reg){
  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(IR_ADDRESS, 1);
  return Wire.read();
}

/*
 * Write to the sensor register
 */
void wiiCam::writeRegister(uint8_t reg,uint8_t val){
  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(reg); 
  Wire.write(val);
  Wire.endTransmission();
}

void wiiCam::setSensitivity(uint8_t val) {
  uint8_t sens = (uint8_t)val;
  uint8_t p0, p1, p2, p3;

  if (sens == 0) {
    p0 = 0x64;
    p1 = 0xFE;
    p2 = 0xFD;
    p3 = 0x05;
  }
  else if (sens == 1) {
    p0 = 0x96;
    p1 = 0xB4;
    p2 = 0xB3;
    p3 = 0x04;
  }
  else if (sens == 2) {
    p0 = 0xAA;
    p1 = 0x64;
    p2 = 0x63;
    p3 = 0x03;
  }
  else if (sens == 3) {
    p0 = 0xC8;
    p1 = 0x36;
    p2 = 0x35;
    p3 = 0x03;
  }
  else if (sens == 4) {
    p0 = 0x72;
    p1 = 0x20;
    p2 = 0x1F;
    p3 = 0x03;
  }
  p0 = 0xFF;
  
  sensitivityBlock1[0] = 0x02;
  sensitivityBlock1[1] = 0x00;
  sensitivityBlock1[2] = 0x00;
  sensitivityBlock1[3] = 0x71;
  sensitivityBlock1[4] = 0x01;
  sensitivityBlock1[5] = 0x00;
  sensitivityBlock1[6] = p0;  //max intensity threshold
  sensitivityBlock1[7] = 0x00;  
  sensitivityBlock1[8] = p1;  //brightness
  
  sensitivityBlock2[0] = p2;  //max brightness        REG 0x1A
  sensitivityBlock2[1] = p3; //min intensity threshold  REG 0x1B

  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(0x00); 
  for (int i=0; i<9; i++) Wire.write(sensitivityBlock1[i]);
  Wire.endTransmission();
  delay(10);
  
  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(0x1A); Wire.write(sensitivityBlock2[0]); Wire.write(sensitivityBlock2[1]);
  Wire.endTransmission();
  
  delay(10);
  
  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(0x30); Wire.write(0x08);
  Wire.endTransmission();
  delay(10);
}

float wiiCam::getGain(){
  uint8_t reg = readRegister(0x08);
  float gain = 0.01*round((255 - readRegister(0x08))/0.3657143) + 1;
  return gain;
}

void wiiCam::setGain(float gainTemp){
  uint8_t val = round(255-(gainTemp-1)*36.57143);
  if ((uint8_t)val == 1) val = 0;
  writeRegister(0x08,(uint8_t)val);
  writeRegister(0x1A,(uint8_t)val-1);
}

uint8_t wiiCam::getPixelBrightnessThreshold(){
  return readRegister(0x1B);
}

void wiiCam::setPixelBrightnessThreshold(uint8_t value){
  writeRegister(0x1B,value);
}

bool wiiCam::getOutput(uint8_t format){
 
  /**
   * Set format
   * Format 1: 
   * Addr: 0x36
   * 10 bytes
   * X: 0-1023
   * Y: 0-767
   * 
   * Format 2: 
   * Addr: 0x33
   * 12 bytes
   * X: 0-1023
   * Y: 0-767
   * Area: 4 bits
   * 
   * Format 3: 
   * Addr: 0x36
   * 18 bytes
   * X: 0-1023
   * Y: 0-767
   * Area: 3 bits
   * Xmin: 7 bits
   * Xmax: 7 bits
   * Ymin: 7 bits
   * Ymax: 7 bits
   * Intensity: 8 bits
   */

  //IR sensor read
  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(0x36);
  Wire.endTransmission();

  // Request the 2 byte heading (MSB comes first)
  Wire.requestFrom(IR_ADDRESS, MSGSIZE);

  int i=0;
  while(Wire.available() && i < MSGSIZE) {
    outputBuffer[i] = Wire.read();
    i++;
  }
  
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

void wiiCam::getMeasuredPoint(uint8_t point){
  uint16_t xCoord,yCoord;
  uint8_t area,maxBrightness, avgBrightness;

  xCoord = ((outputBuffer[3+point*9]>>4)&3) << 8 | outputBuffer[1+point*9]; 
  yCoord = ((outputBuffer[3+point*9]>>6)&3) << 8 | outputBuffer[2+point*9];
  area = outputBuffer[3+point*9]&&15;
  maxBrightness = outputBuffer[9+point*9];
  avgBrightness = maxBrightness;

  xCoord *= 4;
  yCoord *= 5.33;
  
  //point not detected
  if (maxBrightness == 255) {
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
        cal2.calculateCoordinates(x, y);
        x = cal2.getX();
        y = cal2.getY();

        //If calibration offset is enabled, perform homography transform
        if (_calOffsetEn) {
          offset2.calculateCoordinates(x, y);
          x = offset2.getX();
          y = offset2.getY();
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
  //irPoints[point].range = range;
  //irPoints[point].radius = radius;
  //irPoints[point].boundaryLeft = boundaryLeft;
  //irPoints[point].boundaryRight = boundaryRight;
  //irPoints[point].boundaryUp = boundaryUp;
  //irPoints[point].boundaryDown = boundaryDown;
  //irPoints[point].aspectRatio = aspectRatio;
  //irPoints[point].Vx = Vx;
  //irPoints[point].Vy = Vy;
}

void wiiCam::setFramePeriod(float fPeriod) {
  if (fPeriod < 15) fPeriod = 15;
  framePeriod = fPeriod;
}

float wiiCam::getFramePeriod() {
  return framePeriod;
}

uint8_t wiiCam::getObjectNumberSetting() {
  return objectNumber;
}

void wiiCam::setObjectNumberSetting(uint8_t val) {
  if (val > 4) val = 4;
  objectNumber = val;
}




void wiiCam::setAverageCount(uint8_t averageCount) {
  if (averageCount < 1) averageCount = 1;
  _avgCount = averageCount;
}

uint8_t wiiCam::getAverageCount() {
  return _avgCount;
}

void wiiCam::setMirrorX(bool en){
  _mirrorX = en;
}
bool wiiCam::getMirrorX(){
  return _mirrorX;
}
void wiiCam::setMirrorY(bool en){
  _mirrorY = en;
}
bool wiiCam::getMirrorY(){
  return _mirrorY;
}
void wiiCam::setRotation(bool en){
  _rotation = en;
}
bool wiiCam::getRotation(){
  return _rotation;
}
void wiiCam::setOffset(int16_t x, int16_t y){
  _offsetX = x;
  _offsetY = y;
}
void wiiCam::setOffsetX(int16_t x){
  _offsetX = x;
}
void wiiCam::setOffsetY(int16_t y){
  _offsetY = y;
}
int16_t wiiCam::getOffsetX(){
  return _offsetX;
}
int16_t wiiCam::getOffsetY(){
  return _offsetY;
}

void wiiCam::setScale(float scaleX, float scaleY) {
  _scaleX = scaleX;
  _scaleY = scaleY;
}
void wiiCam::setScaleX(float scale) {
  _scaleX = scale;
}
void wiiCam::setScaleY(float scale) {
  _scaleY = scale;
}
float wiiCam::getScaleX() {
  return _scaleX;
}
float wiiCam::getScaleY() {
  return _scaleY;
}





void wiiCam::setCalibrationEnable(bool en){
  _calEn = en;
}
bool wiiCam::getCalibrationEnable(){
  return _calEn;
}
int wiiCam::getCalibrationArray(int point, int axis) {
  return cal2.getCalArray(point, axis);
}
void wiiCam::setCalibrationArray(int point,int axis, int value) {
  cal2.setCalArray(point, axis, value);
}

void wiiCam::orderCalibrationArray() {
  cal2.orderCalArray();
}

void wiiCam::calculateHomographyMatrix() {
  cal2.calculateHomographyMatrix();
}



void wiiCam::setCalibrationOffsetEnable(bool en){
  _calOffsetEn = en;
}
bool wiiCam::getCalibrationOffsetEnable(){
  return _calOffsetEn;
}
int wiiCam::getCalibrationOffsetArray(int point, int axis) {
  return offset2.getCalArray(point, axis);
}
void wiiCam::setCalibrationOffsetArray(int point,int axis, int value) {
  offset2.setCalArray(point, axis, value);
}

void wiiCam::orderCalibrationOffsetArray() {
  offset2.orderCalArray();
}

void wiiCam::calculateOffsetHomographyMatrix() {
  offset2.calculateHomographyMatrix();
}