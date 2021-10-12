
#define FIRMWARE_VERSION        "2.1.0"

#define DEVICE_NAME             "materialsensor"

//=============================================================
//Debug
//=============================================================
/**
 * DEBUG
 * Turn on debugging over the serial port
 */
#define DEBUG                   false

/**
 * IR_DEBUG
 * Turn on IR tracking specific debugging over the serial port
 */
#define SERIAL_OUTPUT           false

//=============================================================
//IR Tracker
//=============================================================

/**
 * FRAME_PERIOD
 * The frame period is the time period between each sample in milliseconds, which is the inverse of the frame rate.
 * The frame rate in Hz = 1000 / FRAME_PERIOD
 * Decreasing the frame period will decrease the maximum exposure time, and might cause issues when smart IR bases are used.
 * The value is a float that can be between 5 and 100ms (200Hz and 0.01Hz). 
 */
 #define FRAME_PERIOD           5

/**
 * EXPOSURE_TIME (BETA kit only)
 * The exposure time sets the length of time in milliseconds that the sensor will gather data each time it is sampled. 
 * A higher exposure time allows the sensor to pick up dimmer IR sources, but increases noise.
 * The value is a float that can be between 0.02 and (FRAME_PERIOD-2.7)ms.
 */
 #define EXPOSURE_TIME          2

/**
 * GAIN
 * The gain is the amplification of the sensor data. 
 * Increasing the gain will allow the sensor to pick up dimmer IR sources, but will increase noise.
 * It is preferable to increase the exposure time before you increase the gain
 * The value is a float that can be between 1 and 8.
 */
 #define GAIN                   2

/**
 * DSP Settings
 * The DSP (digital signal processor) processes the data that is sampled by the sensor. 
 * Each sensor pixel has a brightness value between 0 and 255 and is put into one of 3 categories:
 * -Bright pixel: brightness > BRIGHTNESS_THRESHOLD
 * -Ambiguous pixel: BRIGHTNESS_THRESHOLD >= brightness > BRIGHTNESS_TRESHOLD-NOISE_TRESHOLD
 * -Dark pixel: BRIGHTNESS_TRESHOLD-NOISE_TRESHOLD > brightness
 * 
 * Bright pixels will be considered valid IR sources, while ambiguous pixels will be considered as a lower brightness area around the IR sources. Dark pixels will be ignored by the sensor. 
 * An area of bright pixels, surrounded by ambiguous pixels, surrounded by dark pixels will be considered as a single point.
 * 
 * BRIGHTNESS_TRESHOLD should be set high enough so the sensor filters out noise, but low enough that it reliably detects each IR base.
 * NOISE_TRESHOLD should be set in such a way that closely spaced IR bases will be identified as multiple IR points. Setting the value too high will result in multiple bases being considered a single point.
 * The values are integers between 0 and 255.
 */
 #define BRIGHTNESS_TRESHOLD    10
 #define NOISE_TRESHOLD         5  //(BETA kit only)

/**
 * Area Treshold  (BETA kit only)
 * The sensor calculates the area of each IR point, and can filter out any point that is either too large (area > MAX_AREA_TRESHOLD) or too small (area < MIN_AREA_TRESHOLD).
 * The values are integers between 0 and 255 (MIN_AREA_TRESHOLD) or 0 and 16382 (MAX_AREA_TRESHOLD).
 */
 #define MAX_AREA_TRESHOLD      20
 #define MIN_AREA_TRESHOLD      0

/**
 * MAX_IR_POINTS  (BETA kit only)
 * The MAX_IR_POINTS setting sets the maximum number of points the sensor will track.
 */
 #define MAX_IR_POINTS          16

//=============================================================
//MISC
//=============================================================
#define BAT_WARNING_PERCENTAGE  25

#define CAL_CANCEL              0x02
#define CAL_NEXT                0x03

#define PING_MAIN_LOOP          0
#define PING_CAL                1

#define PING_PERIOD             500

#define BATT_NOT_CHARGING       0
#define BATT_CHARGING           1
#define BATT_FULL               2

#define AVERAGE_NUM             2

#define LEDL_R_CH               0
#define LEDL_G_CH               1
#define LEDR_R_CH               2
#define LEDR_G_CH               3
#define LED_STEPSIZE            LEDL_R_INTENSITY/50
