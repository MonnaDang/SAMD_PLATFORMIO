/*
* Creator: DChuong
* Date: 11/19/2024
* Modify from Wire.cpp to match Swift-2 Hardware
*/

/*
 * TWI/I2C library for Arduino Zero
 * Copyright (c) 2015 Arduino LLC. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef TwoWire_h
#define TwoWire_h

#include <stdint.h>
#include <Arduino.h>
#include "api/HardwareI2C.h"
#include "variant.h"
#include "SERCOM.h"
#include <wiring_private.h>

 // WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END 1

// Pin struct for SAMD21
typedef struct {
  uint16_t pinNumber;
  EPioType pinType;
}I2C_Pin;

typedef struct {
  const I2C_Pin SDA_pin;
  const I2C_Pin SCL_pin;
  SERCOM* sercomBase;
} I2C_I2cConfig;

extern I2C_I2cConfig i2cConfigs[];


namespace arduino {

class TwoWire : public HardwareI2C
{
  public:
    // TwoWire(SERCOM *s, I2C_Pin SDA_pin, I2C_Pin SCL_pin);
    TwoWire(I2C_I2cConfig I2cConfig);
    void begin();
    void begin(uint8_t address, bool enableGeneralCall);
    void begin(uint8_t address) {
        begin(address, false);
    }
    void end();
    void setClock(uint32_t);

    void beginTransmission(uint8_t);
    uint8_t endTransmission(bool stopBit);
    uint8_t endTransmission(void);

    size_t requestFrom(uint8_t address, size_t quantity, bool stopBit);
    size_t requestFrom(uint8_t address, size_t quantity);

    size_t write(uint8_t data);
    size_t write(const uint8_t * data, size_t quantity);

    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
    virtual void flush(void);
    void onReceive(void(*)(int));
    void onRequest(void(*)(void));

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write;

    void onService(void);

  private:
    SERCOM * sercom;
    I2C_Pin _SDA_pin;
    I2C_Pin _SCL_pin;

    bool transmissionBegun;

    // RX Buffer
    arduino::RingBufferN<256> rxBuffer;

    //TX buffer
    arduino::RingBufferN<256> txBuffer;
    uint8_t txAddress;

    // Callback user functions
    void (*onRequestCallback)(void);
    void (*onReceiveCallback)(int);

    // TWI clock frequency
    static const uint32_t TWI_CLOCK = 100000;
};

}


extern arduino::TwoWire Wire;
extern arduino::TwoWire Wire1;
extern arduino::TwoWire Wire2;
extern arduino::TwoWire Wire3;

#endif