#include "Arduino.h"

#ifndef MC3419_H
#define MC3419_H

/********************************************************************
 * Register Adresses
 ********************************************************************/
#define STATUS_REG      0x05
#define INTR_CTRL_REG   0x06
#define MODE_REG        0x07
#define RATE_REG        0x08
#define MOTION_CTRL_REG 0x09
#define STATUS_REG      0x13
#define INTR_STAT_REG   0x14

#define RANGE_REG       0x20
#define COMM_CTRL_REG   0x31
#define GPIO_CTRL_REG   0x33
#define TF_THRESH_REG   0x40
#define TF_DEBOUNCE_REG 0x42
#define TIMER_CTRL_REG  0x4A

#define XOUT            0x0D
#define YOUT            0x0F
#define ZOUT            0x11

/********************************************************************
 * Defaults
 ********************************************************************/
#define MODE_STANDBY    0b00
#define MODE_WAKE       0b01

#define SPI_DELAY       10

class MC3419
{
  public:
    MC3419(uint8_t data, uint8_t sck, uint8_t cs);
    void setDeviceMode(uint8_t mode);
    void setInterrupt(uint8_t data);
    void setSampleRate(uint8_t sampleRate);
    void setMotionControl(uint8_t data);
    void resetMotionControl();
    void setRange(uint8_t data);
    void setComControl(uint8_t data);
    void setGPIOControl(uint8_t data);
    void setTiltThreshold(uint16_t data);
    void setTiltDebounce(uint8_t data);
    void clearInterrupts();
    void writeRegister(uint8_t address, uint8_t data);
  private:
    uint8_t _data;
    uint8_t _sck;
    uint8_t _cs;
    //uint16_t readRegister(uint8_t address);
};

#endif
