 /*
 * MCU: Attiny416
 * Clock: 5MHz
 */
 
#include <avr/sleep.h>
#include "definitions.h"
#include "userSettings.h"
#include "MC3419.h"

uint16_t serialNumber;

MC3419 accel(DATA, SCK, CS);

void setup() {

  //Initialize the accelerometer
  initializeAccel();

  //Initialize INT pins as inputs
  PORTC.DIRCLR = INT_IN;
  
  //Initialize LED pins as outputs
  pinMode(IR_LED, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);
  
  //Get the serial number
  serialNumber = getSerialNumber();

  //Configure PWM
  configurePWM();

  //Set interrupt mode for INT_IN
  PORTC.PIN2CTRL |= 0x02;

  //Set unused pins to low power mode
  //pinsToLowPowerMode();
  
  //Set sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  //Enable sleep
  sleep_enable();

  //If debug is enabled, switch on red led and transmit cmd indefinitely
  if (DEBUG) {
    
    while(1) {
      digitalWrite(LED, HIGH);
      sendIR(CMD);
      delay(1000);
      digitalWrite(LED, LOW);
      sendIR(CMD);
      delay(1000);
    }
  }

  //Set red led low
  //PORTA.OUT &= ~R_LED;
  
  //Check if INT1 is currently asserted
  if ((PORTC.IN & INT_IN) == LOW) {
    //Sleep CPU
    sleep_cpu();
  }

  //Switch red led on
  digitalWrite(LED, HIGH);

}

void loop() {
  //digitalWrite(LED, (PORTC.IN & INT_IN));
  
  //Send IR message
  sendIR(CMD);

  if ((PORTC.IN & INT_IN) == LOW) {

    for (int i=0; i<STOP_REPEATS; i++) {
      sendIR(CMD);
    }
    if ((PORTC.IN & INT_IN) == LOW) {
      //Send stop command
      uint8_t stopCMD = 1<<7 | CMD;
      sendIR(stopCMD);
      
      //Switch leds off
      digitalWrite(LED, LOW);
      setPWM(PWM_OFF);
  
      //Delay 1 ms to ensure IR led is off
      delay(1);
      
      //Sleep cpu
      sleep_cpu();
      digitalWrite(LED, HIGH);
    }
  }
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

void setPWM(uint8_t dutyCycle) {
  TCA0.SPLIT.HCMP2 = dutyCycle;
}

void configurePWM() {
  TCA0.SPLIT.CTRLB=TCA_SPLIT_HCMP2EN_bm; //PWM on WO5
  TCA0.SPLIT.HPER=PWM_COUNT;  // Count down on WO5
  TCA0.SPLIT.HCMP2=PWM_OFF;   //0% duty cycle
  TCA0.SPLIT.CTRLA=PWM_PRESCALER<<1|TCA_SPLIT_ENABLE_bm; //enable the timer with prescaler of 16
  PORTA.PIN5CTRL |= 0x80;   //Invert pin
}

/*
 * Set all unused pins to power converving mode by disabling the input buffer
 */
 void pinsToLowPowerMode() {
  //PORTA.PIN0CTRL = 0x4;   //UPDI
  PORTA.PIN1CTRL = 0x4;     //MOSI
  PORTA.PIN2CTRL = 0x4;     //MISO
  PORTA.PIN3CTRL = 0x4;     //SCLK
  PORTA.PIN4CTRL = 0x4;   
  //PORTA.PIN5CTRL = 0x4;   //IR_LED
  PORTA.PIN6CTRL = 0x4;
  PORTA.PIN7CTRL = 0x4;

  PORTB.PIN0CTRL = 0x4;
  PORTB.PIN1CTRL = 0x4;
  PORTB.PIN2CTRL = 0x4;   
  PORTB.PIN3CTRL = 0x4;
  PORTB.PIN4CTRL = 0x4;
  PORTB.PIN5CTRL = 0x4;   

  //PORTC.PIN0CTRL = 0x4;     //LED
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
    accel.setInterrupt(0b01000001);    //Set the interrupt enable register, bit 0 enables tilt, bit 1 enables flip. Set bit 6 to 1 for autoclear
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
