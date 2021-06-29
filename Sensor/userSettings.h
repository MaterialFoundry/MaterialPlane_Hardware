//=============================================================
//Hardware settings
//=============================================================

/**
 * HW_VERSION
 * Sets the hardware version
 * HW_DIY_BASIC: Basic DIY sensor
 * HW_DIY_FULL: Full DIY sensor
 * HW_BETA: Beta production sensor
 * 
 * Uncomment the hardware you're using
 */
//#define HW_DIY_BASIC
#define HW_DIY_FULL
//#define HW_BETA

//=============================================================
//Basic DIY Sensor config
//=============================================================
#if defined(HW_DIY_BASIC)

  //SDA pins for the IR sensor
  #define SDA 21
  #define SCL 22
  
#endif

//=============================================================
//Full DIY Sensor config
//=============================================================
#if defined(HW_DIY_FULL)

  //SDA pins for the IR sensor
  #define SDA 21
  #define SCL 22

  //LED pins
  #define LEDL_R  15
  #define LEDL_G  27
  #define LEDR_R  26
  #define LEDR_G  25
  
  #define USB_ACTIVE 32
  
#endif

//=============================================================
//BETA Sensor config
//=============================================================
#if defined(HW_BETA)
  
  //LED pins
  #define LEDR_R  2
  #define LEDR_G  15
  #define LEDL_R  13
  #define LEDL_G  14

  //Battery voltage monitor pins
  #define VBAT_EN     32
  #define VBAT_SENSE  33
  #define VBAT_STAT   25
  
  #define USB_ACTIVE 27

  //IR pins
  #define IR_IN 22
  #define EXPOSURE 21 //G13/LED_SIDE
  #define PAJ_CS  4
  
#endif

//=============================================================
//WIFI
//=============================================================
/**
 * WIFI_TIMEOUT
 * When attempting to connect to the wifi, stop trying after WIFI_TIMEOUT amount of seconds
 */
 #define WIFI_TIMEOUT 5

/**
 * WS_PORT_DEFAULT
 * The default websocket port
 */
 #define WS_PORT_DEFAULT 3000
