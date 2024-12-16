#include "AD5593R_W.h"

AD5593R::AD5593R(uint8_t address, TwoWire *wire)
    : _i2cAddress(address), _wire(wire) {}

void AD5593R::begin() {
    _wire->begin();
    enableInternalVref();
    setMax2xVref();
    configureAdc(_ADC_DEFAULT_CHANNEL);
    configureDdc(_DAC_DEFAULT_CHANNEL);
}

void AD5593R::enableInternalVref() {
    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_POWER_REF_CTRL);
    _wire->write(_ADAC_VREF_ON);
    _wire->write(0x00);
    _wire->endTransmission();
}

void AD5593R::setMax2xVref() {
    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_GP_CONTROL);
    _wire->write(0x20|0x10);
    _wire->write(0x00);
    _wire->endTransmission();
}

void AD5593R::setMax1xVref() {
    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_GP_CONTROL);
    _wire->write(0x00);
    _wire->write(0x00);
    _wire->endTransmission();
}

void AD5593R::configureAdc(uint8_t channels) {

    if (channels & dacChannels) {      // check to see if the channel is a DAC already
        return;
    }

    adcChannels = channels;

    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_ADC_CONFIG);
    _wire->write(0x00);
    _wire->write(adcChannels);
    _wire->endTransmission();
}

void AD5593R::configureDdc(uint8_t channels) {

    if (channels & adcChannels) {       // check to see if the channel is a ADC already
        return;
    }

    dacChannels = channels;

    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_DAC_CONFIG);
    _wire->write(0x00);
    _wire->write(dacChannels);
    _wire->endTransmission();
}

uint16_t AD5593R::readAdc(uint8_t channel) {
    _wire->setClock(400000);
    // STEP1: Config GPIO as ADCs channel, Should be done in "begin" function
    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_ADC_SEQUENCE);       // STEP2: Write ADC sequence register
    _wire->write(0x02);                     //        Set REP bit
    _wire->write(1 << channel);             // STEP3: Select ADC channel
    _wire->endTransmission();

    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_ADC_READ);           // STEP4: Begin read ADC value
    _wire->endTransmission();

    uint8_t buff[2] = {0,0};
    if(_wire->requestFrom((int)_i2cAddress, 2, 1))
      _wire->readBytes(buff, 2);

    _wire->setClock(100000);
    return (buff[0] & 0x0F) << 8 | buff[1];
}

void AD5593R::readAdcs(uint8_t channels, uint16_t *values) {
    _wire->setClock(400000);

    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_ADC_SEQUENCE);
    _wire->write(0x02);
    _wire->write(channels);
    _wire->endTransmission();

    _wire->beginTransmission(_i2cAddress);
    _wire->write(_ADAC_ADC_READ);
    _wire->endTransmission();

    uint8_t buff[8] = {0}; // Buffer for 4 ADC channels (2 bytes each)
    _wire->requestFrom((int)_i2cAddress, 8, 1);
    _wire->readBytes(buff, 8);

    // Parse the buffer into 16-bit values
    values[0] = (buff[0] & 0x0F) << 8 | buff[1];
    values[1] = (buff[2] & 0x0F) << 8 | buff[3];
    values[2] = (buff[4] & 0x0F) << 8 | buff[5];
    values[3] = (buff[6] & 0x0F) << 8 | buff[7];

    _wire->setClock(100000);
}


void AD5593R::writeDac(uint8_t channel, uint16_t value){

    // extract the 4 most signifigant bits, and move them down to the bottom
    byte data_msbs = (value & 0xf00) >> 8;
    byte lsbs = (value & 0x0ff);
    // place the channel data in the most signifigant bits
    byte msbs = (0b10000000 | (channel << 4)) | data_msbs;

    _wire->beginTransmission(_i2cAddress);
    _wire->write((_ADAC_DAC_WRITE | channel));
    _wire->write(msbs);
    _wire->write(lsbs);
    _wire->endTransmission();
}