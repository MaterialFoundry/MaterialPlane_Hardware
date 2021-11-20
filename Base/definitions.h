/********************************************************************
 * Definitions
 ********************************************************************/

#define FIRMWARE_VERSION        "1.1.0"

//#define DEBUG

/*
 * CMD
 * Sets the command to be sent
 */
#define CMD 1
#define CMD_CAL 3

/*
 * PWM settings
 */
#define PWM_COUNT       129  //129 for 5MHz
#define PWM_PRESCALER   0
#define PWM_LOW         PWM_COUNT/2  
#define PWM_HIGH        0           
#define PWM_OFF         PWM_COUNT           

/*
 * IR timing settings
 */
#define IR_SHORT  480   //us
#define IR_LONG   980   //us

/*
 * ID settings
 */
#define SIGROW  0x1100
#define SER_X   11 
#define SER_Y   12

/*
 * Pin Definitions
 */

//SPI CS pin
#define CS      PIN_PC1

//Interrupt pins
#define INT_IN PIN2_bm

//LED pins
#define LED       PIN_PC0
#define IR_LED    PIN_PA5

/*
 * Misc
 */
#define TILT_THRESHOLD 15*TILT_ANGLE
#define CAL_LED_PERIOD  500

/**
 * Force compilation error if board or clock is set incorrectly
 */
#ifndef __AVR_ATtiny416__
#error This firmware was written for the ATtiny416. Please select it from the Tools > Board menu.
#endif

#if (F_CPU!=5000000UL)
#error Wrong clock frequency selected. Please set it to "5 MHz Internal" in Tools > Clock
#endif
