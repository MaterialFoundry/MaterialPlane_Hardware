#ifndef PAJ7025R3_H
#define PAJ7025R3_H

#include "Arduino.h"

class PAJ7025R3
{
  public:
    PAJ7025R3(uint8_t);
    void initialize();
    void powerOn(bool en=true);
    void resetSensor();
    bool getFrameSubstration();
    void setFrameSubstraction(bool val);
    float getFramePeriod();
    void setFramePeriod(float period);
    float getExposureTime();
    void setExposureTime(float val);
    float getGain();
    void setGain(float gainTemp);
    uint8_t getPixelBrightnessThreshold();
    void setPixelBrightnessThreshold(uint8_t value);
    uint8_t getPixelNoiseTreshold();
    void setPixelNoiseTreshold(uint8_t value);
    uint16_t getMaxAreaThreshold();
    void setMaxAreaThreshold(uint16_t val);
    uint8_t getMinAreaThreshold();
    void setMinAreaThreshold(uint8_t val);
    uint16_t getXResolutionScale();
    uint16_t getYResolutionScale();
    void setResolutionScale(uint16_t x, uint16_t y);
    bool getObjectLabelingMode();
    void setObjectLabelingMode(bool val);
    uint8_t getObjectNumberSetting();
    void setObjectNumberSetting(uint8_t val);
    uint8_t getBarOrientationRatio();
    void setBarOrientationRatio(uint8_t val);
    bool checkProductId();
    void getOutput(uint8_t format);
    void getMeasuredPoint(uint8_t point);
    bool getVsync();
    void setVsync(bool enable);
    bool getExposureSignal();
    void setExposureSignal(bool enable);
    unsigned long framePeriod;
    unsigned long exposureTime;
    float gain;
    uint8_t outputBuffer[256];
    uint8_t outputFormat;
    uint16_t pointStorage[16][14];
  
  private:
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg,uint8_t val);
    void switchBank(uint8_t bank);
    uint8_t objectNumber;
    uint8_t _cs;
};
#endif /* PAJ7025R3_H */
