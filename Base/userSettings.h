/********************************************************************
 * User Settings
 ********************************************************************/
/*
 * ID
 * This sets the ID of the base. If left commented (with // in front), the ID will be gotten from the ID of the microcontroller. Only 16 bits of the full ID is being used, so it's possible
 * for multiple bases to have the same ID. Uncommenting this line (remove the //) will set the ID to the specified value.
 * Values are in the range from 0 to 65536
 */
//#define ID 0

/*
 * SAMPLE_RATE
 * Sets the sample rate of the accelerometer.
 * Higher sample rates improve response times, but will drain the battery faster.
 * 0x10 = 25Hz
 * 0x11 = 50Hz
 * 0x12 = 62.5Hz
 * 0x13 = 100Hz
 * 0x14 = 125Hz
 * 0x15 = 250Hz
 * 0x16 = 500Hz
 * 0x17 = 1000Hz
 * Default: 0x10 (25Hz)
 */
#define SAMPLE_RATE 0x10

/*
 * TILT_ANGLE
 * Angling the base at angles higher than TILT_ANGLE will activate the IR LED. 
 * Too low values might result in unintended activations due to sensor noise, your table being at an agle, or accidental bumps.
 * Values are in degrees, default: 10
 */
 #define TILT_ANGLE 10

/*
 * TILT_DEBOUNCE
 * Due to bumps or noise, a tilt might be accidentally detected. Using TILT_DEBOUNCE, you can filter out short triggers.
 * It works by analyzing the current angle, and ignoring any triggers (meaning angles bigger than TILT_ANGLE) until a certain amount of consecutive measurements
 * above TILT_ANGLE have been measured. TILT_DEBOUNCE sets this number of measurements.
 * Lower values result in a higher chance at unintended triggers, higher values result in a slower response time.
 * Value is a multiple of the sample rate. Default: 2
 */
#define TILT_DEBOUNCE 5

/*
 * STOP_REPEATS
 * After placing down the base, the base will continue sending data for a short amount of time.
 * STOP_REPEATS sets the number of times it will transmit.
 * Increasing it will leave the base on for longer, giving the sensor a better change at detecting the base's resting position, but it might cause interference with other bases.
 * Default: 2
 */
#define STOP_REPEATS 5

/*
 * IR_FREQUENCY_SWEEP
 * Uncommenting '#define IR_FREQUENCY_SWEEP' (removing '//') will put the base in IR frequency sweep mode.
 * In this mode, the base will not function as normal, instead, it will emit IR light at an increasing frequency for debugging purposes (in case the sensor isn't properly picking up the base ID).
 * It will sweep the value of PWM_COUNT starting at IR_FREQUENCY_START ending at IR_FREQUENCY_END in steps of IR_FREQUENCY_STEP. It will repeat each frequency IR_FREQUENCY_REPEATS times.
 * 
 * To use: observe the 'Coordinates' section in the sensor's webserver or within the Foundry 'Sensor Configuration'. Make sure the base is detected by the sensor (point 0 is showing data), and then observe the 'Command' field. 
 * At some point it should start displaying a value and counting up until the value stays constant. Look for the range of values that follow a consistent pattern (so using the default settings, an increase of 5 every second or so).
 * Take the middle value that falls in this range, and fill that in as PWM_COUNT in definitions.h. Then comment '#define IR_FREQUENCY_SWEEP' again by placing '//' before it, and reupload the firmware.
 * 
 * See this video for a demonstration: https://youtu.be/oVvm5mzcHHQ
 * You can see that it starts detecting the command at 80, and stops at 170. It also displays 250, but since this fell out of the consistent pattern, we can ignore it. To find the correct value for PWM_COUNT, we calculate the middle value:
 * (170-80)/2 + 80 = 125  [(maxValue-minValue)/2 + minValue]. You can increase the precision by decreasing IR_FREQUENCY_STEP or increasing IR_FREQUENCY_REPEATS, but this will increase the time it takes to perform this test.
 */
//#define IR_FREQUENCY_SWEEP
#define IR_FREQUENCY_START    10
#define IR_FREQUENCY_END      255
#define IR_FREQUENCY_STEP     5
#define IR_FREQUENCY_REPEATS  5
