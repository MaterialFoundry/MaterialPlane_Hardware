/*
 * Copyright (c) 2021 by Cristian Deenen <cdeenen@outlook.com>
 * Base firmware for Material Plane: https://github.com/cdeenen/materialplane
 *
 * Personal use of this firmware is allowed.
 * Commercial use using official Material Plane hardware is allowed.
 * For redistribution or other commercial use, contact the copyright holder.
 * 
 * Firmware update instructions: https://github.com/CDeenen/MaterialPlane/wiki/Beta-Hardware-Guide#updating-the-base-or-pen-firmware
 * 
 * Configuration of the 'Tools' menu:
 *  Board: megaTinyCore => ATtiny3226/3216/1626...
 *  Chip or Board: ATtiny416
 *  Clock: 5MHz
 *  Rest is default
 */
 
#include <avr/sleep.h>
#include "definitions.h"
#include "userSettings.h"
#include "MC3419.h"

uint16_t serialNumber;
bool calMode = false;
unsigned long calModeTimer = 0;
volatile bool pitTriggered = false;

MC3419 accel(CS);

void setup() {
  #if defined(DEBUG) 
    Serial.begin(115200);
  #endif

  //Initialize the accelerometer
  initializeAccel();

  //Initialize INT pins as inputs
  PORTC.DIRCLR = INT_IN;
  
  //Initialize LED pins as outputs
  pinMode(IR_LED, OUTPUT);
  pinMode(LED, OUTPUT);

  //Set red LED low
  digitalWrite(LED, LOW);
  
  //Get the serial number
  serialNumber = getSerialNumber();

  //Configure PWM
  configurePWM();

  #if defined(ALWAYS_ON) 
    //Configure PIT
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
    RTC.PITINTCTRL = RTC_PI_bm;
    RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;    //set PIT to a period of 1Hz and enable it
  #endif

  //Set interrupt mode for INT_IN
  PORTC.PIN2CTRL |= 0x02;

  //Set unused pins to low power mode
  pinsToLowPowerMode();
  
  //Set sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  //Enable sleep
  sleep_enable();

  #if defined(IR_FREQUENCY_SWEEP) 
    frequencySweepMode();
  #endif

  //If base is held upside down, set base into calibration mode
  #if defined(DEBUG)
    if (accel.getZ() < 0) calMode = true;
  #else
    if (accel.getZ() > 0) calMode = true;
  #endif
  
  //Check if the base is currently not tilted and calMode is false. If so, sleep cpu to deactivate the base
  bool tilted = accel.getStatus()&1;
  if (!tilted && calMode == false) {
    //Sleep CPU
    sleep_cpu();
  }
    
  //Switch red led on
  digitalWrite(LED, HIGH);

}

void loop() {
  
  //Print debug data
  #if defined(DEBUG)
    printAccelData();
  #endif
  
  //Send IR message
  if (calMode) sendIR(CMD_CAL);
  else if (pitTriggered) sendIR(CMD_PIT);
  else sendIR(CMD);

  //Blink red led if base is in calibration mode
  if (calMode && millis() - calModeTimer >= CAL_LED_PERIOD/2) {
    digitalWrite(LED, LOW);
  }
  if (calMode && millis() - calModeTimer >= CAL_LED_PERIOD) {
    calModeTimer = millis();
    digitalWrite(LED, HIGH);
  }

  //Check if base is tilted. If not, and if not in calibration mode, start stop procedure to deactivate the base
  bool tilted = accel.getStatus()&1;
  if ((!tilted || pitTriggered) && calMode == false) {

    if (pitTriggered) {
      sendIR(CMD_PIT);
    }
    if (!pitTriggered) {
      //Send the normal command a few times. If base is tilted, return and continue 'active' operation
      for (int i=0; i<STOP_REPEATS; i++) {
        sendIR(CMD);
        tilted = accel.getStatus()&1;
        if (tilted) return;
      }

      //Send stop command
      uint8_t stopCMD = 1<<7 | CMD;
      sendIR(stopCMD);
    }
    
    
    
    //Switch leds off
    digitalWrite(LED, LOW);
    setPWM(PWM_OFF);

    //Delay 1 ms to ensure IR led is off
    delay(1);
    
    pitTriggered = false;
    #if defined(DEBUG) 
      //Print debug info
      while (!tilted) {
        accel.clearInterrupts();
        printAccelData();
        delay(100);
        tilted = accel.getStatus()&1;
      }
    #else
      //Sleep cpu to deactivate the base
      accel.clearInterrupts();
      RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;
      sleep_cpu();
    #endif

    //Switch red LED on after base activation
    digitalWrite(LED, HIGH);
  }
  else accel.clearInterrupts();
}

/**
 * PIT ISR
 */
ISR(RTC_PIT_vect) {
  RTC.PITINTFLAGS=RTC_PI_bm;;  //clear PIT interrupt flags
  pitTriggered = true;
}

/*
 * Interrupt service routines: clear interrupt flags
 */
ISR(PORTC_PORT_vect) {
  RTC.PITCTRLA = 0;
  PORTC.INTFLAGS=PORTC.INTFLAGS;  //clear pin interrupt flags
}

/*
 * Get the serial number of the base
 */
uint16_t getSerialNumber() {
  #ifdef ID
    return ID;
  #else
    uint8_t serX = *(uint8_t *)(SIGROW + SER_X);
    uint8_t serY = *(uint8_t *)(SIGROW + SER_Y);
    return serX | serY<<8;
  #endif
}

/*
 * Set PWM duty cycle
 */
void setPWM(uint8_t dutyCycle) {
  TCA0.SPLIT.HCMP2 = dutyCycle;
}

/*
 * Configure PWM registers
 */
void configurePWM() {
  TCA0.SPLIT.CTRLB=TCA_SPLIT_HCMP2EN_bm;                  //PWM on WO5
  TCA0.SPLIT.HPER=PWM_COUNT;                              //Count down on WO5
  TCA0.SPLIT.HCMP2=PWM_OFF;                               //Set duty cycle
  TCA0.SPLIT.CTRLA=PWM_PRESCALER<<1|TCA_SPLIT_ENABLE_bm;  //Enable the timer and set prescaler
  PORTA.PIN5CTRL |= 0x80;                                 //Invert pin
}

/*
 * Set all unused pins to power converving mode by disabling the input buffer
 */
 void pinsToLowPowerMode() {
  //PORTA.PIN0CTRL = 0x4;     //UPDI
  //PORTA.PIN1CTRL = 0x4;     //MOSI
  //PORTA.PIN2CTRL = 0x4;     //MISO
  //PORTA.PIN3CTRL = 0x4;     //SCLK
  PORTA.PIN4CTRL = 0x4;   
  //PORTA.PIN5CTRL = 0x4;     //IR_LED
  PORTA.PIN6CTRL = 0x4;
  PORTA.PIN7CTRL = 0x4;

  PORTB.PIN0CTRL = 0x4;
  PORTB.PIN1CTRL = 0x4;
  PORTB.PIN2CTRL = 0x4;   
  PORTB.PIN3CTRL = 0x4;
  PORTB.PIN4CTRL = 0x4;
  PORTB.PIN5CTRL = 0x4;   

  //PORTC.PIN0CTRL = 0x4;     //LED
  //PORTC.PIN1CTRL = 0x4;     //CS
  //PORTC.PIN2CTRL = 0x4;     //INT_IN
  PORTC.PIN3CTRL = 0x4;
 }

/*
 * Initialize the accelerometer
 */
 void initializeAccel() {
    accel.initialize();
    accel.setDeviceMode(MODE_STANDBY);      //Put accelerometer in standby mode
    accel.setGPIOControl(0b00001100);       //Set GPIO control. Set bit 2 to 1 for INT active high, set bit 3 to 1 for INT push-pull
    accel.setSampleRate(SAMPLE_RATE);       //Set the sample rate
    
    accel.setTiltThreshold(TILT_THRESHOLD); //Set tilt threshold
    accel.setTiltDebounce(TILT_DEBOUNCE);   //Set tilt debounce
    /*
    accel.setAnymotionThreshold(200);
    accel.setAnymotionDebounce(0);
    accel.setShakeThreshold(500);
    accel.setShakePeakToPeakDuration(50);
    accel.setShakeDuration(2);
    */
    #if defined(ANY_MOTION) 
      accel.setMotionControl(0b00000100);     //Enable motion control features. Bit 0 enables tilt/flip, bit 2 enables anymotion (req for shake), bit 3 enables shake, bit 5 inverts z-axis
      accel.setInterrupt(0b00000100);         //Set the interrupt enable register, bit 0 enables tilt, bit 1 enables flip, bit 3 enables shake. Set bit 6 to 1 for autoclear
      accel.setAnymotionThreshold(10);
      accel.setAnymotionDebounce(1);
      accel.setRange(0b00001001);           //Set accelerometer range
    #else
      accel.setMotionControl(0b00000001); 
      accel.setInterrupt(0b00000001);
      accel.setRange(0b00000000);           //Set accelerometer range
    #endif
    //accel.setRange(0b00000000);           //Set accelerometer range
    accel.clearInterrupts();                //Clear the interrupt register
    accel.resetMotionControl();             //Reset the motion control
    accel.setDeviceMode(MODE_WAKE);         //Wake up accelerometer
}

/*
 * If debug is enabled, print debug info to the serial port
 */
#if defined(DEBUG)
  void printAccelData() {
    Serial.print("C ");
    Serial.print(calMode);
    Serial.print("\tS ");
    Serial.print(accel.getStatus(),BIN);
    Serial.print("\tI ");
    Serial.print(accel.getInterruptStatus(),BIN);
    /*
    Serial.print("\tX: ");
    Serial.print(accel.getX());
    Serial.print("\t\tY: ");
    Serial.print(accel.getY());
    Serial.print("\t\tZ: ");
    Serial.print(accel.getZ());
    */
    Serial.println();
  }
#endif

void frequencySweepMode() {
  int frequency = IR_FREQUENCY_START;
  bool led = true;

  while(1) {
    digitalWrite(LED, led);
    led = !led;
    TCA0.SPLIT.HPER=frequency;
    for (int i=0; i<IR_FREQUENCY_REPEATS; i++)
      sendIR(frequency);
    frequency += IR_FREQUENCY_STEP;
    if (frequency >= IR_FREQUENCY_END)
      frequency = IR_FREQUENCY_START;
  }
}
