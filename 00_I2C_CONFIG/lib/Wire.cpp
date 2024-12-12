/*
* Creator: DChuong
* Date: 11/19/2024
* Modify from Wire.cpp to match Swift-2 Hardware
*/

extern "C" {
#include <string.h>
}

#include "Wire.h"

using namespace arduino;

I2C_I2cConfig i2cConfigs[] = {
  // ---SDA/PAD[0]---||---SCL/PAD[1]---||
  { {17, PIO_SERCOM_ALT}, {18, PIO_SERCOM_ALT}  , &sercom0},          // SER0 - SDA: PA04 SCL: PA05
  { {11, PIO_SERCOM},     {13, PIO_SERCOM}      , &sercom1},          // SER1 - SDA: PA16 SCL: PA17
  { {4,  PIO_SERCOM_ALT}, {3,  PIO_SERCOM_ALT}  , &sercom2},          // SER2 - SDA: PA08 SCL: PA09
  { {20, PIO_SERCOM},     {21, PIO_SERCOM}      , &sercom3},          // SER3 - SDA: PA22 SCL: PA23
};

TwoWire::TwoWire(I2C_I2cConfig I2cConfig)
{
  this->sercom = I2cConfig.sercomBase;
  this->_SDA_pin=I2cConfig.SDA_pin;
  this->_SCL_pin=I2cConfig.SCL_pin;
  transmissionBegun = false;
}

void TwoWire::begin(void) {
  //Master Mode
  sercom->initMasterWIRE(TWI_CLOCK);
  sercom->enableWIRE();

  pinPeripheral(_SDA_pin.pinNumber, _SDA_pin.pinType);
  pinPeripheral(_SCL_pin.pinNumber, _SCL_pin.pinType);
}

void TwoWire::begin(uint8_t address, bool enableGeneralCall) {
  //Slave mode
  sercom->initSlaveWIRE(address, enableGeneralCall);
  sercom->enableWIRE();

  pinPeripheral(_SDA_pin.pinNumber, _SDA_pin.pinType);
  pinPeripheral(_SCL_pin.pinNumber, _SCL_pin.pinType);
}

void TwoWire::setClock(uint32_t baudrate) {
  sercom->disableWIRE();
  sercom->initMasterWIRE(baudrate);
  sercom->enableWIRE();
}

void TwoWire::end() {
  sercom->disableWIRE();
}

size_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool stopBit)
{
  if(quantity == 0)
  {
    return 0;
  }

  size_t byteRead = 0;

  rxBuffer.clear();

  if(sercom->startTransmissionWIRE(address, WIRE_READ_FLAG))
  {
    // Read first data
    rxBuffer.store_char(sercom->readDataWIRE());

    bool busOwner;
    // Connected to slave
    for (byteRead = 1; byteRead < quantity && (busOwner = sercom->isBusOwnerWIRE()); ++byteRead)
    {
      sercom->prepareAckBitWIRE();                          // Prepare Acknowledge
      sercom->prepareCommandBitsWire(WIRE_MASTER_ACT_READ); // Prepare the ACK command for the slave
      rxBuffer.store_char(sercom->readDataWIRE());          // Read data and send the ACK
    }
    sercom->prepareNackBitWIRE();                           // Prepare NACK to stop slave transmission
    //sercom->readDataWIRE();                               // Clear data register to send NACK

    if (stopBit && busOwner)
    {
      sercom->prepareCommandBitsWire(WIRE_MASTER_ACT_STOP);   // Send Stop unless arbitration was lost
    }

    if (!busOwner)
    {
      byteRead--;   // because last read byte was garbage/invalid
    }
  }

  return byteRead;
}

size_t TwoWire::requestFrom(uint8_t address, size_t quantity)
{
  return requestFrom(address, quantity, true);
}

void TwoWire::beginTransmission(uint8_t address) {
  // save address of target and clear buffer
  txAddress = address;
  txBuffer.clear();

  transmissionBegun = true;
}

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
uint8_t TwoWire::endTransmission(bool stopBit)
{
  transmissionBegun = false ;

  // Start I2C transmission
  if ( !sercom->startTransmissionWIRE( txAddress, WIRE_WRITE_FLAG ) )
  {
    sercom->prepareCommandBitsWire(WIRE_MASTER_ACT_STOP);
    return 2 ;  // Address error
  }

  // Send all buffer
  while( txBuffer.available() )
  {
    // Trying to send data
    if ( !sercom->sendDataMasterWIRE( txBuffer.read_char() ) )
    {
      sercom->prepareCommandBitsWire(WIRE_MASTER_ACT_STOP);
      return 3 ;  // Nack or error
    }
  }
  
  if (stopBit)
  {
    sercom->prepareCommandBitsWire(WIRE_MASTER_ACT_STOP);
  }   

  return 0;
}

uint8_t TwoWire::endTransmission()
{
  return endTransmission(true);
}

size_t TwoWire::write(uint8_t ucData)
{
  // No writing, without begun transmission or a full buffer
  if ( !transmissionBegun || txBuffer.isFull() )
  {
    return 0 ;
  }

  txBuffer.store_char( ucData ) ;

  return 1 ;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  //Try to store all data
  for(size_t i = 0; i < quantity; ++i)
  {
    //Return the number of data stored, when the buffer is full (if write return 0)
    if(!write(data[i]))
      return i;
  }

  //All data stored
  return quantity;
}

int TwoWire::available(void)
{
  return rxBuffer.available();
}

int TwoWire::read(void)
{
  return rxBuffer.read_char();
}

int TwoWire::peek(void)
{
  return rxBuffer.peek();
}

void TwoWire::flush(void)
{
  // Do nothing, use endTransmission(..) to force
  // data transfer.
}

void TwoWire::onReceive(void(*function)(int))
{
  onReceiveCallback = function;
}

void TwoWire::onRequest(void(*function)(void))
{
  onRequestCallback = function;
}

void TwoWire::onService(void)
{
  if ( sercom->isSlaveWIRE() )
  {
    if(sercom->isStopDetectedWIRE() || 
        (sercom->isAddressMatch() && sercom->isRestartDetectedWIRE() && !sercom->isMasterReadOperationWIRE())) //Stop or Restart detected
    {
      sercom->prepareAckBitWIRE();
      sercom->prepareCommandBitsWire(0x03);

      //Calling onReceiveCallback, if exists
      if(onReceiveCallback)
      {
        onReceiveCallback(available());
      }
      
      rxBuffer.clear();
    }
    else if(sercom->isAddressMatch())  //Address Match
    {
      sercom->prepareAckBitWIRE();
      sercom->prepareCommandBitsWire(0x03);

      if(sercom->isMasterReadOperationWIRE()) //Is a request ?
      {
        txBuffer.clear();

        transmissionBegun = true;

        //Calling onRequestCallback, if exists
        if(onRequestCallback)
        {
          onRequestCallback();
        }
      }
    }
    else if(sercom->isDataReadyWIRE())
    {
      if (sercom->isMasterReadOperationWIRE())
      {
        uint8_t c = 0xff;

        if( txBuffer.available() ) {
          c = txBuffer.read_char();
        }

        transmissionBegun = sercom->sendDataSlaveWIRE(c);
      } else { //Received data
        if (rxBuffer.isFull()) {
          sercom->prepareNackBitWIRE(); 
        } else {
          //Store data
          rxBuffer.store_char(sercom->readDataWIRE());

          sercom->prepareAckBitWIRE(); 
        }

        sercom->prepareCommandBitsWire(0x03);
      }
    }
  }
}

arduino::TwoWire Wire(i2cConfigs[0]);

void WIRE_IT_HANDLER(void) {
    Wire.onService();
}

arduino::TwoWire Wire1(i2cConfigs[1]);

void WIRE1_IT_HANDLER(void) {
    Wire1.onService();
}

arduino::TwoWire Wire2(i2cConfigs[2]);

void WIRE2_IT_HANDLER(void) {
    Wire2.onService();
} 

arduino::TwoWire Wire3(i2cConfigs[3]);

void WIRE3_IT_HANDLER(void) {
    Wire3.onService();
}