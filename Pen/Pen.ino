#include <avr/sleep.h>
#include "definitions.h"
#include "userSettings.h"

uint16_t serialNumber;
bool active = false;
uint8_t data = 0;
unsigned long holdTimer = 0;
bool holdCheck = false;
bool holdActive = false;

void setup() {
  //Initialize IR LED pins as outputs
  pinMode(IR_LED_F, OUTPUT);
  pinMode(IR_LED_R, OUTPUT);

  //Initialize switch pins as inputs
  pinMode(SW_L, INPUT_PULLUP);
  pinMode(SW_RF, INPUT_PULLUP);
  pinMode(SW_RR, INPUT_PULLUP);

  //Get the serial number
  serialNumber = getSerialNumber();

  //Set PWM
  PORTMUX.CTRLC = PORTMUX_TCA02_ALTERNATE_gc;
  TCA0.SPLIT.CTRLA = 0; //disable TCA0 and set divider to 1
  TCA0.SPLIT.CTRLESET=TCA_SPLIT_CMD_RESET_gc|0x03; //set CMD to RESET, and enable on both pins.
  TCA0.SPLIT.CTRLD=0; //Split mode now off, CMPn = 0, CNT = 0, PER = 255
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP2EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); //Single slope PWM mode, PWM on WO2
  TCA0.SINGLE.PER=PWM_COUNT;
  TCA0.SINGLE.CMP0=2;
  TCA0.SINGLE.CTRLA=PWM_PRESCALER<<1 | TCA_SINGLE_ENABLE_bm;

  //Set interrupt mode for SW_RR
  PORTA.PIN2CTRL |= 0x03;
  
  //Set interrupt mode for SW_RF
  PORTA.PIN6CTRL |= 0x03;

  //Set interrupt mode for SW_L
  PORTB.PIN2CTRL |= 0x03;

  pinsToLowPowerMode();
  
  //Set sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  //Enable sleep
  sleep_enable();

  if (DEBUG) {
    while(1) {
      sendIR(0);
    }
  }
  
  //Sleep CPU
  sleep_cpu();
}


void loop() {

  //Check if both switches are pressed
  if (digitalRead(SW_RF)==LOW && digitalRead(SW_L)==LOW) {
    if (holdCheck == false) {
      holdCheck = true;
      holdTimer = millis();
    }
  }
  else {
    holdCheck = false;
  }
  
  if (holdCheck && millis() - holdTimer >= HOLD_TIME) {
    if (holdActive) {
      sendIR(ON_BM | OFF_BM);
      TCA0.SINGLE.CMP2 = 0;
      while(digitalRead(SW_RF) == LOW && digitalRead(SW_L)==LOW) {};
      delay(100);
      sleep_cpu();
    }
    else {
      while(digitalRead(SW_RF) == LOW && digitalRead(SW_L)==LOW) {
        sendIR(ON_BM);
        delay(100);
      }
      
    }
 
    holdActive = !holdActive;
    holdCheck = false;
  }
  
  if (holdActive) {
    uint8_t switches = 0;
    //Read the switches
    if (digitalRead(SW_L) == LOW && digitalRead(SW_RF) == LOW) switches = LR_BM;
    else if (digitalRead(SW_L)==LOW) switches = L_BM;
    else if (digitalRead(SW_RF)==LOW) switches = R_BM;
    else if (digitalRead(SW_RR)==LOW) switches = RR_BM;
  
  
    //If any of the switches are pressed
    if (switches > 0 || holdActive) {
      //Set active bool to true
      active = true;
  
      //Switch on rear LED if rear switch is pressed
      if (switches == RR_BM) digitalWrite(IR_LED_R,HIGH);
      else digitalWrite(IR_LED_R,LOW);
  
      //Store switch data in data int
      data = ON_BM | switches;
  
      //Send data over IR
      sendIR(data);
    }
  }
}

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
 * Interrupt service routines: clear interrupt flags for the pin ISR
 */
ISR(PORTA_PORT_vect) {
  PORTA.INTFLAGS=PORTA.INTFLAGS;  //clear pin interrupt flags
}

ISR(PORTB_PORT_vect) {
  PORTB.INTFLAGS=PORTB.INTFLAGS;  //clear pin interrupt flags
}

/*
 * Set all unused pins to power converving mode by disabling the input buffer
 */
 void pinsToLowPowerMode() {
  PORTA.PIN0CTRL = 0x4;
  PORTA.PIN1CTRL = 0x4;
  PORTA.PIN3CTRL = 0x4;
  PORTA.PIN5CTRL = 0x4;
  PORTA.PIN7CTRL = 0x4;

  PORTB.PIN0CTRL = 0x4;
  PORTB.PIN3CTRL = 0x4;
  PORTB.PIN4CTRL = 0x4;
  PORTB.PIN5CTRL = 0x4;

  PORTC.PIN0CTRL = 0x4;
  PORTC.PIN1CTRL = 0x4;
  PORTC.PIN2CTRL = 0x4;
  PORTC.PIN3CTRL = 0x4;
 }
