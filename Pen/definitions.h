/********************************************************************
 * Definitions
 ********************************************************************/

#define DEBUG false

/*
 * Switch Bitmasks
 */
#define ON_BM   0x08
#define L_BM    0x10
#define R_BM    0x20
#define LR_BM    0x56
#define RR_BM   0x40
#define OFF_BM  0x80

/*
 * Pin Definitions
 */
#define LED PIN_PA5 //PA5
#define IR_LED_F  PIN_PB5
#define IR_LED_R  PIN_PA4

#define SW_L  PIN_PB2
#define SW_RF PIN_PA6
#define SW_RR PIN_PA2 

/*
 * PWM Settings
 */
#define PWM_COUNT 121  //25 for 38khz, 29 for 33khz
#define PWM_PRESCALER 0
#define PWM_LOW PWM_COUNT/2  
#define PWM_HIGH 255

/*
 * IR Timing Settings
 */
#define IR_SHORT  450   //us
#define IR_LONG   900   //us

/*
 * ID Settings
 */
#define SIGROW  0x1100
#define SER_X   11 
#define SER_Y   12
