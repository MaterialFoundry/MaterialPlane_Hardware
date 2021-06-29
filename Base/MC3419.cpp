#include "MC3419.h"
#include "definitions.h"
#include "userSettings.h"
#include "Arduino.h"

MC3419::MC3419() {
  //Set CS and SCK pins to output
  PORTA.DIRSET = CS;
  PORTA.DIRSET = SCK;
  PORTA.DIRSET = DATA;

  PORTA.OUT |= CS;
  PORTA.OUT |= SCK;
}
void MC3419::writeRegister(uint8_t address, uint8_t data) {
  uint16_t fullData = address<<8 | data;
  
  PORTA.OUT &= ~CS;                    //CS LOW
  for (int i = 15; i >= 0; i--) {
    PORTA.OUT &= ~SCK;                  //SCK LOW
    if ((fullData >> i) & 1) PORTA.OUT |= DATA; //MOSI HIGH
    else PORTA.OUT &= ~DATA;             //MOSI LOW
  
    PORTA.OUT |= SCK;                   //SCK HIGH
  
  }
  PORTA.OUT |= CS;                     //CS HIGH
}

void MC3419::initialize() {
    setDeviceMode(MODE_STANDBY);   //Set device in standby mode
    delay(SPI_DELAY);
    setComControl(0b00000000);  //Set communication control. Set bit 5 to 1 for 3-wire SPI
    delay(SPI_DELAY);
    setGPIOControl(0b00001100); //Set GPIO control. Set bit 2 to 1 for INT active high, set bit 3 to 1 for INT push-pull
    delay(SPI_DELAY);
    
    setSampleRate(SAMPLE_RATE);         //Set the sample rate
    delay(SPI_DELAY);
    setInterrupt(0b01000001);    //Set the interrupt enable register, enable tilt. Set bit 6 to 1 for autoclear
    delay(SPI_DELAY);

    setTiltThreshold(TILT_THRESHOLD);  //set tilt threshold
    delay(SPI_DELAY);
    setTiltDebounce(TILT_DEBOUNCE);
    delay(SPI_DELAY);
    setMotionControl(0b00000001);  //enable tilt/flip
    delay(SPI_DELAY);
    setRange(0b00000000);
    delay(SPI_DELAY);
    
    
    clearInterrupts();
    delay(SPI_DELAY);
    setDeviceMode(MODE_WAKE);   //Wake up device
     
    resetMotionControl();
    
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
