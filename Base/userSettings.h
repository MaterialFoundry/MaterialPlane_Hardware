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
