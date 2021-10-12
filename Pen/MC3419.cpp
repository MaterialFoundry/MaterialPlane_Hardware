#include "MC3419.h"
#include "definitions.h"
#include "userSettings.h"
#include "Arduino.h"

MC3419::MC3419(uint8_t data, uint8_t sck, uint8_t cs) {
 
  _data = data;
  _sck = sck;
  _cs = CS;

  //Set pins to output
  pinMode(_data,OUTPUT);
  pinMode(_sck,OUTPUT);
  pinMode(_cs,OUTPUT);

  //Set pins low
  digitalWrite(_data,LOW);
  digitalWrite(_sck,LOW);
  digitalWrite(_cs,LOW);
}
void MC3419::writeRegister(uint8_t address, uint8_t data) {
  uint16_t fullData = address<<8 | data;
  
  digitalWrite(_cs,LOW);                    //CS LOW
  for (int i = 15; i >= 0; i--) {
    digitalWrite(_sck,LOW);                  //SCK LOW

    digitalWrite(_data,(fullData >> i) & 1);  //set MOSI
  
    digitalWrite(_sck,HIGH);                  //SCK HIGH
  }
  digitalWrite(_cs,HIGH);                     //CS HIGH
}

void MC3419::setDeviceMode(uint8_t mode) {
  writeRegister(MODE_REG, mode);
}

void MC3419::setInterrupt(uint8_t data) {
  writeRegister(INTR_CTRL_REG, data);
}

void MC3419::setSampleRate(uint8_t sampleRate) {
  writeRegister(RATE_REG, sampleRate);
}

void MC3419::setMotionControl(uint8_t data) {
  writeRegister(MOTION_CTRL_REG, data);
}

void MC3419::resetMotionControl() {
  setDeviceMode(MODE_STANDBY);
  //delay(10);
  setMotionControl(0b00000001 | 1<<7);
  //delay(10);
  setMotionControl(0b00000001);
  //delay(10);
  setDeviceMode(MODE_WAKE);
}

void MC3419::setRange(uint8_t data) {
  setDeviceMode(MODE_STANDBY);
  delay(SPI_DELAY);
  writeRegister(RANGE_REG, data);
  delay(SPI_DELAY);
  setDeviceMode(MODE_WAKE);
}

void MC3419::setComControl(uint8_t data) {
  writeRegister(COMM_CTRL_REG, data);
}

void MC3419::setGPIOControl(uint8_t data) {
  writeRegister(GPIO_CTRL_REG, data);
}

void MC3419::setTiltThreshold(uint16_t data) {
  writeRegister(TF_THRESH_REG, data&255);
  writeRegister(TF_THRESH_REG+1, (data>>8)&255);
}

void MC3419::setTiltDebounce(uint8_t data) {
  writeRegister(TF_DEBOUNCE_REG, data);
}

void MC3419::clearInterrupts() {
  writeRegister(INTR_STAT_REG, 0);
}
