#include "AD5593R.h"

AD5593R::AD5593R(uint8_t address, I2CDriver *_mwire)
    : _i2cAddress(address), _mI2C(_mwire) {}

bool AD5593R::begin()
{
    bool isACK = false;
    _mI2C->init();
    _mI2C->setClock(400000);
    isACK = enableInternalVref();
    isACK &= setMax2xVref();
    isACK &= configureAdc(_ADC_DEFAULT_CHANNEL);
    isACK &= configureDdc(_DAC_DEFAULT_CHANNEL);
    return isACK;
}

bool AD5593R::enableInternalVref()
{
    bool isACK = false;
    _mI2C->beginTransmission(_i2cAddress);
    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_POWER_REF_CTRL);
        _mI2C->write_byte(_ADAC_VREF_ON);
        _mI2C->write_byte(0x00);
    }
    _mI2C->endTransmission();

    return isACK;
}

bool AD5593R::setMax2xVref()
{
    bool isACK = false;
    _mI2C->beginTransmission(_i2cAddress);
    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_GP_CONTROL);
        _mI2C->write_byte(0x30);
        _mI2C->write_byte(0x00);
    }
    _mI2C->endTransmission();

    return isACK;
}

bool AD5593R::setMax1xVref()
{
    bool isACK = false;
    _mI2C->beginTransmission(_i2cAddress);
    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_GP_CONTROL);
        _mI2C->write_byte(0x00);
        _mI2C->write_byte(0x00);
    }
    _mI2C->endTransmission();

    return isACK;
}

bool AD5593R::configureAdc(uint8_t channels)
{

    if (channels & dacChannels)
    { // check to see if the channel is a DAC already
        return false;
    }

    adcChannels = channels;

    bool isACK = false;
    _mI2C->beginTransmission(_i2cAddress);
    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_ADC_CONFIG);
        _mI2C->write_byte(0x00);
        _mI2C->write_byte(adcChannels);
    }
    _mI2C->endTransmission();

    return isACK;
}

bool AD5593R::configureDdc(uint8_t channels)
{

    if (channels & adcChannels)
    { // check to see if the channel is a ADC already
        return false;
    }

    dacChannels = channels;

    bool isACK = false;
    _mI2C->beginTransmission(_i2cAddress);
    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_DAC_CONFIG);
        _mI2C->write_byte(0x00);
        _mI2C->write_byte(dacChannels);
    }
    _mI2C->endTransmission();

    return isACK;
}

uint16_t AD5593R::readAdc(uint8_t channel)
{
    bool isACK = false;
    uint8_t buff[2] = {0, 0};

    // _mI2C->setClock(400000);
    _mI2C->beginTransmission(_i2cAddress);
    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_ADC_SEQUENCE); // STEP2: Write ADC sequence register
        _mI2C->write_byte(0x02);               //        Set REP bit
        _mI2C->write_byte(1 << channel);       // STEP3: Select ADC channel
    }
    _mI2C->endTransmission();

    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_ADC_READ); // STEP4: Begin read ADC value
    }
    _mI2C->endTransmission();

    _mI2C->read(buff, 2); // Read ADC
    // _mI2C->setClock(100000);

    return (buff[0] & 0x0F) << 8 | buff[1];
}

bool AD5593R::readAdcs(uint8_t channels, uint16_t *values)
{
    bool isACK = false;
    uint8_t buff[8] = {0};

    _mI2C->beginTransmission(_i2cAddress);
    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_ADC_SEQUENCE); // STEP2: Write ADC sequence register
        _mI2C->write_byte(0x02);               //        Set REP bit
        _mI2C->write_byte(channels);           // STEP3: Select ADC channel
    }
    _mI2C->endTransmission();

    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_ADC_READ); // STEP4: Begin read ADC value
    }
    _mI2C->endTransmission();

    _mI2C->read(buff, 8); // Read ADC

    // Parse the buffer into 16-bit values
    values[0] = (buff[0] & 0x0F) << 8 | buff[1];
    values[1] = (buff[2] & 0x0F) << 8 | buff[3];
    values[2] = (buff[4] & 0x0F) << 8 | buff[5];
    values[3] = (buff[6] & 0x0F) << 8 | buff[7];

    return isACK;
}

bool AD5593R::writeDac(uint8_t channel, uint16_t value)
{

    bool isACK = false;
    // extract the 4 most signifigant bits, and move them down to the bottom
    byte data_msbs = (value & 0xf00) >> 8;
    byte lsbs = (value & 0x0ff);
    // place the channel data in the most signifigant bits
    byte msbs = (0b10000000 | (channel << 4)) | data_msbs;

    _mI2C->beginTransmission(_i2cAddress);
    isACK = _mI2C->write_start();
    if (isACK)
    {
        _mI2C->write_byte(_ADAC_ADC_SEQUENCE);
        _mI2C->write_byte((_ADAC_DAC_WRITE | channel));
        _mI2C->write_byte(msbs);
        _mI2C->write_byte(lsbs);
    }
    _mI2C->endTransmission();

    return isACK;
}