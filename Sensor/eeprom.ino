//Addresses
#define FIRST_BOOT_ADDR     0
#define DEBUG_ADDR          1
#define SERIAL_OUTPUT_ADDR  2

#define SSID_ADDR           100   //max 32 chars
#define PASSWORD_ADDR       132    //max 64 chars
#define DEVICE_NAME_ADDR    210   //max 32 chars

#define WS_MODE_ADDR        200   //bool
#define WS_PORT_ADDR        201   //2 bytes
#define WS_IP_ADDR          203   //4 bytes

#define IR_FRAMEPERIOD_ADDR 300   //3 bytes
#define IR_EXPOSURE_ADDR    303   //2 bytes
#define IR_GAIN_ADDR        305   //1 byte
#define IR_BRIGHTNESS_ADDR  306   //1 byte
#define IR_NOISE_ADDR       307   //1 byte
#define IR_MINAREA_ADDR     308   //1 byte
#define IR_MAXAREA_ADDR     309   //2 bytes
#define IR_POINTS_ADDR      311   //1 byte
#define IR_AVG_ADDR         312   //1 byte    

#define CAL_EN_ADDR         400
#define OFFSET_EN_ADDR      401
#define MIRROR_X_ADDR       402
#define MIRROR_Y_ADDR       403
#define ROTATION_ADDR       404
#define SENSITIVITY_ADDR    405
#define CAL_ADDR            406   //8 * uint16_t = 16 bytes
#define OFFSET_ADDR         422   //8 * uint16_t = 16 bytes
#define CAL_OFFSET_X_ADDR   438   //2 bytes
#define CAL_OFFSET_Y_ADDR   440   //2 bytes
#define CAL_SCALE_X_ADDR    442   //4 bytes
#define CAL_SCALE_Y_ADDR    446   //4 bytes

void startupEeprom(){
  if (EEPROM.read(FIRST_BOOT_ADDR) != 1) firstBoot();
  getEepromSSID();
  getEepromPassword(); 
  getEepromDeviceName();
  if (nameString == "") {
    nameString = DEVICE_NAME;
    setEepromDeviceName(DEVICE_NAME,nameString.length());
  }
  wsPort = getEepromWsPort();
  wsMode = getEepromWsMode();
  debug = getEepromDebug();
  serialOutput = getEepromSerialOutput();
}

void initializeEepromIRsensor(){
  IRsensor.setAverageCount(getEepromAvg());
  IRsensor.setMirrorX(getEepromCalMirrorX());
  IRsensor.setMirrorY(getEepromCalMirrorY());
  IRsensor.setRotation(getEepromCalRotation());
  IRsensor.setOffset(getEepromCalOffsetX(), getEepromCalOffsetY());
  IRsensor.setScale(getEepromCalScaleX(), getEepromCalScaleY());
  IRsensor.setGain(getEepromIrGain());
  IRsensor.setPixelBrightnessThreshold(getEepromIrBrightness());
  IRsensor.setCalibrationEnable(getEepromCalCalibration());
  IRsensor.setCalibrationOffsetEnable(getEepromCalOffset());
  IRsensor.setFramePeriod(getEepromIrFramePeriod());
  readCal();
  readCalOffset();
  #if defined(HW_BETA)
    IRsensor.setExposureTime(getEepromIrExposure());
    IRsensor.setPixelNoiseTreshold(getEepromIrNoise());
    IRsensor.setMinAreaThreshold(getEepromMinArea());
    IRsensor.setMaxAreaThreshold(getEepromMaxArea());
    IRsensor.setObjectNumberSetting(getEepromIrPoints());
  #endif
}


void firstBoot(){
  Serial.println("Setting up default settings.");
  //Clear the EEPROM
  for (int i=0; i<512; i++) EEPROM.write(i,255);
  EEPROM.write(FIRST_BOOT_ADDR,1);
  String deviceName = DEVICE_NAME;
  setEepromDeviceName(DEVICE_NAME,deviceName.length());
  setEepromDebug(DEBUG);
  setEepromSerialOutput(SERIAL_OUTPUT);
  setEepromWsPort(WS_PORT_DEFAULT);
  setEepromWsMode(WS_MODE_SERVER);
  setEepromAvg(AVERAGE_NUM);
  setEepromIrFramePeriod(FRAME_PERIOD);
  setEepromIrExposure(EXPOSURE_TIME);
  setEepromIrGain(GAIN);
  setEepromIrBrightness(BRIGHTNESS_TRESHOLD);
  setEepromIrNoise(NOISE_TRESHOLD);
  setEepromIrMinArea(MIN_AREA_TRESHOLD);
  setEepromIrMaxArea(MAX_AREA_TRESHOLD);
  setEepromIrPoints(MAX_IR_POINTS);
  setEepromCalCalibration(false);
  setEepromCalOffset(false);
  setEepromCalMirrorX(false);
  setEepromCalMirrorY(false);
  setEepromCalRotation(false);
  setEepromCalOffsetX(0);
  setEepromCalOffsetY(0);
  setEepromCalScaleX(1);
  setEepromCalScaleY(1);
  EEPROM.commit(); 
}

uint16_t readEeprom(uint16_t addr) {
  return EEPROM.read(addr) | EEPROM.read(addr+1)<<8;
}

void writeEeprom(uint16_t addr, uint16_t data){
  EEPROM.write(addr,data&255);
  EEPROM.write(addr+1,(data>>8)&255);
}

void storeCal(){
  for (int i=0; i<2; i++)
    for (int j=0; j<4; j++)
      writeEeprom(CAL_ADDR+2*j+8*i, IRsensor.getCalibrationArray(j,i));
  EEPROM.commit();
}
void readCal(){
  for (int i=0; i<2; i++)
    for (int j=0; j<4; j++)
      IRsensor.setCalibrationArray(j,i,readEeprom(CAL_ADDR+2*j+8*i));
  IRsensor.calculateHomographyMatrix();
}

void storeCalOffset(){
  for (int i=0; i<2; i++)
    for (int j=0; j<4; j++)
      writeEeprom(OFFSET_ADDR+2*j+8*i, IRsensor.getCalibrationOffsetArray(j,i));
  EEPROM.commit();
}
void readCalOffset(){
  for (int i=0; i<2; i++)
    for (int j=0; j<4; j++)
      IRsensor.setCalibrationOffsetArray(j,i,readEeprom(OFFSET_ADDR+2*j+8*i));
  IRsensor.calculateOffsetHomographyMatrix();
}

void setEepromDebug(bool en){
  EEPROM.write(DEBUG_ADDR,en);
  EEPROM.commit();
}

bool getEepromDebug(){
  return EEPROM.read(DEBUG_ADDR);
}

void setEepromSerialOutput(bool en){
  EEPROM.write(SERIAL_OUTPUT_ADDR,en);
  EEPROM.commit();
}

bool getEepromSerialOutput(){
  return EEPROM.read(SERIAL_OUTPUT_ADDR);
}

void setEepromSSID(const char* ssid, int ssidLength) {
  for (int i=0; i<32; i++) {
    char c = 0;
    if (i<ssidLength) c = ssid[i];
    EEPROM.write(SSID_ADDR+i,c);
  }
  EEPROM.commit();
}

void getEepromSSID() {
  ssidString = "";
  uint8_t emptyCheck = 0;
  for (int i=0; i<32; i++) {
    char c = EEPROM.read(SSID_ADDR+i);
    if (c != 0) ssidString += c;
    if (c == 255) emptyCheck++;
  }
  if (emptyCheck == 32) {
    ssidString = "";
  }
}

void setEepromPassword(const char* password, int passwordLength) {
  for (int i=0; i<64; i++) {
    char c = 0;
    if (i<passwordLength) c = password[i];
    EEPROM.write(PASSWORD_ADDR+i,c);
  }
  EEPROM.commit();
}

void getEepromPassword() {
  passwordString = "";
  for (int i=0; i<64; i++) {
    char c = EEPROM.read(PASSWORD_ADDR+i);
    if (c != 0) passwordString += c;
  }
}

void setEepromDeviceName(const char* deviceName, int nameLength) {
  for (int i=0; i<32; i++) {
    char c = 0;
    if (i<nameLength) c = deviceName[i];
    EEPROM.write(DEVICE_NAME_ADDR+i,c);
  }
  EEPROM.commit();
}

void getEepromDeviceName() {
  nameString = "";
  uint8_t emptyCheck = 0;
  for (int i=0; i<32; i++) {
    char c = EEPROM.read(DEVICE_NAME_ADDR+i);
    if (c != 0) nameString += c;
    if (c == 255) emptyCheck++;
  }
  if (emptyCheck == 32) {
    nameString = "";
  }
}

void setEepromWsPort(uint16_t port) {
  writeEeprom(WS_PORT_ADDR, wsPort);
  EEPROM.commit();
}

uint16_t getEepromWsPort() {
  return readEeprom(WS_PORT_ADDR);
}

void setEepromWsMode(uint8_t Mode){
  EEPROM.write(WS_MODE_ADDR,Mode);
  EEPROM.commit();
}

uint8_t getEepromWsMode(){
  return EEPROM.read(WS_MODE_ADDR);
}

void setEepromWsIP(uint8_t IPa, uint8_t IPb, uint8_t IPc, uint8_t IPd){
  EEPROM.write(WS_IP_ADDR,IPa);
  EEPROM.write(WS_IP_ADDR+1,IPb);
  EEPROM.write(WS_IP_ADDR+2,IPc);
  EEPROM.write(WS_IP_ADDR+3,IPd);
  EEPROM.commit(); 
}

void getEepromWsIP(){
  uint8_t IPa = EEPROM.read(WS_IP_ADDR);
  uint8_t IPb = EEPROM.read(WS_IP_ADDR+1);
  uint8_t IPc = EEPROM.read(WS_IP_ADDR+2);
  uint8_t IPd = EEPROM.read(WS_IP_ADDR+3);
}

uint8_t getEepromAvg(){
  return EEPROM.read(IR_AVG_ADDR);
}

void setEepromAvg(uint8_t val) {
  EEPROM.write(IR_AVG_ADDR,val);
  EEPROM.commit(); 
}

void setEepromIrFramePeriod(float period){
  if (period > 100) period = 100;
  if (period < 5) period = 5;
  unsigned long framePeriod = 10000000/period;
  EEPROM.write(IR_FRAMEPERIOD_ADDR,framePeriod&0xFF);
  EEPROM.write(IR_FRAMEPERIOD_ADDR+1,(framePeriod>>8)&0xFF);
  EEPROM.write(IR_FRAMEPERIOD_ADDR+2,(framePeriod>>16)&0xFF);
  EEPROM.commit();
}

float getEepromIrFramePeriod() {
  unsigned long framePeriod = EEPROM.read(IR_FRAMEPERIOD_ADDR) | EEPROM.read(IR_FRAMEPERIOD_ADDR+1)<<8 | EEPROM.read(IR_FRAMEPERIOD_ADDR+2)<<16;
  return (float)10000000/framePeriod;
}

void setEepromIrExposure(float exposure) {
  #if defined(HW_BETA)
    float maxExposureTime = (float)IRsensor.getFramePeriod()-2.7;
    if (exposure > maxExposureTime) exposure = maxExposureTime;
    if (exposure > 13) exposure = 13;
    if (exposure < 0.02) exposure = 0.02;
   
    uint16_t exposureTime = exposure*5000;
  
    EEPROM.write(IR_EXPOSURE_ADDR,exposureTime&0xFF);
    EEPROM.write(IR_EXPOSURE_ADDR+1,(exposureTime>>8)&0xFF);
    EEPROM.commit();
  #endif
}

float getEepromIrExposure() {
  uint16_t exposureTime = EEPROM.read(IR_EXPOSURE_ADDR) | EEPROM.read(IR_EXPOSURE_ADDR+1)<<8;
  return exposureTime * 0.0002;
}

void setEepromIrGain(float gain) {
  uint8_t gainInt = round((gain-1)*32);
  EEPROM.write(IR_GAIN_ADDR, gainInt);
  EEPROM.commit();
}

float getEepromIrGain() {
  return EEPROM.read(IR_GAIN_ADDR)/32+1;
}

void setEepromIrBrightness(uint8_t brightness) {
  EEPROM.write(IR_BRIGHTNESS_ADDR, brightness);
  EEPROM.commit();
}

uint8_t getEepromIrBrightness() {
  return EEPROM.read(IR_BRIGHTNESS_ADDR);
}

void setEepromIrNoise(uint8_t noise) {
  EEPROM.write(IR_NOISE_ADDR, noise);
  EEPROM.commit();
}

uint8_t getEepromIrNoise() {
  return EEPROM.read(IR_NOISE_ADDR);
}

void setEepromIrMinArea(uint8_t area) {
  EEPROM.write(IR_MINAREA_ADDR, area);
  EEPROM.commit();
}

uint8_t getEepromMinArea() {
  return EEPROM.read(IR_MINAREA_ADDR);
}

void setEepromIrMaxArea(uint16_t area) {
  writeEeprom(IR_MAXAREA_ADDR, area);
  EEPROM.commit();
}

uint8_t getEepromMaxArea() {
  return readEeprom(IR_MAXAREA_ADDR);
}

void setEepromIrPoints(uint8_t points) {
  EEPROM.write(IR_POINTS_ADDR,points);
  EEPROM.commit();
}

uint8_t getEepromIrPoints() {
  #if defined(HW_BETA)
    return EEPROM.read(IR_POINTS_ADDR);
  #else
    uint8_t points = EEPROM.read(IR_POINTS_ADDR);
    if (points > 4) points = 4;
    return points;
  #endif
}

void setEepromCalCalibration(bool en){
  EEPROM.write(CAL_EN_ADDR,en);
  EEPROM.commit();
}

bool getEepromCalCalibration(){
  return EEPROM.read(CAL_EN_ADDR);
}

void setEepromCalOffset(bool en){
  EEPROM.write(OFFSET_EN_ADDR,en);
  EEPROM.commit();
}

bool getEepromCalOffset(){
  return EEPROM.read(OFFSET_EN_ADDR);
}

void setEepromCalMirrorX(bool en){
  EEPROM.write(MIRROR_X_ADDR,en);
  EEPROM.commit();
}

bool getEepromCalMirrorX(){
  return EEPROM.read(MIRROR_X_ADDR);
}

void setEepromCalMirrorY(bool en){
  EEPROM.write(MIRROR_Y_ADDR,en);
  EEPROM.commit();
}

bool getEepromCalMirrorY(){
  return EEPROM.read(MIRROR_Y_ADDR);
}

void setEepromCalRotation(bool rotation){
  EEPROM.write(ROTATION_ADDR,rotation);
  EEPROM.commit();
}

bool getEepromCalRotation(){
  return EEPROM.read(ROTATION_ADDR);
}

void setEepromCalOffsetX(int16_t offset) {
  writeEeprom(CAL_OFFSET_X_ADDR, offset);
  EEPROM.commit();
}

int16_t getEepromCalOffsetX() {
  return (int16_t)readEeprom(CAL_OFFSET_X_ADDR);
}

void setEepromCalOffsetY(int16_t offset) {
  writeEeprom(CAL_OFFSET_Y_ADDR, offset);
  EEPROM.commit();
}

int16_t getEepromCalOffsetY() {
  return (int16_t)readEeprom(CAL_OFFSET_Y_ADDR);
}

typedef union
{
  float value;
  uint8_t bytes[4];
} FLOATUNION_t;

void setEepromCalScaleX(float scale){
  FLOATUNION_t myFloat;
  myFloat.value = scale;
  EEPROM.write(CAL_SCALE_X_ADDR,myFloat.bytes[0]);
  EEPROM.write(CAL_SCALE_X_ADDR+1,myFloat.bytes[1]);
  EEPROM.write(CAL_SCALE_X_ADDR+2,myFloat.bytes[2]);
  EEPROM.write(CAL_SCALE_X_ADDR+3,myFloat.bytes[3]);
  EEPROM.commit();
}

float getEepromCalScaleX() {
  FLOATUNION_t myFloat;
  myFloat.bytes[0] = EEPROM.read(CAL_SCALE_X_ADDR);
  myFloat.bytes[1] = EEPROM.read(CAL_SCALE_X_ADDR+1);
  myFloat.bytes[2] = EEPROM.read(CAL_SCALE_X_ADDR+2);
  myFloat.bytes[3] = EEPROM.read(CAL_SCALE_X_ADDR+3);
  if (isnan(myFloat.value)) {
    setEepromCalScaleX(1);
    return 1;
  }
  else return myFloat.value;
}

void setEepromCalScaleY(float scale){
  FLOATUNION_t myFloat;
  myFloat.value = scale;
  EEPROM.write(CAL_SCALE_Y_ADDR,myFloat.bytes[0]);
  EEPROM.write(CAL_SCALE_Y_ADDR+1,myFloat.bytes[1]);
  EEPROM.write(CAL_SCALE_Y_ADDR+2,myFloat.bytes[2]);
  EEPROM.write(CAL_SCALE_Y_ADDR+3,myFloat.bytes[3]);
  EEPROM.commit();
}

float getEepromCalScaleY() {
  FLOATUNION_t myFloat;
  myFloat.bytes[0] = EEPROM.read(CAL_SCALE_Y_ADDR);
  myFloat.bytes[1] = EEPROM.read(CAL_SCALE_Y_ADDR+1);
  myFloat.bytes[2] = EEPROM.read(CAL_SCALE_Y_ADDR+2);
  myFloat.bytes[3] = EEPROM.read(CAL_SCALE_Y_ADDR+3);
  if (isnan(myFloat.value)) {
    setEepromCalScaleY(1);
    return 1;
  }
  else return myFloat.value;
}
