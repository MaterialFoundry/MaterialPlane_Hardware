#include "PAJ7025R3.h"
#include "Arduino.h"
#include "../../configuration.h"
#include "../../userSettings.h"
#include <SPI.h>

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
  //setFramePeriod(FRAME_PERIOD);
  //setExposureTime(EXPOSURE_TIME);
  //setGain(GAIN);
  //setPixelBrightnessThreshold(BRIGHTNESS_TRESHOLD);
  //setPixelNoiseTreshold(NOISE_TRESHOLD);
  //setMinAreaThreshold(MIN_AREA_TRESHOLD);
  //setMaxAreaThreshold(MAX_AREA_TRESHOLD);
  setResolutionScale(4095,4095);
  setObjectLabelingMode(1);
  //setObjectNumberSetting(16);
  setBarOrientationRatio(3);
  setExposureSignal(true);
}

uint8_t PAJ7025R3::readRegister(uint8_t reg){
  uint8_t val;
  digitalWrite(_cs,LOW);
  SPI.transfer(0x80);
  SPI.transfer(reg);
  val = SPI.transfer(0x00);
  digitalWrite(_cs,HIGH);
  return val;
}

/*
 * Write to the sensor register
 */
void PAJ7025R3::writeRegister(uint8_t reg,uint8_t val){
  digitalWrite(_cs,LOW);
  SPI.transfer(0x00);
  SPI.transfer(reg);
  SPI.transfer(val);
  digitalWrite(_cs,HIGH);
}

void PAJ7025R3::switchBank(uint8_t bank){
  writeRegister(0xEF,bank); //switch to register bank
}

/*
 * Set the power state of the sensor
 */
void PAJ7025R3::powerOn(bool en){
  writeRegister(0xEF,0x00); //switch to register bank 0x00
  if (en)
    writeRegister(0x2F,0x05); //cmd_manual_powercontrol_power && cmd_manual_powercontrol_sensor On = 1
  else 
    writeRegister(0x2F,0x00); //cmd_manual_powercontrol_power && cmd_manual_powercontrol_sensor On = 0
  writeRegister(0x30,0x00); //make the cmd_manual_powercontrol setting effective
  writeRegister(0x30,0x01); //make the cmd_manual_powercontrol setting effective
}

/*
 * Reset the sensor
 */
void PAJ7025R3::resetSensor(){
  switchBank(0x00);
  writeRegister(0x64,0x00); //cmd_global_resetn
}

bool PAJ7025R3::getFrameSubstration(){
  switchBank(0x00);
  return readRegister(0x28);
}

void PAJ7025R3::setFrameSubstraction(bool val){
  switchBank(0x00);
  writeRegister(0x28,val);
}

/*
 * Get the frame period of the sensor
 */
float PAJ7025R3::getFramePeriod(){
  switchBank(0x0C);
  framePeriod = readRegister(0x07) | readRegister(0x08)<<8 | readRegister(0x09)<<16;
  return 0.0001*framePeriod;
}

void PAJ7025R3::setFramePeriod(float period){
  if (period > 100) period = 100;
  if (period < 5) period = 5;
  framePeriod = 10000*period;
  switchBank(0x0C);
  writeRegister(0x07,framePeriod&0xFF);
  writeRegister(0x08,(framePeriod>>8)&0xFF);
  writeRegister(0x09,(framePeriod>>16)&0xFF);
}

float PAJ7025R3::getExposureTime(){
  switchBank(0x01);
  exposureTime = readRegister(0x0E) | readRegister(0x0F)<<8;
  return exposureTime * 0.0002;
}

void PAJ7025R3::setExposureTime(float val){
  float maxExposureTime = (float)framePeriod*0.0001-2.7;
  if (val > maxExposureTime) val = maxExposureTime;
  if (val > 13) val = 13;
  if (val < 0.02) val = 0.02;
  
  exposureTime = val*5000;

  switchBank(0x0C);
  writeRegister(0x0F,exposureTime&0xFF);
  writeRegister(0x10,(exposureTime>>8)&0xFF);
  switchBank(0x01);
  writeRegister(0x01,0x01);
}

float PAJ7025R3::getGain(){
  switchBank(0x01);
  gain = (1 + 0.0625*readRegister(0x05));
  uint8_t H = readRegister(0x06);
  if (H == 0x02) gain *= 2;
  else if (H == 0x03) gain *= 4;
  return gain;
}

void PAJ7025R3::setGain(float gainTemp){
  if (gainTemp < 1) gainTemp = 1;
  if (gainTemp > 8) gainTemp = 8;
  uint8_t B_global,B_ggh;
  if (gainTemp < 2) {
    B_global = (gainTemp - 1)/0.0625;
    B_ggh = 0x00;
  }
  else if (gainTemp < 4) {
    B_global = (gainTemp - 2)/0.125;
    B_ggh = 0x02;
  }
  else {
    B_global = (gainTemp - 4)/0.25;
    B_ggh = 0x03;
  }
  
  gain = (1 + 0.0625*readRegister(0x05));
  if (B_ggh == 0x02) gain *= 2;
  else if (B_ggh == 0x03) gain *= 4;
 
  switchBank(0x0C);
  writeRegister(0x0B,B_global);
  writeRegister(0x0C,B_ggh);
  switchBank(0x01);
  writeRegister(0x01,0x01);
}

uint8_t PAJ7025R3::getPixelBrightnessThreshold(){
  switchBank(0x0C);
  return readRegister(0x47);
}

void PAJ7025R3::setPixelBrightnessThreshold(uint8_t value){
  switchBank(0x0C);
  writeRegister(0x47,value);
}

uint8_t PAJ7025R3::getPixelNoiseTreshold(){
  switchBank(0x00);
  return readRegister(0x0F);
}

void PAJ7025R3::setPixelNoiseTreshold(uint8_t value){
  switchBank(0x00);
  writeRegister(0x0F,value);
}

uint16_t PAJ7025R3::getMaxAreaThreshold(){
  switchBank(0x00);
  return readRegister(0x0B) | readRegister(0x0C)<<8;
}

void PAJ7025R3::setMaxAreaThreshold(uint16_t val){
  switchBank(0x00);
  writeRegister(0x0B,val&0xFF);
  writeRegister(0x0C,(val>>8)&0xFF);
}

uint8_t PAJ7025R3::getMinAreaThreshold(){
  switchBank(0x0C);
  return readRegister(0x46);
}

void PAJ7025R3::setMinAreaThreshold(uint8_t val){
  switchBank(0x0C);
  writeRegister(0x46,val);
}

uint16_t PAJ7025R3::getXResolutionScale(){
  switchBank(0x0C);
  return readRegister(0x60) | readRegister(0x61)<<8;
}

uint16_t PAJ7025R3::getYResolutionScale(){
  switchBank(0x0C);
  return readRegister(0x62) | readRegister(0x63)<<8;
}

void PAJ7025R3::setResolutionScale(uint16_t x, uint16_t y){
  if (x>4095) x = 4095;
  if (y>4095) y = 4095;
  switchBank(0x0C);
  writeRegister(0x60,x&0xFF);
  writeRegister(0x61,(x>>8)&0xFF);
  writeRegister(0x62,y&0xFF);
  writeRegister(0x63,(y>>8)&0xFF);
}

bool PAJ7025R3::getObjectLabelingMode(){
  switchBank(0x00);
  return readRegister(0x12);
}

void PAJ7025R3::setObjectLabelingMode(bool val){
  switchBank(0x00);
  writeRegister(0x12,val);
}

uint8_t PAJ7025R3::getObjectNumberSetting(){
  switchBank(0x00);
  objectNumber = readRegister(0x19);
  return objectNumber;
}

void PAJ7025R3::setObjectNumberSetting(uint8_t val){
  if (val > 16) val = 16;
  switchBank(0x00);
  writeRegister(0x19,val);
  objectNumber = val;
}

uint8_t PAJ7025R3::getBarOrientationRatio(){
  switchBank(0x00);
  return readRegister(0x10);
}

void PAJ7025R3::setBarOrientationRatio(uint8_t val){
  switchBank(0x00);
  writeRegister(0x10,val);
}

bool PAJ7025R3::getVsync(){
  switchBank(0x0C);
  uint8_t Vsync = readRegister(0x6C);
  if (Vsync == 0x0B) return true;
  else return false;
}

void PAJ7025R3::setVsync(bool enable){
  switchBank(0x0C);
  if (enable) writeRegister(0x6C,0x0B);
  else writeRegister(0x6C,0x02);
}

bool PAJ7025R3::getExposureSignal(){
  return false;
}

void PAJ7025R3::setExposureSignal(bool enable){
  switchBank(0x0C);
  if (enable) writeRegister(0x71,0x08); //assign LED_SIDE to G13
  else writeRegister(0x71,0x00);
  switchBank(0x00);
  if (enable) writeRegister(0x4F,0x2C); //G13 output Exposure Signal
  else writeRegister(0x4F,0xDC);
  writeRegister(0x01,0x01); //Bank0 sync update flag
}

bool PAJ7025R3::checkProductId(){
  writeRegister(0xEF,0x00); //switch to register bank 0x00
  uint8_t val1 = readRegister(0x03);
  uint8_t val2 = readRegister(0x02);
  if (val1 == 0x70 && val2 == 0x25) return true;
  else return false;
}

void PAJ7025R3::getOutput(uint8_t format){
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
  else return;

  switchBank(bank);
  
  digitalWrite(_cs,LOW);
  SPI.transfer(0x81);
  SPI.transfer(0x00);
  for (int i=0; i<256; i++){
    if (i<num) outputBuffer[i] = SPI.transfer(0x00);
    else outputBuffer[i] = 0;
  }
  digitalWrite(_cs,HIGH);
      
  for (int i=0; i<objectNumber; i++){
    getMeasuredPoint(i);
  }
  //unsigned long elapsedTime = micros()-timer;
  //Serial.println(elapsedTime);
}

void PAJ7025R3::getMeasuredPoint(uint8_t point){
  uint16_t area,xCoord,yCoord;
  uint8_t avgBrightness,maxBrightness,range,radius,boundaryLeft,boundaryRight,boundaryUp,boundaryDown,aspectRatio,Vx,Vy;
  
  area = outputBuffer[point*16] | outputBuffer[point*16+1]<<8;
  //Serial.println(area);
  xCoord = outputBuffer[point*16+2] | outputBuffer[point*16+3]<<8;
  yCoord = outputBuffer[point*16+4] | outputBuffer[point*16+5]<<8;
  if (outputFormat == 1 || outputFormat == 3){
    avgBrightness = outputBuffer[point*16+6];
    maxBrightness = outputBuffer[point*16+7];
    range = outputBuffer[point*16+8]>>4;
    radius = outputBuffer[point*16+8]&0x0F;
    
  }
  if (outputFormat == 1){
    boundaryLeft = outputBuffer[point*16+9];
    boundaryRight = outputBuffer[point*16+10];
    boundaryUp = outputBuffer[point*16+11];
    boundaryDown = outputBuffer[point*16+12];
    aspectRatio = outputBuffer[point*16+13];
    Vx = outputBuffer[point*16+14];
    Vy = outputBuffer[point*16+15];
    //Serial.println(Vx);
  }
  else if (outputFormat == 4){
    boundaryLeft = outputBuffer[point*16+6];
    boundaryRight = outputBuffer[point*16+7];
    boundaryUp = outputBuffer[point*16+8];
    boundaryDown = outputBuffer[point*16+9];
    aspectRatio = outputBuffer[point*16+10];
    Vx = outputBuffer[point*16+11];
    Vy = outputBuffer[point*16+12];
  }
  pointStorage[point][0]= area;
  pointStorage[point][1]= xCoord;
  pointStorage[point][2]= yCoord;
  pointStorage[point][3]= avgBrightness;
  pointStorage[point][4]= maxBrightness;
  pointStorage[point][5]= range;
  pointStorage[point][6]= radius;
  pointStorage[point][7]= boundaryLeft;
  pointStorage[point][8]= boundaryRight;
  pointStorage[point][9]= boundaryUp;
  pointStorage[point][10]= boundaryDown;
  pointStorage[point][11]= aspectRatio;
  pointStorage[point][12]= Vx;
  pointStorage[point][13]= Vy;
}