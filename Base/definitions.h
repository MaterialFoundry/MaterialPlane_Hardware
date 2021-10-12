/********************************************************************
 * Definitions
 ********************************************************************/

#define DEBUG false

/*
 * CMD
 * Sets the command to be sent
 */
#define CMD 1

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

//SPI pins
#define CS      PIN_PC1
#define DATA    PIN_PA1
#define SCK     PIN_PA3

//Interrupt pins
#define INT_IN PIN2_bm

//LED pins
#define LED       PIN_PC0 //PA5
#define IR_LED    PIN_PA5

/*
 * Misc
 */
#define TILT_THRESHOLD 15*TILT_ANGLE
