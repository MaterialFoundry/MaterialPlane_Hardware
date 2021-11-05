/********************************************************************
 * Definitions
 ********************************************************************/

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
#define LED       PIN_PC0 //PA5
#define IR_LED_F  PIN_PA5
#define IR_LED_R  PIN_PA4
#define R_LED_F   PIN_PB4
#define R_LED_R   PIN_PC3

#define SW_R      PIN_PB2
#define SW_LF     PIN_PA6
#define SW_LR     PIN_PB5 

//SPI pins
#define CS      PIN_PC1
#define DATA    PIN_PA1
#define SCK     PIN_PA3

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
