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
  SERCOM* sercomBase;
} I2C_I2cConfig;

I2C_I2cConfig i2cConfigs[] = {
  //-----SDA/PAD[0]-----||-----SCL/PAD[1]----||---SERCOM---||
  { {17, PIO_SERCOM_ALT}, {18, PIO_SERCOM_ALT}  , &sercom0},          // SER0 - SDA: PA04 SCL: PA05
  { {11, PIO_SERCOM},     {13, PIO_SERCOM}      , &sercom1},          // SER1 - SDA: PA16 SCL: PA17
  { {4,  PIO_SERCOM_ALT}, {3,  PIO_SERCOM_ALT}  , &sercom2},          // SER2 - SDA: PA08 SCL: PA09
  { {20, PIO_SERCOM},     {21, PIO_SERCOM}      , &sercom3},          // SER3 - SDA: PA22 SCL: PA23
};

/*- Definitions -------------------------------------------------------------*/
#define STANDARD_MODE_FAST_MODE  0x0 
#define FAST_MODE_PLUS           0X01 
#define HIGHSPEED_MODE           0X02 
#define I2C_SERCOM            SERCOM3
#define I2C_SERCOM_GCLK_ID    SERCOM3_GCLK_ID_CORE
#define I2C_SERCOM_CLK_GEN    0
#define I2C_SERCOM_APBCMASK   PM_APBCMASK_SERCOM3

#define I2C_ADDRESS           0xa0

enum
{
  I2C_TRANSFER_WRITE = 0,
  I2C_TRANSFER_READ  = 1,
};

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------

/* calculating the BAUD value using Fgclk,Fscl,Trise 
    FSCL =fGCLK / (10 + BAUD +BAUDLOW + fGCLKTRISE )*/ 
uint32_t calculate_baud(uint32_t fgclk, uint32_t fscl) 
{ 
float f_temp, f_baud; 
f_temp = ((float)fgclk/(float)fscl) - (((float)fgclk/(float)1000000)*0.3); 
f_baud = (f_temp/2)-5; 
return ((uint32_t)f_baud); 
} 

void i2c_init(void)
{

  pinPeripheral(i2cConfigs[3].SCL_pin.pinNumber, i2cConfigs[3].SCL_pin.pinType) ;
  pinPeripheral(i2cConfigs[3].SDA_pin.pinNumber, i2cConfigs[3].SDA_pin.pinType) ;

  PM->APBCMASK.reg |= I2C_SERCOM_APBCMASK;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(I2C_SERCOM_GCLK_ID) |
      GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(I2C_SERCOM_CLK_GEN);

  I2C_SERCOM->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;
  while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

  uint32_t baudReate = calculate_baud(SystemCoreClock,100000);

  I2C_SERCOM->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(baudReate);
  while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

  I2C_SERCOM->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_ENABLE |
                              SERCOM_I2CM_CTRLA_MODE_I2C_MASTER |
                              SERCOM_I2CM_CTRLA_SDAHOLD(3)|
                              SERCOM_I2CM_CTRLA_SPEED(STANDARD_MODE_FAST_MODE);
  while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

  I2C_SERCOM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
  while (I2C_SERCOM->I2CM.SYNCBUSY.reg);
}

//-----------------------------------------------------------------------------
bool i2c_write(uint8_t *data, int size)
{
  I2C_SERCOM->I2CM.ADDR.reg = I2C_ADDRESS | I2C_TRANSFER_WRITE;

  while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

  if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
  {
    I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
    return false;
  }

  for (int i = 0; i < size; i++)
  {
    I2C_SERCOM->I2CM.DATA.reg = data[i];

    while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

    if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
    {
      I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
      return false;
    }
  }

  I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);

  return true;
}

//-----------------------------------------------------------------------------
bool i2c_write_start(uint8_t i2c_addr)
{
  I2C_SERCOM->I2CM.ADDR.reg = (i2c_addr<<1) | I2C_TRANSFER_WRITE;

  while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

  if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
  {
    I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool i2c_write_byte(uint8_t byte)
{
  I2C_SERCOM->I2CM.DATA.reg = byte;

  while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

  if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
  {
    I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
void i2c_write_stop(void)
{
  I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
}

//-----------------------------------------------------------------------------
bool i2c_read(uint8_t *data, int size)
{
  I2C_SERCOM->I2CM.ADDR.reg = I2C_ADDRESS | I2C_TRANSFER_READ;

  while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB));

  if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
  {
    I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
    return false;
  }

  I2C_SERCOM->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

  for (int i = 0; i < size-1; i++)
  {
    data[i] = I2C_SERCOM->I2CM.DATA.reg;
    while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB));
  }

  if (size)
  {
    I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
    I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
    data[size-1] = I2C_SERCOM->I2CM.DATA.reg;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool i2c_busy(void)
{
  bool busy;

  I2C_SERCOM->I2CM.ADDR.reg = I2C_ADDRESS | I2C_TRANSFER_WRITE;

  while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

  busy = (0 != (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK));

  I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);

  return busy;
}

void setup() {
  // put your setup code here, to run once:
  i2c_init();
  pinMode(13,OUTPUT);
  SerialUSB.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int nDevices = 0;

  SerialUSB.println("Scanning...");

  for (byte address = 1; address < 127; ++address) {
    bool isACK = i2c_write_start(address);

    if (isACK == 1) {
      SerialUSB.print("I2C device found at address 0x");
      if (address < 16) {
        SerialUSB.print("0");
      }
      SerialUSB.print(address, HEX);
      SerialUSB.println("  !");

      ++nDevices;
    }
  }
  if (nDevices == 0) {
    SerialUSB.println("No I2C devices found\n");
  } else {
    SerialUSB.println("done\n");
  }
  delay(1000); // Wait 5 seconds for next scan
}
