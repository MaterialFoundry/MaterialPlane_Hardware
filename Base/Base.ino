#include <avr/sleep.h>
#include "definitions.h"
#include "userSettings.h"
#include "MC3419.h"

uint16_t serialNumber;

MC3419 accel;

void setup() {

  //Initialize the accelerometer
  accel.initialize();

  //Initialize INT pins as inputs
  PORTC.DIRCLR = INT1_IN;
  
  //Initialize LED pins as outputs
  PORTA.DIRSET = R_LED;
  PORTB.DIRSET = IR_LED;

  PORTA.OUT |= R_LED;
  

  //Get the serial number
  serialNumber = getSerialNumber();

  //Configure PWM
  configurePWM();

  //Set unused pins to low power mode
  pinsToLowPowerMode();

  //Set interrupt mode for INT1_IN
  PORTC.PIN2CTRL |= 0x02;
  
  //Set sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  //Enable sleep
  sleep_enable();

  //If debug is enabled, switch on red led and transmit cmd indefinitely
  if (DEBUG) {
    PORTA.OUT |= R_LED;
    while(1) {
      sendIR(CMD);
    }
  }

  delay(500);
  //Set red led low
  PORTA.OUT &= ~R_LED;
  
  //Check if INT1 is currently asserted
  if ((PORTC.IN & INT1_IN) == LOW) {
    //Sleep CPU
    //sleep_cpu();
  }

  //Switch red led on
  PORTA.OUT |= R_LED;

}

void loop() {
  
  //Send IR message
  sendIR(CMD);

  if ((PORTC.IN & INT1_IN) == LOW) {

    for (int i=0; i<STOP_REPEATS; i++) {
      sendIR(CMD);
    }
    if ((PORTC.IN & INT1_IN) == LOW) {
      //Send stop command
      uint8_t stopCMD = 1<<7 | CMD;
      sendIR(stopCMD);
      
      //Switch leds off
      PORTA.OUT &= ~R_LED;
      TCA0.SINGLE.CMP2 = 0;
  
      //Delay 1 ms to ensure IR led is off
      delay(1);
      
      //Sleep cpu
      sleep_cpu();
      PORTA.OUT |= R_LED;
    }
      
      
    
  }
}

void configurePWM() {

  //Set WO2 of TCA0 to alternative pin
  PORTMUX.CTRLC = PORTMUX_TCA02_ALTERNATE_gc;

  //disable TCA0 and set divider to 1
  TCA0.SPLIT.CTRLA = 0; 

  //set CMD to RESET, and enable on both pins.
  TCA0.SPLIT.CTRLESET=TCA_SPLIT_CMD_RESET_gc|0x03; 

  //Split mode now off, CMPn = 0, CNT = 0, PER = 255
  TCA0.SPLIT.CTRLD=0; 

  //Single slope PWM mode, PWM on WO2
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP2EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); 

  //Set period
  TCA0.SINGLE.PER=PWM_COUNT;

  //Set duty cycle
  TCA0.SINGLE.CMP2=0;

  //Set prescaler and enable timer
  TCA0.SINGLE.CTRLA=PWM_PRESCALER<<1 | TCA_SINGLE_ENABLE_bm;
}

/*
 * Interrupt service routines: clear interrupt flags
 */
ISR(PORTC_PORT_vect) {
  PORTC.INTFLAGS=PORTC.INTFLAGS;  //clear pin interrupt flags
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
 * Set all unused pins to power converving mode by disabling the input buffer
 */
 void pinsToLowPowerMode() {
  //PORTA.PIN0CTRL = 0x4;   //UPDI
  PORTA.PIN1CTRL = 0x4;   //MOSI
  PORTA.PIN2CTRL = 0x4;   //MISO
  PORTA.PIN3CTRL = 0x4;   //SCLK
  PORTA.PIN4CTRL = 0x4;   //CS
  //PORTA.PIN5CTRL = 0x4;   //R_LED
  PORTA.PIN6CTRL = 0x4;
  PORTA.PIN7CTRL = 0x4;

  PORTB.PIN0CTRL = 0x4;
  PORTB.PIN1CTRL = 0x4;
  PORTB.PIN2CTRL = 0x4;   //INT2_IN
  PORTB.PIN3CTRL = 0x4;
  PORTB.PIN4CTRL = 0x4;
  //PORTB.PIN5CTRL = 0x4;   //IR_LED

  PORTC.PIN0CTRL = 0x4;
  PORTC.PIN1CTRL = 0x4;
  //PORTC.PIN2CTRL = 0x4;   //INT1_IN
  PORTC.PIN3CTRL = 0x4;
 }
