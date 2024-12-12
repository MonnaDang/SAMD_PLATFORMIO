
#ifndef __I2C_DRIVER_H__
#define __I2C_DRIVER_H__

#include <Arduino.h>
#include "wiring_private.h"

// Pin struct for SAMD21
typedef struct {
  uint16_t pinNumber;
  EPioType pinType;
}I2C_Pin;

typedef struct {
  const I2C_Pin SDA_pin;
  const I2C_Pin SCL_pin;
  Sercom* sercomBase;
  uint16_t sercomModule_ID;
  uint16_t sercomPM_mask;
} I2C_I2cConfig;

typedef struct {
    uint8_t address;
    uint8_t* data;
    uint8_t data_length;
} I2C_Packet;

enum
{
  I2C_TRANSFER_WRITE = 0,
  I2C_TRANSFER_READ  = 1,
};

extern I2C_I2cConfig i2cConfigs[];

class I2CDriver
{
private:
    /* data */
    I2C_I2cConfig _mI2c;
    uint32_t _mFSCL = 100000; // Default Baudrate 
    uint8_t _maddr;

    uint32_t calculateBaud(uint32_t fgclk, uint32_t fscl);
public:
    I2CDriver(I2C_I2cConfig I2cConfig);

    void init(void);
    void beginTransmission(uint8_t i2c_adress);

    bool write_start(void);
    bool write_byte(uint8_t byte);
    bool write(uint8_t *data, int size);

    bool read(uint8_t *data, int size);

    void setClock(uint32_t fscl);
    bool isBusy(void);

    void endTransmission(void);

    void disableI2C(void);
    void enableI2C(void);
};

extern I2CDriver SAMDWire;
extern I2CDriver SAMDWire1;
extern I2CDriver SAMDWire2;
extern I2CDriver SAMDWire3;

#endif