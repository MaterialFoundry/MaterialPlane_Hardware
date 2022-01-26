#ifndef wiiCam_H
#define wiiCam_H

#include "Arduino.h"

/**
 * IR point data structure
 */
struct irPoint {
  bool valid;
  uint8_t invalidCount;
  double x;
  uint16_t xRaw;
  double y;
  uint16_t yRaw;
  float avgBrightness;
  uint8_t avgBrightnessRaw;
  float maxBrightness;
  uint8_t maxBrightnessRaw;
  uint16_t area;
  uint8_t range;
  uint8_t radius;
  uint8_t boundaryLeft;
  uint8_t boundaryRight;
  uint8_t boundaryUp;
  uint8_t boundaryDown;
  uint8_t aspectRatio;
  uint8_t Vx;
  uint8_t Vy;
};

struct avgPoint {
  double x;
  double y;
  float avgBrightness;
  float maxBrightness;
};

class wiiCam
{
  public:
    wiiCam(uint8_t sda, uint8_t scl);
    void initialize();
    void setSensitivity(uint8_t val);
    float getGain();
    void setGain(float gainTemp);
    uint8_t getPixelBrightnessThreshold();
    void setPixelBrightnessThreshold(uint8_t value);
    bool getOutput(uint8_t format);
    void getMeasuredPoint(uint8_t point);
    void setFramePeriod(float framePeriod);
    float getFramePeriod();
    uint8_t getObjectNumberSetting();
    void setObjectNumberSetting(uint8_t val);
  
    void setAverageCount(uint8_t averageCount);
    uint8_t getAverageCount();
    void setMirrorX(bool en);
    bool getMirrorX();
    void setMirrorY(bool en);
    bool getMirrorY();
    void setRotation(bool en);
    bool getRotation();
    void setOffset(int16_t x, int16_t y);
    void setOffsetX(int16_t x);
    void setOffsetY(int16_t y);
    int16_t getOffsetX();
    int16_t getOffsetY();
    void setScale(float scaleX, float scaleY);
    void setScaleX(float scale);
    void setScaleY(float scale);
    float getScaleX();
    float getScaleY();

    void setCalibrationEnable(bool en);
    bool getCalibrationEnable();
    int getCalibrationArray(int point, int axis);
    void setCalibrationArray(int point, int axis, int value);
    void orderCalibrationArray();
    void calculateHomographyMatrix();

    void setCalibrationOffsetEnable(bool en);
    bool getCalibrationOffsetEnable();
    int getCalibrationOffsetArray(int point, int axis);
    void setCalibrationOffsetArray(int point, int axis, int value);
    void orderCalibrationOffsetArray();
    void calculateOffsetHomographyMatrix();

    irPoint irPoints[16];
    uint8_t detectedPoints;

  private:
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg,uint8_t val);
    uint8_t sensitivityBlock1[9];
    uint8_t sensitivityBlock2[2];
    float framePeriod = 1;
    unsigned long exposureTime;
    float gain;
    uint8_t outputBuffer[256];
    uint8_t outputFormat;
    uint8_t objectNumber = 4;
    uint8_t _sda;
    uint8_t _scl;
    avgPoint _avgPoints[16];
    uint8_t _avgCounter;
    uint8_t _avgCount = 1;
    bool _calEn = false;
    bool _calOffsetEn = false;
    bool _mirrorX = false;
    bool _mirrorY = false;
    bool _rotation = false;
    int16_t _offsetX = 0;
    int16_t _offsetY = 0;
    float _scaleX = 1;
    float _scaleY = 1;
};
#endif /* wiiCam_H */
