#ifndef wiiCam_H
#define wiiCam_H

#include "Arduino.h"

class wiiCam
{
  public:
    wiiCam();
    void initialize();
    void setSensitivity(uint8_t val);
    float getGain();
    void setGain(float gainTemp);
    uint8_t getPixelBrightnessThreshold();
    void setPixelBrightnessThreshold(uint8_t value);
    void getOutput(uint8_t format);
    void getMeasuredPoint(uint8_t point);

    unsigned long framePeriod;
    unsigned long exposureTime;
    float gain;
    uint8_t outputBuffer[256];
    uint8_t outputFormat;
    uint16_t pointStorage[16][14];
    uint8_t sensitivityBlock1[9];
    uint8_t sensitivityBlock2[2];

    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg,uint8_t val);
  
  private:
    uint8_t objectNumer;
    uint8_t _cs;
};
#endif /* wiiCam_H */
