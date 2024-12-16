/*
Creator: DChuong
Date: 11/19/2024

Description:
    Modify to match Hardware of Swift-2

Source Library: https://github.com/LukasJanavicius/AD5593R-Arduino-ESP32-Library/tree/master
*/

#ifndef _AD5593R_H_
#define _AD5593R_H_

#include <Arduino.h>
#include <stdint.h>
#include "I2CDriver.h"

// Definitions
#define _ADAC_NULL 0b00000000
#define _ADAC_ADC_SEQUENCE 0b00000010   // ADC sequence register - Selects ADCs for conversion
#define _ADAC_GP_CONTROL 0b00000011     // General-purpose control register - DAC and ADC control register
#define _ADAC_ADC_CONFIG 0b00000100     // ADC pin configuration - Selects which pins are ADC inputs
#define _ADAC_DAC_CONFIG 0b00000101     // DAC pin configuration - Selects which pins are DAC outputs
#define _ADAC_PULL_DOWN 0b00000110      // Pull-down configuration - Selects which pins have an 85 kO pull-down resistor to GND
#define _ADAC_LDAC_MODE 0b00000111      // LDAC mode - Selects the operation of the load DAC
#define _ADAC_GPIO_WR_CONFIG 0b00001000 // GPIO write configuration - Selects which pins are general-purpose outputs
#define _ADAC_GPIO_WR_DATA 0b00001001   // GPIO write data - Writes data to general-purpose outputs
#define _ADAC_GPIO_RD_CONFIG 0b00001010 // GPIO read configuration - Selects which pins are general-purpose inputs
#define _ADAC_POWER_REF_CTRL 0b00001011 // Power-down/reference control - Powers down the DACs and enables/disables the reference
#define _ADAC_OPEN_DRAIN_CFG 0b00001100 // Open-drain configuration - Selects open-drain or push-pull for general-purpose outputs
#define _ADAC_THREE_STATE 0b00001101    // Three-state pins - Selects which pins are three-stated
#define _ADAC_RESERVED 0b00001110       // Reserved
#define _ADAC_SOFT_RESET 0b00001111     // Software reset - Resets the AD5593R

/**
 * @name     ADAC Configuration Data Bytes
 ******************************************************************************/
///@{
// write into MSB after _ADAC_POWER_REF_CTRL command to enable VREF
#define _ADAC_VREF_ON 0b00000010
#define _ADAC_SEQUENCE_ON 0b00000010

/**
 * @name   ADAC Write / Read Pointer Bytes
 ******************************************************************************/
///@{
#define _ADAC_DAC_WRITE 0b00010000
#define _ADAC_ADC_READ 0b01000000
#define _ADAC_DAC_READ 0b01010000
#define _ADAC_GPIO_READ 0b01110000
#define _ADAC_REG_READ 0b01100000


#define _ADC_DEFAULT_CHANNEL 0xF0     
#define _DAC_DEFAULT_CHANNEL 0x0F

class AD5593R {
public:
    AD5593R(uint8_t address = 0x11, I2CDriver *_mI2C = &SAMDWire3);

    // Start the I2C sercom and config defaut AD5593R
    bool begin(void);
    
    // Enable Internal Vref = 2.5V, based on Schematic of Swif-2
    bool enableInternalVref(void);

    // Set Max Vref to 2xVref to measure full scale of signal
    bool setMax2xVref();
    bool setMax1xVref();

    // Config ADC channels, set to 1 to enable ADC (each bit)
    bool configureAdc(uint8_t channels);
    
    // Config DAC channels, set to 1 to enable DAC (each bit)
    bool configureDdc(uint8_t channels);

    // Read single ADC channel
    uint16_t readAdc(uint8_t channel);

    // Write signle DAC channel
    bool writeDac(uint8_t channel, uint16_t value);

    // Read multiple ADC channel (for default read all 4-adc channel)
    bool readAdcs(uint8_t channels, uint16_t *values);
    
private:
    uint8_t _i2cAddress;
    I2CDriver* _mI2C;

    uint8_t adcChannels;
    uint8_t dacChannels;
};

#endif