#ifndef PAJ7025R3_H
#define PAJ7025R3_H

#include "Arduino.h"

/**
 * Registers
 */
#define Bank_Select                         0xEF

//Bank 0x00
#define Bank0_Sync_Updated_Flag             0x01  
#define Product_ID                          0x02  
#define Cmd_oahb                            0x0B  
#define Cmd_nthd                            0x0F
#define Cmd_orientation_ratio               0x10
#define Cmd_dsp_operation_mode              0x12
#define Cmd_max_objects_num                 0x19
#define Cmd_FrameSubstraction_On            0x28
#define Cmd_Manual_PowerControl             0x2F
#define Cmd_Manual_PowerControl_Update_Req  0x30
#define Cmd_OtherGPO_13                     0x4F
#define Cmd_Global_RESETN                   0x64

//Bank 0x01
#define Bank1_Sync_Update_Flag              0x01
#define B_global_R                          0x05
#define B_ggh_r                             0x06
#define B_expo_R                            0x0E  
#define B_tg_outgen_DebugMode               0x2B

//Bank 0x0C
#define Cmd_frame_period                    0x07  
#define B_global                            0x0B
#define B_ggh                               0x0C
#define B_expo                              0x0F  
#define Cmd_oalb                            0x46
#define Cmd_thd                             0x47
#define Cmd_scale_resolution_x              0x60 
#define Cmd_scale_resolution_y              0x62 
#define Cmd_IOMode_GPIO_06                  0x6A
#define Cmd_IOMode_GPIO_08                  0x6C
#define Cmd_IOMode_GPIO_13                  0x71

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

/**
 * PAJ7025R3 class
 */

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
    bool getOutput(uint8_t format);
    void getMeasuredPoint(uint8_t point);
    bool getVsync();
    void setVsync(bool enable);
    bool getExposureSignal();
    void setExposureSignal(bool enable);

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
    uint32_t readRegister(uint8_t reg, uint8_t bytesToRead = 1);
    void writeRegister(uint8_t reg, uint32_t val, uint8_t bytesToWrite = 1);
    void switchBank(uint8_t bank);
    unsigned long framePeriod;
    unsigned long exposureTime;
    float gain;
    uint8_t outputBuffer[256];
    uint8_t outputFormat;
    uint8_t objectNumber;
    uint8_t _cs;
    avgPoint _avgPoints[16];
    uint8_t _avgCounter = 0;
    uint8_t _avgCount = 0;
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
#endif /* PAJ7025R3_H */
