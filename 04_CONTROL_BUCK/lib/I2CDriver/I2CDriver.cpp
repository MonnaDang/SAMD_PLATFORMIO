
#include "I2CDriver.h"

I2C_I2cConfig i2cConfigs[] = {
    //-----SDA/PAD[0]-----||-----SCL/PAD[1]----||---SERCOM---||-----GCLK_ID------||-----PM_MASK------||
    {{17, PIO_SERCOM_ALT}, {18, PIO_SERCOM_ALT}, SERCOM0, SERCOM0_GCLK_ID_CORE, PM_APBCMASK_SERCOM0}, // SER0 - SDA: PA04 SCL: PA05
    {{11, PIO_SERCOM}, {13, PIO_SERCOM}, SERCOM1, SERCOM1_GCLK_ID_CORE, PM_APBCMASK_SERCOM1},         // SER1 - SDA: PA16 SCL: PA17
    {{4, PIO_SERCOM_ALT}, {3, PIO_SERCOM_ALT}, SERCOM2, SERCOM2_GCLK_ID_CORE, PM_APBCMASK_SERCOM2},   // SER2 - SDA: PA08 SCL: PA09
    {{20, PIO_SERCOM}, {21, PIO_SERCOM}, SERCOM3, SERCOM3_GCLK_ID_CORE, PM_APBCMASK_SERCOM3},         // SER3 - SDA: PA22 SCL: PA23
};

// Constructor definition
I2CDriver::I2CDriver(I2C_I2cConfig I2cConfig) : _mI2c(I2cConfig)
{
}

void I2CDriver::init()
{
    // set pin to I2C mode
    pinPeripheral(_mI2c.SCL_pin.pinNumber, _mI2c.SCL_pin.pinType);
    pinPeripheral(_mI2c.SDA_pin.pinNumber, _mI2c.SDA_pin.pinType);

    // Enable power
    PM->APBCMASK.reg |= _mI2c.sercomPM_mask;

    // Select source clk (GLCK0 - 48M)
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(_mI2c.sercomModule_ID) |
                        GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;

    // Enable smart mode
    _mI2c.sercomBase->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;
    while (_mI2c.sercomBase->I2CM.SYNCBUSY.reg)
        ;

    // Calulate baudrate
    uint32_t baudReate = calculateBaud(SystemCoreClock, _mFSCL);

    // Set Baudrate
    _mI2c.sercomBase->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(baudReate);
    while (_mI2c.sercomBase->I2CM.SYNCBUSY.reg)
        ;

    // Config I2C master mode
    _mI2c.sercomBase->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_ENABLE |
                                       SERCOM_I2CM_CTRLA_MODE_I2C_MASTER |
                                       SERCOM_I2CM_CTRLA_SDAHOLD(3);
    while (_mI2c.sercomBase->I2CM.SYNCBUSY.reg)
        ;

    // Force bus state to IDLE
    _mI2c.sercomBase->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
    while (_mI2c.sercomBase->I2CM.SYNCBUSY.reg)
        ;

    // _mI2c.sercomBase->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;
}

void I2CDriver::beginTransmission(uint8_t i2c_adress)
{
    // Assign i2c address
    _maddr = i2c_adress;
}

bool I2CDriver::write_start()
{
    // send write cmmd
    _mI2c.sercomBase->I2CM.ADDR.reg = (_maddr << 1) | I2C_TRANSFER_WRITE;
    while (0 == (_mI2c.sercomBase->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB))
        ;

    // Check NACK
    if (_mI2c.sercomBase->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
    {
        _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
        return false;
    }

    return true;
}

bool I2CDriver::write_byte(uint8_t byte)
{
    // Load data
    _mI2c.sercomBase->I2CM.DATA.reg = byte;
    while (0 == (_mI2c.sercomBase->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB))
        ;

    // Check ACK
    if (_mI2c.sercomBase->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
    {
        _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
        return false;
    }

    return true;
}

bool I2CDriver::write(uint8_t *data, int size)
{
    // prepare ack
    _mI2c.sercomBase->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
    // Send write cmd
    _mI2c.sercomBase->I2CM.ADDR.reg = (_maddr << 1) | I2C_TRANSFER_WRITE;
    // Wait transmission successful
    while (!_mI2c.sercomBase->I2CM.INTFLAG.bit.MB)
    {
        // If a bus error occurs, the MB bit may never be set.
        // Check the bus error bit and ARBLOST bit and bail if either is set.
        if (_mI2c.sercomBase->I2CM.STATUS.bit.BUSERR || _mI2c.sercomBase->I2CM.STATUS.bit.ARBLOST)
        {
            return false;
        }
    }

    if (_mI2c.sercomBase->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
    {
        _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
        return false;
    }

    // Write byte
    for (int i = 0; i < size; i++)
    {
        _mI2c.sercomBase->I2CM.DATA.reg = data[i];
        // Wait transmission successful
        while (!_mI2c.sercomBase->I2CM.INTFLAG.bit.MB)
        {
            // If a bus error occurs, the MB bit may never be set.
            // Check the bus error bit and ARBLOST bit and bail if either is set.
            if (_mI2c.sercomBase->I2CM.STATUS.bit.BUSERR || _mI2c.sercomBase->I2CM.STATUS.bit.ARBLOST)
            {
                return false;
            }
        }

        if (_mI2c.sercomBase->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
        {
            _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
            return false;
        }
    }

    // Send stop cmd
    _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);

    return true;
}

bool I2CDriver::read(uint8_t *data, int size)
{
    // prepare ack
    _mI2c.sercomBase->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
    // Send read cmd
    _mI2c.sercomBase->I2CM.ADDR.reg = (_maddr << 1) | I2C_TRANSFER_READ;
    while ((_mI2c.sercomBase->I2CM.INTFLAG.bit.SB == 0) && (_mI2c.sercomBase->I2CM.INTFLAG.bit.MB == 0))
    {
        // Waiting complete receive
    }

    // Check NACK
    if (_mI2c.sercomBase->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
    {
        _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
        return false;
    }

    _mI2c.sercomBase->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

    for (int i = 0; i < size - 1; i++)
    {
        data[i] = _mI2c.sercomBase->I2CM.DATA.reg;
        while (_mI2c.sercomBase->I2CM.INTFLAG.bit.SB == 0 && _mI2c.sercomBase->I2CM.INTFLAG.bit.MB == 0)
        {
            // Waiting complete receive
        }
    }

    if (size)
    {
        _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT; // send NACK after send last byte
        _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
        data[size - 1] = _mI2c.sercomBase->I2CM.DATA.reg;
    }

    return true;
}

bool I2CDriver::isBusy()
{
    bool busy;
    _mI2c.sercomBase->I2CM.ADDR.reg = (_maddr << 1) | I2C_TRANSFER_WRITE;
    while (0 == (_mI2c.sercomBase->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB))
        ;

    busy = (0 != (_mI2c.sercomBase->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK));

    _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);

    return busy;
}

void I2CDriver::endTransmission()
{
    _mI2c.sercomBase->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
}

void I2CDriver::setClock(uint32_t fscl)
{
    disableI2C();
    _mFSCL = fscl;

    // init();

    // Enable smart mode
    _mI2c.sercomBase->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;
    while (_mI2c.sercomBase->I2CM.SYNCBUSY.reg)
        ;

    // Calulate baudrate
    uint32_t baudReate = calculateBaud(SystemCoreClock, _mFSCL);

    // Set Baudrate
    _mI2c.sercomBase->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(baudReate);
    while (_mI2c.sercomBase->I2CM.SYNCBUSY.reg)
        ;

    // Config I2C master mode
    _mI2c.sercomBase->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_ENABLE |
                                       SERCOM_I2CM_CTRLA_MODE_I2C_MASTER |
                                       SERCOM_I2CM_CTRLA_SDAHOLD(3);
    while (_mI2c.sercomBase->I2CM.SYNCBUSY.reg)
        ;

    // Force bus state to IDLE
    _mI2c.sercomBase->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
    while (_mI2c.sercomBase->I2CM.SYNCBUSY.reg)
        ;
    enableI2C();
}

uint32_t I2CDriver::calculateBaud(uint32_t fgclk, uint32_t fscl)
{
    float f_temp, f_baud;
    f_temp = ((float)fgclk / (float)fscl) - (((float)fgclk / (float)1000000) * 0.125);
    f_baud = (f_temp / 2) - 5;
    return ((uint32_t)f_baud);
}

void I2CDriver::disableI2C()
{
    // Enable the I2C master mode
    _mI2c.sercomBase->I2CM.CTRLA.bit.ENABLE = 0;

    while (_mI2c.sercomBase->I2CM.SYNCBUSY.bit.ENABLE != 0)
    {
        // Waiting the enable bit from SYNCBUSY is equal to 0;
    }
}
void I2CDriver::enableI2C()
{
    // Enable the I2C master mode
    _mI2c.sercomBase->I2CM.CTRLA.bit.ENABLE = 1;

    while (_mI2c.sercomBase->I2CM.SYNCBUSY.bit.ENABLE != 0)
    {
        // Waiting the enable bit from SYNCBUSY is equal to 0;
    }

    // Setting bus idle mode
    _mI2c.sercomBase->I2CM.STATUS.bit.BUSSTATE = 1;

    while (_mI2c.sercomBase->I2CM.SYNCBUSY.bit.SYSOP != 0)
    {
        // Wait the SYSOP bit from SYNCBUSY coming back to 0
    }
}

I2CDriver SAMDWire(i2cConfigs[0]);
I2CDriver SAMDWire1(i2cConfigs[1]);
I2CDriver SAMDWire2(i2cConfigs[2]);
I2CDriver SAMDWire3(i2cConfigs[3]);
