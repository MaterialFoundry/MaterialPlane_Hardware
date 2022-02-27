#include "configuration.h"
#include "userSettings.h"
#include "src/WebSocketsServer/src/WebSocketsServer.h"
//#include "src/homography/homography.h"
#include "esp32-hal-cpu.h"
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include "FS.h"
#include "SPIFFS.h"
#include "src/ESP32WebServer/src/ESP32WebServer.h"
#include "esp_adc_cal.h"


#if defined(HW_DIY_BASIC)
  #include "src/wiiCam/wiiCam.h"
  wiiCam IRsensor(SDA,SCL);
#elif defined(HW_DIY_FULL)
  #include "src/wiiCam/wiiCam.h"
  #include "src/TinyPICO_Helper_Library/src/TinyPICO.h"
  wiiCam IRsensor(SDA,SCL);
  TinyPICO tp = TinyPICO();
#elif defined(HW_BETA)
  #include "src/PAJ7025R3/PAJ7025R3.h"
  #include "src/IR32/src/IRRecv.h"
  
  PAJ7025R3 IRsensor(PAJ_CS);
  IRRecv IDsensor;
#endif

#define WEBSOCKETS_NETWORK_TYPE NETWORK_ESP32 
#define MSG_LENGTH 150*MAX_IR_POINTS
#define WS_MODE_OFF 0
#define WS_MODE_SERVER 1
#define WS_MODE_CLIENT 2

bool debug = false;
bool serialOutput = false;

/**
 * IR Sensor variables
 */
volatile bool exposureDone = false;
bool calOpen = true;
volatile uint8_t calibrationProcedure = 0;
volatile uint8_t calibrationMode = 0;
bool calibrationRunning = false;

/**
 * Websocket variables
 */
uint8_t wsMode = WS_MODE_SERVER;
uint16_t wsPort = WS_PORT_DEFAULT;
String wsIP = "";
bool wsConnected = false;
uint8_t wsClients = 0;

/**
 * ID sensor variables
 */
uint8_t irMode;
uint16_t irAddress;

/**
 * Battery monitoring variables
 */
float vBat = 0;
uint8_t chargeState = 0;
bool lowBattery = false;
uint8_t batteryCounter = 0;


/**
 * WiFi Variables
 */
String ssidString = "";
String passwordString = "";
String nameString = "";

/**
 * LED variables
 */
unsigned long ledTimer = 0;
unsigned long leftLedTimer = 0;

float batStorage = 0;
uint16_t chargeCounter = 0;
uint16_t chargeSum = 0;
bool usbActive = false;
uint8_t usbCounter = 0;
uint8_t batPercentage = 0;
uint8_t autoExposureProcedure = EXP_STOPPED;


ESP32WebServer webServer(80);
DNSServer dnsServer;
WebSocketsServer webSocketServer = WebSocketsServer(wsPort);
//homography cal;

//Tasks
TaskHandle_t irSensorTask;
TaskHandle_t pingTask;
TaskHandle_t core0Task;
TaskHandle_t comTask;

void IRAM_ATTR pajInterruptHandler(){
  exposureDone = true;
}

void setup() {
  initialization();
}

void core0Loop( void * parameter ) {
 while(1) {
  webServer.handleClient();
  if (WiFi.status() != WL_CONNECTED) {
    dnsServer.processNextRequest();
  }
  delay(1);
 }
}

/*void comLoop( void * parameter ) {
  while(1) {
    if (Serial.available() > 0) {
      bool result = checkSerial(); 
      if (result == true) if (debug) Serial.println("OK");
      else if (result == false) if (debug) Serial.println("ERROR"); 
    }
    webSocketServer.loop();
    delay(1);
  }
}*/

void loop() {
  readIR();

  if (Serial.available() > 0) {
      bool result = checkSerial(); 
      if (result == true) if (debug) Serial.println("OK");
      else if (result == false) if (debug) Serial.println("ERROR"); 
    }
    webSocketServer.loop();

  #if defined(HW_DIY_FULL) 
    if (millis()-ledTimer >= 2){
      getBatteryVoltage();
      ledTimer = millis();
      if (webSocketServer.connectedClients() > 0) setRightLED(true);
      else                                        setRightLED(false);
    } 

    if (millis()-leftLedTimer >= CHARGING_LED_TIMER) {
      leftLedTimer = millis();
      setLeftLED(chargeState);
    }
  #elif defined(HW_BETA)
    while(IDsensor.available()){
      char* rcvGroup;
      uint32_t result = IDsensor.read(rcvGroup);
      if (rcvGroup == "MP" && result) {
          irMode = result&255;
          irAddress = result>>8;
          if (debug) Serial.println("IR ID sensor. Address: " + (String)irAddress + "\tMode: " + (String)irMode);
      }
      else if (result) {
        sendIRcode(rcvGroup, result);
      }
    }
  
    if (millis()-ledTimer >= 100){
      ledTimer = millis();
      batteryCounter++;
      
      //Check battery voltage every second
      if (batteryCounter == 10) {
        batteryCounter = 0;
        getBatteryVoltage();
      }
      
      if (webSocketServer.connectedClients() > 0) setRightLED(true);
      else                                        setRightLED(false);
    }    
                                     
    if (millis()-leftLedTimer >= CHARGING_LED_TIMER) {
      leftLedTimer = millis();
      setLeftLED(chargeState);
    }
  #endif
  
}

void getBatteryVoltage() {
  #if defined(HW_DIY_FULL)
    usbActive = analogRead(USB_ACTIVE)>1000 ? true : false;
    batStorage += tp.GetBatteryVoltage();
    chargeSum += (int)tp.IsChargingBattery();
    chargeCounter++;
    
    if (chargeCounter >= 500) {
      chargeState = 0;
      if (chargeSum < 40) { //full battery
        chargeState = BATT_FULL;
      }
      else if (chargeSum < 75){ //no battery or not charging
        chargeState = BATT_NOT_CHARGING;
      }
      else {                  //charging
        chargeState = BATT_CHARGING;
      }
      setLeftLED(chargeState);
      vBat = processBattery(batStorage/chargeCounter);
      batPercentage = getBatteryPercentage(vBat);
      if (debug) Serial.println("Battery Voltage: " + (String)vBat + "V\tCharge Sum: " + (String)chargeSum + "\tCharge State: " + (String)chargeState);
      batStorage = 0;
      chargeSum = 0;
      chargeCounter = 0;
    }
    
  #elif defined(HW_BETA)
    usbActive = digitalRead(USB_ACTIVE);
    //Disable charging to get more accurate battery voltage
    //digitalWrite(CHARGE_EN,HIGH);
    
    //Short delay for battery voltage to settle after charge disable
    delay(50);
    
    digitalWrite(VBAT_EN,HIGH);
    unsigned long bat = 0;
    for (int i=0; i<10; i++) bat += analogRead(VBAT_SENSE);
    bat *= 0.1;
    vBat = processBattery((float)readADC_Cal(bat)*1.468/1000);

    batPercentage = getBatteryPercentage(vBat);
    if (debug) Serial.println("Battery Voltage: " + (String)vBat + "V\tPercentage: " + (String)batPercentage + "%\tCharge Sum: " + (String)chargeSum + "\tCharge State: " + (String)chargeState);

    if (batPercentage >= 98) {
      chargeState = BATT_FULL;
    }
    else if (digitalRead(VBAT_STAT == LOW) && vBat < 4.15) {
      chargeState = BATT_CHARGING;
    }
    
    setLeftLED(chargeState);

    //Enable charging
    digitalWrite(CHARGE_EN,LOW);

    if (vBat < BAT_SLEEP_VOLTAGE) sleepSensor();
  #endif
}

uint32_t readADC_Cal(int ADC_Raw)
{
  esp_adc_cal_characteristics_t adc_chars;
  
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  return(esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
}

float voltageStorage[10] = {0,0,0,0,0,0,0,0,0,0};
uint8_t voltageCount = 0;

float processBattery(float vbat) {
  for (int i=8; i>=0; i--) {
    voltageStorage[i+1] = voltageStorage[i];
  }
  voltageStorage[0] = vbat;
  if (voltageCount < 10) voltageCount++;
  float tempVoltage = 0;
  for (int i=0; i<voltageCount; i++) tempVoltage += voltageStorage[i];
  return tempVoltage / voltageCount;
}

/**
 * Get a very rough estimate of the battery voltage.
 */
uint8_t getBatteryPercentage(float v) {
  if (v > 4.2) v = 4.20;
  int8_t percentage = 0;
  if (v < 3.25) percentage =  round(80*(v-3.00));
  else if (v >= 3.25 && v < 3.75) percentage =  round(20 + 120*(v-3.25));
  else if (v >= 3.75 && v < 4.00) percentage =  round(80 + 60*(v-3.75));
  else if (v >= 4.00) percentage =  round(95 + 50*(v-4.00));
  if (percentage < 0) percentage = 0;
  else if (percentage > 100) percentage = 100;
  return percentage;
}

#if defined(HW_DIY_FULL) || defined(HW_BETA)
  /**
   * Set the right led, depending on whether there are any active connections
   */
  void setRightLED(bool connections){
    if (connections) {
      ledcWrite(LEDR_R_CH, 0);
      ledcWrite(LEDR_G_CH, LEDR_G_INTENSITY);
    }
    else {
      ledcWrite(LEDR_R_CH, LEDR_R_INTENSITY);
      ledcWrite(LEDR_G_CH, 0);
    }
  }

  float battLedDuty = 0;
  bool battLedDir = 1;
  
  /**
   * Set the left led, depending on power/battery state
   */
  void setLeftLED(int batteryState){
    if (usbActive) {
      if (batteryState == BATT_NOT_CHARGING) {  //no battery or not charging
        ledcWrite(LEDL_R_CH, 0);
        ledcWrite(LEDL_G_CH, 0);
      }
      else if (batteryState == BATT_CHARGING) { //charging

        //digitalWrite(LEDL_R,HIGH);
        ledcWrite(LEDL_R_CH, (int16_t)battLedDuty);
   
        if (battLedDir) battLedDuty += CHARGING_LED_STEPSIZE;
        else battLedDuty -= CHARGING_LED_STEPSIZE;
        
        if (battLedDuty >= LEDL_R_INTENSITY) {
          battLedDuty = LEDL_R_INTENSITY;
          battLedDir = 0;
        }
        else if (battLedDuty < CHARGING_LED_MIN) {
          battLedDuty = CHARGING_LED_MIN;
          battLedDir = 1;
        }

        ledcWrite(LEDL_G_CH, 0);
      }
      else if (batteryState == BATT_FULL) {    //full battery
        ledcWrite(LEDL_R_CH, 0);
        ledcWrite(LEDL_G_CH, LEDL_G_INTENSITY);
      }
    }
    else {
      chargeState = BATT_NOT_CHARGING;
      if (batPercentage < BAT_WARNING_PERCENTAGE){
        lowBattery = true;
        ledcWrite(LEDL_R_CH, LEDL_R_INTENSITY);
        ledcWrite(LEDL_G_CH, 0);
      }
      else {
        lowBattery = false;
        ledcWrite(LEDL_R_CH, 0);
        ledcWrite(LEDL_G_CH, LEDL_G_INTENSITY);
      }
    }
  }
#endif

void stringToChar(String in, char* out) {
  in.toCharArray(out,in.length()+1);
}

#if defined(HW_BETA)
  void sleepSensor() {
    Serial.println("Low voltage detected, please recharge the sensor. Switching to hibernation.");
    String msg = "{\"status\":\"lowVoltage\"}";
    webSocketServer.broadcastTXT(msg);
    if (serialOutput) Serial.println(msg);
    
    //shut down PAJ sensor
    IRsensor.powerOn(false);
    
    //hibernate ESP32 for 10 minutes
    esp_sleep_enable_timer_wakeup(600000000);
    esp_deep_sleep_start();
  
    ESP.restart();
  }
#endif
