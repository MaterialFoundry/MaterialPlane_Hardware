/*
 * Copyright (c) 2021 by Cristian Deenen <cdeenen@outlook.com>
 * MEMSIC MC3419 accelerometer library designed for Material Plane: https://github.com/cdeenen/materialplane
 *
 * Personal use of this library is allowed.
 * Commercial use using official Material Plane hardware is allowed.
 * For redistribution or other commercial use, contact the copyright holder.
 */
 
#include "MC3419.h"
#include "Arduino.h"
#include <SPI.h>

MC3419::MC3419(uint8_t cs) {
  _cs = cs;

  //Set cs pin to output
  pinMode(_cs,OUTPUT);

  //Set pins low
  digitalWrite(_cs,LOW);
}

void MC3419::initialize() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE3));
}

void MC3419::setDeviceMode(uint8_t mode) {
  writeRegister(MODE_REG, mode);
}

void MC3419::setSampleRate(uint8_t sampleRate) {
  writeRegister(RATE_REG, sampleRate);
}

void MC3419::setRange(uint8_t data) {
  writeRegister(RANGE_REG, data);
}

void MC3419::setComControl(uint8_t data) {
  writeRegister(COMM_CTRL_REG, data);
}

void MC3419::setGPIOControl(uint8_t data) {
  writeRegister(GPIO_CTRL_REG, data);
}

void MC3419::setMotionControl(uint8_t data) {
  _motionCtrlReg = data;
  writeRegister(MOTION_CTRL_REG, data);
}

void MC3419::resetMotionControl() {
  setDeviceMode(MODE_STANDBY);
  writeRegister(MOTION_CTRL_REG, _motionCtrlReg | 1<<7);
  writeRegister(MOTION_CTRL_REG, _motionCtrlReg);
  setDeviceMode(MODE_WAKE);
}

uint8_t MC3419::getStatus() {
  return readRegister(STATUS_REG);
}

void MC3419::setInterrupt(uint8_t data) {
  writeRegister(INTR_CTRL_REG, data);
}

uint8_t MC3419::getInterruptStatus() {
  return readRegister(INTR_STAT_REG);
}

void MC3419::clearInterrupts() {
  writeRegister(INTR_STAT_REG, 0);
}

int16_t MC3419::getX() {
  return (int16_t)readRegister(XOUT_REG, 2);
}

int16_t MC3419::getY() {
  return (int16_t)readRegister(YOUT_REG, 2);
}

int16_t MC3419::getZ() {
  return (int16_t)readRegister(ZOUT_REG, 2);
}

void MC3419::setTiltThreshold(uint16_t threshold) {
  writeRegister(TF_THRESH_REG, threshold, 2);
}

void MC3419::setTiltDebounce(uint8_t duration) {
  writeRegister(TF_DEBOUNCE_REG, duration);
}

/*
void MC3419::setAnymotionThreshold(uint16_t data) {
  writeRegister(AM_THRESH_REG, data, 2);
}

void MC3419::setAnymotionDebounce(uint8_t data) {
  writeRegister(AM_DB_REG, data);
}

void MC3419::setShakeThreshold(uint16_t data) {
  writeRegister(SHK_THRESH_REG, data, 2);
}

void MC3419::setShakePeakToPeakDuration(uint16_t data) {
  if (data > 4095) return;
  _shakePeakToPeakDuration = data;
  uint16_t d = _shakePeakToPeakDuration | _shakeDuration<<12;
  writeRegister(PK_P2P_DUR_THRES_REG, data, 2);
}

void MC3419::setShakeDuration(uint8_t data) {
  if (data > 7) return;
  _shakeDuration = data;
  setShakePeakToPeakDuration(_shakePeakToPeakDuration);
}
*/

void MC3419::writeRegister(uint8_t address, uint16_t data, uint8_t bytesToSend) {
  digitalWrite(_cs,LOW);                    //CS LOW
  SPI.transfer(address);
  for (int i=0; i<bytesToSend; i++) 
    SPI.transfer((data>>(i*8))&255);
  digitalWrite(_cs,HIGH);                     //CS HIGH
  delay(SPI_DELAY);
}

uint16_t MC3419::readRegister(uint8_t address, uint8_t bytesToRead) {
  uint16_t data = 0;
  digitalWrite(_cs, LOW);                    //CS LOW
  SPI.transfer(address | 0x80);
  SPI.transfer(0);
  for (int i=0; i<bytesToRead; i++) 
    data |= SPI.transfer(0) << (i*8);
  digitalWrite(_cs, HIGH);                     //CS HIGH
  return data;
}
