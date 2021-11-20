/********************************************************************
 * Definitions
 ********************************************************************/

#define FIRMWARE_VERSION        "1.0.1"

/*
 * Switch Bitmasks
 */
#define ON_BM   0x08
#define L_BM    0x10
#define R_BM    0x20
#define LR_BM   0x56
#define RR_BM   0x40
#define OFF_BM  0x80

/*
 * Pin Definitions
 */
#define LED       PIN_PC0
#define IR_LED_F  PIN_PA5
#define IR_LED_R  PIN_PA4

#define SW_R      PIN_PB2
#define SW_LF     PIN_PA6
#define SW_LR     PIN_PB5 

//SPI CS pin
#define CS        PIN_PC1

//Interrupt pins
#define INT_IN PIN2_bm

/*
 * PWM Settings
 */
#define PWM_COUNT       129  //129 for 5MHz
#define PWM_PRESCALER   0
#define PWM_LOW         PWM_COUNT/2  
#define PWM_HIGH        0           
#define PWM_OFF         PWM_COUNT  

/*
 * IR Timing Settings
 */
#define IR_SHORT  480   //us
#define IR_LONG   980   //us

/*
 * ID Settings
 */
#define SIGROW  0x1100
#define SER_X   11 
#define SER_Y   12

/*
 * Misc
 */
#define TILT_THRESHOLD 15*TILT_ANGLE


/**
 * Force compilation error if board or clock is set incorrectly
 */
#ifndef __AVR_ATtiny416__
#error This firmware was written for the ATtiny416. Please select it from the Tools > Board menu.
#endif

#if (F_CPU!=5000000UL)
#error Wrong clock frequency selected. Please set it to "5 MHz Internal" in Tools > Clock
#endif
