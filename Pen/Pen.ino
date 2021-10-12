/*
 * Board: megaTinyCore => ATtiny3226... (without optiboot)
 * Chip or Board: ATtiny416
 * Clock: 5MHz
 * Rest is default
 */

#include <avr/sleep.h>
#include "definitions.h"
#include "userSettings.h"
#include "MC3419.h"

MC3419 accel(DATA, SCK, CS);

uint16_t serialNumber;
unsigned long sleepTimer = 0;

void setup() {
  //Initialize the accelerometer
  initializeAccel();

  //Initialize INT pins as inputs
  PORTC.DIRCLR = INT_IN;
  
  //Initialize LED pins as outputs
  pinMode(IR_LED_F, OUTPUT);
  pinMode(IR_LED_R, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(R_LED_F, OUTPUT);
  pinMode(R_LED_R, OUTPUT);

  //Initialize switch pins as inputs
  pinMode(SW_R, INPUT_PULLUP);
  pinMode(SW_LF, INPUT_PULLUP);
  pinMode(SW_LR, INPUT_PULLUP);

  //Get the serial number
  serialNumber = getSerialNumber();

  //Set PWM
  configurePWM();

/*
  //Set interrupt mode for SW_LR
  PORTA.PIN2CTRL |= 0x03;
  
  //Set interrupt mode for SW_LF
  PORTA.PIN6CTRL |= 0x03;

  //Set interrupt mode for SW_R
  PORTB.PIN2CTRL |= 0x03;
*/
  //Set interrupt mode for INT_IN
  PORTC.PIN2CTRL |= 0x02;

  pinsToLowPowerMode();
  
  //Set sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  //Enable sleep
  sleep_enable();


  //Test mode if switch RF is pressed during boot
  if (digitalRead(SW_LF)==LOW) {
    
    while(1) {
      if (digitalRead(SW_LR)==LOW) {
        TCA0.SINGLE.CMP0 = PWM_OFF;
        digitalWrite(IR_LED_R,LOW);
        break;
      }
      sendIR(0);
    }
  }

  //setPWM(PWM_LOW);
  //while(1){sendIR(0);}
  
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

  
  //Switch on or off F and R red LEDs
  digitalWrite(R_LED_R, !digitalRead(SW_LR));
  digitalWrite(R_LED_F, !digitalRead(SW_R) || !digitalRead(SW_LF));
  
  //Switch on rear LED if rear switch is pressed
  digitalWrite(IR_LED_R, !digitalRead(SW_LR));
  //if (switches == RR_BM) digitalWrite(IR_LED_R,HIGH);
  //else digitalWrite(IR_LED_R,LOW);
  //digitalWrite(IR_LED_R,LOW);
  
  //Send data over IR
  sendIR(data);

  return switches;
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

void setPWM(uint8_t dutyCycle) {
  TCA0.SPLIT.HCMP2 = dutyCycle;
  /*
  if (dutyCycle == PWM_HIGH) {
    TCA0.SPLIT.CTRLA=0;
    delay(10);
    PORTA.OUT &= ~PIN5_bm;
    //digitalWrite(IR_LED_F,HIGH);
  }
  else {
    TCA0.SPLIT.CTRLA=PWM_PRESCALER<<1|TCA_SPLIT_ENABLE_bm;
    //TCA0.SPLIT.HCMP2 = dutyCycle;
  }*/
}

void configurePWM() {

  //Set WO2 of TCA0 to alternative pin
  //PORTMUX.CTRLC = PORTMUX_TCA02_ALTERNATE_gc;

  //disable TCA0 and set divider to 1
  //TCA0.SPLIT.CTRLA = 0; 

  //set CMD to RESET, and enable on both pins.
  //TCA0.SPLIT.CTRLESET=TCA_SPLIT_CMD_RESET_gc|0x03; 
  
/*
  //Split mode now off, CMPn = 0, CNT = 0, PER = 255
  TCA0.SPLIT.CTRLD=0; 

  //Single slope PWM mode, PWM on WO2
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); 

  //Set period
  TCA0.SINGLE.PER=PWM_COUNT;

  //Set duty cycle
  TCA0.SINGLE.CMP0=PWM_OFF;

  //Set prescaler and enable timer
  TCA0.SINGLE.CTRLA=PWM_PRESCALER<<1 | TCA_SINGLE_ENABLE_bm;
  */

  TCA0.SPLIT.CTRLB=TCA_SPLIT_HCMP2EN_bm; //PWM on WO5
  TCA0.SPLIT.HPER=PWM_COUNT;  // Count down on WO5
  TCA0.SPLIT.HCMP2=PWM_OFF;  //0% duty cycle
  TCA0.SPLIT.CTRLA=PWM_PRESCALER<<1|TCA_SPLIT_ENABLE_bm; //enable the timer with prescaler of 16
  PORTA.PIN5CTRL |= 0x80;   //Invert pin
}

/*
 * Interrupt service routines: clear interrupt flags for the pin ISR
 */
 /*
ISR(PORTA_PORT_vect) {
  PORTA.INTFLAGS=PORTA.INTFLAGS;  //clear pin interrupt flags
}

ISR(PORTB_PORT_vect) {
  PORTB.INTFLAGS=PORTB.INTFLAGS;  //clear pin interrupt flags
}
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


 void initializeAccel() {
    accel.setDeviceMode(MODE_STANDBY);   //Set device in standby mode
    delay(SPI_DELAY);
    accel.setComControl(0b00000000);  //Set communication control. Set bit 5 to 1 for 3-wire SPI
    delay(SPI_DELAY);
    accel.setGPIOControl(0b00001100); //Set GPIO control. Set bit 2 to 1 for INT active high, set bit 3 to 1 for INT push-pull
    delay(SPI_DELAY);
    
    accel.setSampleRate(SAMPLE_RATE);         //Set the sample rate
    delay(SPI_DELAY);
    accel.setInterrupt(0b01000011);    //Set the interrupt enable register, bit 0 enables tilt, bit 1 enables flip. Set bit 6 to 1 for autoclear
    delay(SPI_DELAY);

    accel.setTiltThreshold(TILT_THRESHOLD);  //set tilt threshold
    delay(SPI_DELAY);
    accel.setTiltDebounce(TILT_DEBOUNCE);
    delay(SPI_DELAY);
    accel.setMotionControl(0b00000001);  //bit 0 enables tilt/flip, bit 5 inverts z-axis
    delay(SPI_DELAY);
    accel.setRange(0b00000000);
    delay(SPI_DELAY);
    
    
    accel.clearInterrupts();
    delay(SPI_DELAY);
    accel.setDeviceMode(MODE_WAKE);   //Wake up device
     
    accel.resetMotionControl();
    
}
