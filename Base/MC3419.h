/*
 * Copyright (c) 2021 by Cristian Deenen <cdeenen@outlook.com>
 * MEMSIC MC3419 accelerometer library designed for Material Plane: https://github.com/cdeenen/materialplane
 *
 * Personal use of this library is allowed.
 * Commercial use using official Material Plane hardware is allowed.
 * For redistribution or other commercial use, contact the copyright holder.
 */
 
#include "Arduino.h"

#ifndef MC3419_H
#define MC3419_H

/********************************************************************
 * Register Adresses
 ********************************************************************/
#define DEV_STATUS_REG        0x05
#define INTR_CTRL_REG         0x06
#define MODE_REG              0x07
#define RATE_REG              0x08
#define MOTION_CTRL_REG       0x09
#define STATUS_REG            0x13
#define INTR_STAT_REG         0x14

#define RANGE_REG             0x20
#define COMM_CTRL_REG         0x31
#define GPIO_CTRL_REG         0x33
#define TF_THRESH_REG         0x40
#define TF_DEBOUNCE_REG       0x42
#define AM_THRESH_REG         0x43
#define AM_DB_REG             0x45
#define SHK_THRESH_REG        0x46
#define PK_P2P_DUR_THRES_REG  0x48
#define TIMER_CTRL_REG        0x4A

#define XOUT_REG              0x0D
#define YOUT_REG              0x0F
#define ZOUT_REG              0x11

/********************************************************************
 * Defaults
 ********************************************************************/
#define MODE_STANDBY          0b00
#define MODE_WAKE             0b01

#define SPI_DELAY             10  //Delay in ms after each SPI write

class MC3419
{
  public:

    /*
     * Class constructor
     * 
     * @param cs (uint8_t): chip select pin
     */
    MC3419(uint8_t cs);   
     
    /*
     * Initialize the SPI port
     */
    void initialize();

    /*
     * Set the mode register.
     * All bits beyond bit 1 are not used in SPI mode
     * 
     * @param mode (uint8_t): Mode to set device to
     *                        Bits 0-1: Operational state
     *                                  00: Standby
     *                                  01: Wake
     *                                  10: Reserved
     *                                  11: Reserved
     */         
    void setDeviceMode(uint8_t mode);

    /*
     * Set the sample rate.
     * This is only valid for SPI speeds < 4MHz
     * 
     * @param samplerate (uint8_t): Sample rate to set
     *                              0x10: 25 Hz
     *                              0x11: 50 Hz
     *                              0x12: 62.5 Hz
     *                              0x13: 100 Hz
     *                              0x14: 125 Hz
     *                              0x15: 250 Hz
     *                              0x16: 500 Hz
     *                              0x17: 1000 Hz
     */
    void setSampleRate(uint8_t sampleRate);

    /*
     * Set range and scale of accelerometer
     * 
     * @param data (uint8_t): data to set
     *                        Bits 0-2: Low pass filter configuration
     *                                  000: Reserved
     *                                  001: Bandwidth setting 1, Fc = IDR/4.255
     *                                  010: Bandwidth setting 2, Fc = IDR/6
     *                                  011: Bandwidth setting 3, Fc = IDR/12
     *                                  100: Reserved
     *                                  101: Bandwidth setting 5, Fc = IDR/16
     *                                  110: Reserved
     *                                  111: Reserved
     *                        Bit 3:    Low pass filter enabled (1) or disabled (0)
     *                        Bits 4-6: Resolution range
     *                                  000: ± 2g
     *                                  001: ± 4g
     *                                  010: ± 6g
     *                                  011: ± 16g
     *                                  100: ± 12g
     *                                  101: Reserved
     *                                  110: Reserved
     *                                  111: Reserved
     *                        Bit 7:    Reserved
     */
    void setRange(uint8_t data);

    /*
     * Set the communication control register.
     * Bits 0-3 and 7 are reserved
     * 
     * @param data (uint8_t): Com control
     *                        Bit 4: Swap interrupt pin functionality (no swap: 0, swap: 1)
     *                        Bit 5: Enable (1) or disable (0) SPI 3-wire mode. Not supported in this library
     *                        Bit 6: Enable (1) or disable (1) clearing of individual interrupt flags. Not supported in this library
     */
    void setComControl(uint8_t data);

    /*
     * Set GPIO control register
     * Bits 0, 1, 4 and 5 are reserved
     * 
     * @param data (uint8_t): GPIO control
     *                        Bit 2: Set polarity of INT1 output. Active low (0) or active high (1)
     *                        Bit 3: Select between open dran (0) or push/pull (1) mode of INT1 output
     *                        Bit 6: Set polarity of INT2 output. Active low (0) or active high (1)
     *                        Bit 7: Select between open dran (0) or push/pull (1) mode of INT2 output
     */
    void setGPIOControl(uint8_t data);

    /*
     * Set motion control register. Enables flags and interrupts for motion detection features
     * 
     * @param data (uint8_t): features to enable
     *                        bit 0: Tilt & flip enable
     *                        bit 1: Latch outputs (disabled: 0, enabled: 1)
     *                        bit 2: Anymotion enable
     *                        bit 3: Shake enable
     *                        bit 4: Tilt35 enable
     *                        bit 5: Z axis orientation. Positive through top (0) or bottom (1) of package
     *                        bit 6: Enable (0) or disable (1) filtering of motion data
     *                        bit 7: Set (1) or clear (0) motion block reset
     */
    void setMotionControl(uint8_t data);

    /*
     * Reset the motion block.
     * Sets and then clears bit 7 of the motion control register
     */
    void resetMotionControl();

    /*
     * Get motion status data
     * Motion detection must be enabled using setMotionControl() for these flags to be active
     * 
     * @returns status (uint8_t): data in status register
     *                            bit 0: Tilt flag
     *                            bit 1: Flip flag
     *                            bit 2: Anymotion flag
     *                            bit 3: Shake flag
     *                            bit 4: Tilt35 flag
     *                            bit 5: FIFO flag
     *                            bit 6: Reserved
     *                            bit 7: New data flag
     */
    uint8_t getStatus();

    /*
     * Configure what interrupts to enable
     * Motion detection must be enabled using setMotionControl()
     * 
     * @param data (uint8_t):     interrupts to enable
     *                            bit 0: Tilt interrupt
     *                            bit 1: Flip interrupt
     *                            bit 2: Anymotion interrupt
     *                            bit 3: Shake interrupt
     *                            bit 4: Tilt35 interrupt
     *                            bit 5: Reserved
     *                            bit 6: Auto clear
     *                            bit 7: Acquisition interrupt
     */
    void setInterrupt(uint8_t data);

    /*
     * Get the interrupt status. 
     * For interrupts to trigger, motion detection must be enabled using setMotionControl() and interrupts must be enabled using setInterrupt().
     * 
     * @returns interruptStatus (uint8_t):  data in the interrupt status register
     *                                      bit 0: Tilt interrupt
     *                                      bit 1: Flip interrupt
     *                                      bit 2: Anymotion interrupt
     *                                      bit 3: Shake interrupt
     *                                      bit 4: Tilt35 interrupt
     *                                      bit 5: FIFO interrupt
     *                                      bit 6: Auto clear
     *                                      bit 7: Acquisition interrupt
     */
    uint8_t getInterruptStatus();

    /*
     * Clears all interrupts in the interrupt status register
     */
    void clearInterrupts();

    /*
     * Gets the acceleration in x-direction
     * 
     * @returns acceleration (int16_t): acceleration in x direction
     */
    int16_t getX();

    /*
     * Gets the acceleration in y-direction
     * 
     * @returns acceleration (int16_t): acceleration in y direction
     */
    int16_t getY();

    /*
     * Gets the acceleration in x-direction
     * 
     * @returns acceleration (int16_t): acceleration in x direction
     */
    int16_t getZ();

    /*
     * Sets the threshold value for the tilt/flip/tilt-35 functionality.
     * Tilt and/or flip must be enabled in the motion control register using setMotionControl().
     * 
     * Threshold value is 15 bit.
     * Acceleration values greater than the threshold value will result in a tilt condition.
     * Acceleration values smaller than the treshold value will result in a flat/flip condition, depending on the value of the z-axis
     * 
     * @param threshold (uint16_t):  Threshold value
     */
    void setTiltThreshold(uint16_t threshold);

    /*
     * Sets the tilt/flip debounce duration.
     * Each consecutive time a tilt/flip condition is detected, a counter is incremented. If this counter exceeds the duration value, the tilt/flip interrupt is set.
     * 
     * @param duration (uint8_t): Duration
     */
    void setTiltDebounce(uint8_t duration);

    /*
    void setAnymotionThreshold(uint16_t threshold);
    void setAnymotionDebounce(uint8_t duration);
    void setShakeThreshold(uint16_t threshold);
    void setShakePeakToPeakDuration(uint16_t data);
    void setShakeDuration(uint8_t duration);
    */
    
  private:
    /*
     * Writes data to one or more registers
     * 
     * @param address (uint8_t):      register address to write to
     * @param data (uint16_t):        data to write
     * @param bytesToSend (uint8_t):  number of bytes to send (1 or 2)
     */
    void writeRegister(uint8_t address, uint16_t data, uint8_t bytesToSend = 1);

    /*
     * Read data from one or more registers
     * 
     * @param address (uint8_t):      register address to read from
     * @param bytesToRead (uint8_t):  number of bytes to read (1 or 2)
     * @returns data (uint16_t):      data that has been read
     */
    uint16_t readRegister(uint8_t address, uint8_t bytesToRead = 1);

    /*
     * Private variables
     */
    uint8_t _cs;                            //Chip select pin
    uint8_t _motionCtrlReg = 0;             //Motion control register
    uint8_t _shakeDuration = 0;             //Share duration
    uint16_t _shakePeakToPeakDuration = 0;  //Peak to peak setting for shake functionality
};

#endif
