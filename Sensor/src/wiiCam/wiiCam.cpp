#include "wiiCam.h"
#include "Arduino.h"
#include "../../configuration.h"
#include "../../userSettings.h"
#include <Wire.h>

//I2C address
#define IR_ADDRESS 0x58

//I2C message size
#define MSGSIZE 40

wiiCam::wiiCam(){
  Wire.begin(SDA,SCL,100000);
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

void wiiCam::getOutput(uint8_t format){
 
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
  
  for (int i=0; i<4; i++){
    getMeasuredPoint(i);
  }
}

void wiiCam::getMeasuredPoint(uint8_t point){
  uint16_t xCoord,yCoord;
  uint8_t area,brightness;

  xCoord = ((outputBuffer[3+point*9]>>4)&3) << 8 | outputBuffer[1+point*9]; 
  yCoord = ((outputBuffer[3+point*9]>>6)&3) << 8 | outputBuffer[2+point*9];
  area = outputBuffer[3+point*9]&&15;
  brightness = outputBuffer[9+point*9];

  xCoord *= 4;
  yCoord *= 5.33;
  pointStorage[point][0]= area;
  pointStorage[point][1]= xCoord;
  pointStorage[point][2]= yCoord;
  pointStorage[point][4]= brightness;
}