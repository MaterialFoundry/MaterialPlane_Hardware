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
#define PWM_COUNT 121  //25 for 38khz, 29 for 33khz
#define PWM_PRESCALER 0
#define PWM_LOW PWM_COUNT/2  
#define PWM_HIGH 255

/*
 * IR timing settings
 */
#define IR_SHORT  450   //us
#define IR_LONG   900   //us

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
#define CS      PIN4_bm
#define DATA    PIN1_bm
#define SCK     PIN3_bm

//Interrupt pins
#define INT1_IN PIN2_bm
#define INT2_IN PIN2_bm

//LED pins
#define R_LED   PIN5_bm
#define IR_LED  PIN5_bm

/*
 * Misc
 */
#define TILT_THRESHOLD 15*TILT_ANGLE
