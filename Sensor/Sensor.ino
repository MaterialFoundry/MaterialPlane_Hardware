#include "configuration.h"
#include "userSettings.h"
#include "src/WebSocketsServer/src/WebSocketsServer.h"
#include "src/homography/homography.h"
#include "esp32-hal-cpu.h"
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <EEPROM.h>

#if defined(HW_DIY_BASIC)
  #include "src/wiiCam/wiiCam.h"
  wiiCam IRsensor;
#elif defined(HW_DIY_FULL)
  #include "src/wiiCam/wiiCam.h"
  #include "src/TinyPICO_Helper_Library/src/TinyPICO.h"
  wiiCam IRsensor;
  TinyPICO tp = TinyPICO();
#elif defined(HW_BETA)
  #include "src/PAJ7025R3/PAJ7025R3.h"
  #include "src/IR32-master/src/IRRecv.h"
  PAJ7025R3 IRsensor(PAJ_CS);
  IRRecv IDsensor;
#endif

#define WEBSOCKETS_NETWORK_TYPE NETWORK_ESP32 
#define MSG_LENGTH 100*MAX_IR_POINTS
#define WS_MODE_OFF 0
#define WS_MODE_SERVER 1
#define WS_MODE_CLIENT 2


bool debug = false;
bool serialOutput = false;

uint8_t maxIRpoints = 16;

bool calibration = false;
bool offsetOn = false;
bool mirrorX = false;
bool mirrorY = false;
bool rotation = false;
bool calOpen = true;
volatile uint8_t calibrationProcedure = 0;
bool calibrationRunning = false;
float framePeriod = 50;
unsigned long timer = 0;
uint8_t averageCount = 10;

uint8_t wsMode = WS_MODE_SERVER;
uint16_t wsPort = WS_PORT_DEFAULT;
String wsIP = "";
bool wsConnected = false;
uint8_t wsClients = 0;

uint16_t scale[2];

uint8_t irMode;
uint16_t irAddress;

float vBat = 0;
uint8_t chargeState = 0;
bool chargeLedOld = false;
bool lowBattery = false;

unsigned long pingTimer = 0;
unsigned long IRtimeout = 0;
volatile bool exposureDone = false;

String ssidString = "";
String passwordString = "";

void IRAM_ATTR pajInterruptHandler(){
  exposureDone = true;
}

bool noneCheck = false;

unsigned long IRtimer = 0;
unsigned long ledTimer = 0;

WebSocketsServer webSocketServer = WebSocketsServer(wsPort);
homography cal;

void setup() {
  initialization();
}

void loop() {
 
  if (Serial.available() > 0) {
    bool result = checkSerial(); 
    if (result == true) if (debug) Serial.println("OK");
    else if (result == false) if (debug) Serial.println("ERROR"); 
  }
  
  wsPing();
  
  webSocketServer.loop();

  getCal();
    
  #if defined(HW_BETA)
    if (exposureDone) {
      exposureDone = false;
      IRtimer = millis();
      timer = micros();
      readIR();
      //unsigned long timer2 = micros()-timer;
      //Serial.println(timer2);
    }

    while(IDsensor.available()){
      char* rcvGroup;
      uint32_t result = IDsensor.read(rcvGroup);
      //Serial.println("IR ID sensor0. Address: " + (String)(result>>8) + "\tMode: " + (String)(result&255));
      if (rcvGroup == "MP" && result) {
          irMode = result&255;
          irAddress = result>>8;
          if (debug) Serial.println("IR ID sensor. Address: " + (String)irAddress + "\tMode: " + (String)irMode);
          //Serial.println("IR ID sensor1. Address: " + (String)irAddress + "\tMode: " + (String)irMode);
      }
    }
  
    if (millis()-IRtimeout >= 10) {
      bool productId = IRsensor.checkProductId();
      //Serial.println("ProductID: " + (String)IRsensor.checkProductId());
      if (productId == false) {
        Serial.println("Sensor reset");
        IRsensor.initialize();
        initializeEepromIRsensor();
      }
      IRtimeout = millis();
    }
  #else
    if (millis() - IRtimer > framePeriod) {
      IRtimer = millis();
      readIR();
    }
  #endif

  #if defined(HW_DIY_FULL) 
    if (millis()-ledTimer >= 2){
      getBatteryVoltage();
      ledTimer = millis();
      if (webSocketServer.connectedClients() > 0) setRightLED(true);
      else                                        setRightLED(false);
    } 
  #elif defined(HW_BETA)
    if (millis()-ledTimer >= 1000){
      getBatteryVoltage();
      ledTimer = millis();
      if (webSocketServer.connectedClients() > 0) setRightLED(true);
      else                                        setRightLED(false);
    }                                     
  #endif
}

float batStorage = 0;
uint16_t chargeCounter = 0;
uint16_t chargeSum = 0;

void getBatteryVoltage() {
  #if defined(HW_DIY_FULL)
    batStorage += tp.GetBatteryVoltage();
    chargeSum += (int)tp.IsChargingBattery();
    chargeCounter++;
    
    if (chargeCounter >= 500) {
      chargeState = 0;
      if (chargeSum < 15) { //full battery
        chargeState = BATT_FULL;
      }
      else if (chargeSum < 75){ //no battery or not charging
        chargeState = BATT_NOT_CHARGING;
      }
      else {                  //charging
        chargeState = BATT_CHARGING;
      }
      setLeftLED(chargeState);
      vBat = batStorage/chargeCounter;
      batStorage = 0;
      chargeSum = 0;
      chargeCounter = 0;
    }
    
  #elif defined(HW_BETA)
    digitalWrite(VBAT_EN,HIGH);
    unsigned long bat = 0;
    for (int i=0; i<10; i++) bat += analogRead(VBAT_SENSE);
    bat *= 0.1;
    vBat = bat*3.1/(4096*0.68);
    if (digitalRead(VBAT_STAT == LOW)) {
      chargeState = BATT_CHARGING;
    }
    else {
      chargeState = BATT_FULL;
    }
    setLeftLED(chargeState);
  #endif
}

void connectWifi(const char* ssid, const char* password, uint8_t ssidLength, uint8_t passwordLength) {
  ssidString = "";
  for (int i=0; i<ssidLength; i++) ssidString += ssid[i];
  
  if (WiFi.isConnected()) {
    Serial.println("Disconnecting from: \"" + (String)WiFi.SSID() + "\"");
    WiFi.disconnect();
  }
  Serial.print("Attempting to connect to \"" + (String)ssid + "\". Please wait");
  
  WiFi.begin(ssid,password);
  
  int counter = 0;
  while(WiFi.status() != WL_CONNECTED) {
    counter++;
    if (counter >= WIFI_TIMEOUT*2) {
      Serial.println("\nConnection failed\n");
      return;
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.print("\nWiFi connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

#if defined(HW_DIY_FULL) || defined(HW_BETA)
  /**
   * Set the right led, depending on whether there are any active connections
   */
  void setRightLED(bool connections){
    if (connections) {
      digitalWrite(LEDR_R,LOW);
      digitalWrite(LEDR_G,HIGH);
    }
    else {
      digitalWrite(LEDR_R,HIGH);
      digitalWrite(LEDR_G,LOW);
    }
  }
  
  /**
   * Set the left led, depending on power/battery state
   */
  void setLeftLED(int batteryState){
    #if defined(HW_DIY_FULL)
      if(analogRead(USB_ACTIVE)>1000){
    #elif defined(HW_BETA)
      if(digitalRead(USB_ACTIVE)){
    #endif
      if (batteryState == BATT_NOT_CHARGING) {  //no battery or not charging
        digitalWrite(LEDL_R,LOW);
        digitalWrite(LEDL_G,LOW);
      }
      else if (batteryState == BATT_CHARGING) { //charging
        if (chargeLedOld) digitalWrite(LEDL_R,LOW);
        else digitalWrite(LEDL_R,HIGH);
        chargeLedOld = !chargeLedOld;
        digitalWrite(LEDL_G,LOW);
      }
      else if (batteryState == BATT_FULL) {    //full battery
        digitalWrite(LEDL_R,LOW);
        digitalWrite(LEDL_G,HIGH);
      }
    }
    else {
      chargeState = BATT_NOT_CHARGING;
      if (vBat < BAT_WARNING_VOLTAGE){
        lowBattery = true;
        digitalWrite(LEDL_R,HIGH);
        digitalWrite(LEDL_G,LOW);
      }
      else {
        lowBattery = false;
        digitalWrite(LEDL_R,LOW);
        digitalWrite(LEDL_G,HIGH);
      }
    }
  }
#endif
