/*
 * Copyright (c) 2021 by Cristian Deenen <cdeenen@outlook.com>
 * Pen firmware for Material Plane: https://github.com/cdeenen/materialplane
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
unsigned long sleepTimer = 0;

MC3419 accel(CS);

void setup() {
  //Initialize the accelerometer
  initializeAccel();

  //Initialize INT pins as inputs
  PORTC.DIRCLR = INT_IN;
  
  //Initialize LED pins as outputs
  pinMode(IR_LED_F, OUTPUT);
  pinMode(IR_LED_R, OUTPUT);
  pinMode(LED, OUTPUT);

  //Initialize switch pins as inputs
  pinMode(SW_R, INPUT_PULLUP);
  pinMode(SW_LF, INPUT_PULLUP);
  pinMode(SW_LR, INPUT_PULLUP);

  //Get the serial number
  serialNumber = getSerialNumber();

  //Configure PWM
  configurePWM();

  //Set interrupt mode for INT_IN
  PORTC.PIN2CTRL |= 0x02;

  //Set unused pins to low power mode
  pinsToLowPowerMode();
  
  //Set sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  //Enable sleep
  sleep_enable();
  
  //If pen is not tilted, sleep CPU
  if (PORTC.IN & INT_IN) sleep_cpu();
}


void loop() {
  
  //Read the switches
  uint8_t switches = readSwitches();

  //if pen is not tilted
  if ((PORTC.IN & INT_IN) == HIGH) {
    //reset sleep timer
    sleepTimer = millis();
  }
  //else if pin is tilted, and sleep timer has timed out
  else if ((PORTC.IN & INT_IN) == LOW && millis()-sleepTimer >= SLEEP_TIME) {
      
      //Switch leds off
      setPWM(PWM_OFF);
      digitalWrite(IR_LED_R,LOW);
      digitalWrite(LED,LOW);
  
      //Delay 1 ms to ensure IR led is off
      delay(1);
      
      //Sleep cpu
      sleep_cpu();

      //after wakeup, reset sleep timer
      sleepTimer = millis();

      digitalWrite(LED,HIGH);
  }
}

/*
 * Read the switches, and send the corresponding data
 */
uint8_t readSwitches() {
  uint8_t switches = 0;
  uint8_t data = ON_BM;
  
  //Read the switches
  if (digitalRead(SW_R) == LOW && digitalRead(SW_LF) == LOW) switches = LR_BM;
  else if (digitalRead(SW_R)==LOW) switches = L_BM;
  else if (digitalRead(SW_LF)==LOW) switches = R_BM;
  else if (digitalRead(SW_LR)==LOW) switches = RR_BM;

  //If any of the switches are pressed
  if (switches > 0) {
    //Store switch data in data int
    data |= switches;
  }
  
  //Switch on rear LED if rear switch is pressed
  digitalWrite(IR_LED_R, !digitalRead(SW_LR));
  
  //Send data over IR
  sendIR(data);

  return switches;
}

/*
 * Get the serial number of the pen
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

void configurePWM() {
  TCA0.SPLIT.CTRLB=TCA_SPLIT_HCMP2EN_bm;                  //PWM on WO5
  TCA0.SPLIT.HPER=PWM_COUNT;                              //Count down on WO5
  TCA0.SPLIT.HCMP2=PWM_OFF;                               //Set duty cycle
  TCA0.SPLIT.CTRLA=PWM_PRESCALER<<1|TCA_SPLIT_ENABLE_bm;  //Enable the timer and set prescaler
  PORTA.PIN5CTRL |= 0x80;                                 //Invert pin
}

/*
 * Interrupt service routines: clear interrupt flags for the pin ISR
 */
ISR(PORTC_PORT_vect) {
  PORTC.INTFLAGS=PORTC.INTFLAGS;  //clear pin interrupt flags
}

/*
 * Set all unused pins to power converving mode by disabling the input buffer
 */
 void pinsToLowPowerMode() {
  //PORTA.PIN0CTRL = 0x4;   //UPDI
  PORTA.PIN1CTRL = 0x4;     //MOSI
  PORTA.PIN2CTRL = 0x4;     //MISO
  PORTA.PIN3CTRL = 0x4;     //SCLK
  //PORTA.PIN4CTRL = 0x4;   //IR_LED_R
  //PORTA.PIN5CTRL = 0x4;   //IR_LED
  //PORTA.PIN6CTRL = 0x4;   //SW_LF
  PORTA.PIN7CTRL = 0x4;

  PORTB.PIN0CTRL = 0x4;
  PORTB.PIN1CTRL = 0x4;   
  //PORTB.PIN2CTRL = 0x4;   //SW_R
  PORTB.PIN3CTRL = 0x4;
  PORTB.PIN4CTRL = 0x4; 
  //PORTB.PIN5CTRL = 0x4;   //SW_LR

  //PORTC.PIN0CTRL = 0x4;   //LED_R
  PORTC.PIN1CTRL = 0x4;     //CS
  //PORTC.PIN2CTRL = 0x4;   //INT_IN
  PORTC.PIN3CTRL = 0x4;   
 }

/*
 * Initialize the accelerometer
 */
 void initializeAccel() {
    accel.initialize();
    accel.setDeviceMode(MODE_STANDBY);      //Set accelerometer in standby mode
    accel.setComControl(0b00000000);      //Set communication control. Set bit 5 to 1 for 3-wire SPI
    accel.setGPIOControl(0b00001100);       //Set GPIO control. Set bit 2 to 1 for INT active high, set bit 3 to 1 for INT push-pull
    accel.setSampleRate(SAMPLE_RATE);       //Set the sample rate
    accel.setInterrupt(0b01000011);         //Set the interrupt enable register, bit 0 enables tilt, bit 1 enables flip. Set bit 6 to 1 for autoclear
    accel.setTiltThreshold(TILT_THRESHOLD); //Set tilt threshold
    accel.setTiltDebounce(TILT_DEBOUNCE);   //Set tilt debounce
    accel.setMotionControl(0b00000001);     //Enable motion control features. Bit 0 enables tilt/flip, bit 2 enables anymotion (req for shake), bit 3 enables shake, bit 5 inverts z-axis
    accel.setRange(0b00000000);           //Set accelerometer range
    accel.clearInterrupts();                //Clear the interrupt register
    accel.resetMotionControl();             //Reset the motion control
    accel.setDeviceMode(MODE_WAKE);         //Wake up accelerometer
}
