
// #include "MCP9600.h"

// MCP9600::MCP9600(I2CDriver *i2c_dev, uint8_t dev_addr)
//     : _mI2C_dev(i2c_dev), _i2c_adrr(dev_addr)
// {
// }

// bool MCP9600::begin()
// {
//     bool isACK = false;
//     uint8_t devID_REV[2] = {0, 0};
//     _mI2C_dev->init();
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//     {
//         isACK = _mI2C_dev->write_byte(MCP9600_DEVICEID);
//     }
//     _mI2C_dev->endTranmission();

//     if (isACK)
//     {
//         isACK = _mI2C_dev->read(devID_REV, 2);
//     }

//     if (isACK && (devID_REV[0] == 0x40))
//     {
//         return true;
//     }
//     return false;
// }

// float MCP9600::readThermocouple(void)
// {
//     bool isACK = false;
//     uint8_t data[2] = {0, 0};
//     int16_t rawData = 0;

//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_HOTJUNCTION);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(data, 2);
//     rawData = (data[0] << 8) | data[1];

//     if ((data[0] & 0x80) == 0x80) // negative sign
//         rawData -= 0x10000;

//     return rawData * 0.0625;
// }

// float MCP9600::readAmbient(void)
// {
//     bool isACK = false;
//     uint8_t data[2] = {0, 0};
//     int16_t rawData = 0;

//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_COLDJUNCTION);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(data, 2);
//     rawData = (data[0] << 8) | data[1];

//     if ((data[0] & 0x80) == 0x80) // negative sign
//         rawData -= 0x10000;

//     return rawData * 0.0625;
// }

// MCP9600_ThemocoupleType MCP9600::getThermocoupleType(void)
// {
//     bool isACK = false;
//     uint8_t data;

//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_SENSORCONFIG);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(&data, 1);

//     // Get sensor type bit [6:4]
//     data = (data & 0x70) >> 4;

//     return (MCP9600_ThemocoupleType)data;
// }

// void MCP9600::setThermocoupleType(MCP9600_ThemocoupleType type)
// {
//     bool isACK = false;
//     uint8_t data = 0;
//     // read back sensor config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_SENSORCONFIG);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(&data, 1);

//     data = (data & ~0x70) | (type << 3);

//     // write sensor config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_SENSORCONFIG);
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(data);
//     _mI2C_dev->endTranmission();
// }

// uint8_t MCP9600::getFilterCoefficient(void)
// {
//     bool isACK = false;
//     uint8_t data;

//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_SENSORCONFIG);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(&data, 1);

//     // Get sensor type bit [2:0]
//     data = (data & 0x07);

//     return data;
// }

// void MCP9600::setFilterCoefficient(uint8_t fillCoeff)
// {
//     bool isACK = false;
//     uint8_t data = 0;
//     // read back sensor config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_SENSORCONFIG);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(&data, 1);

//     // Clear fillter bit[2:0] and set new fillter coeff
//     data = (data & ~0x07) | (fillCoeff & 0x07);

//     // write sensor config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_SENSORCONFIG);
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(data);
//     _mI2C_dev->endTranmission();
// }

// MCP9600_ADCResolution MCP9600::getADCresolution(void)
// {
//     bool isACK = false;
//     uint8_t data = 0;
//     // read back sensor config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_DEVICECONFIG);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(&data, 1);

//     // return adc resolution bit[6:5]
//     data = data & 0x60;

//     return (MCP9600_ADCResolution)data;
// }

// void MCP9600::setADCresolution(MCP9600_ADCResolution resolution)
// {
//     bool isACK = false;
//     uint8_t data = 0;
//     // read back device config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_DEVICECONFIG);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(&data, 1);

//     // Clear adc resolution bit[2:0] and set new adc resolution
//     data = (data & ~0x60) | (resolution << 5);

//     // write device config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_DEVICECONFIG);
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(data);
//     _mI2C_dev->endTranmission();
// }

// int32_t MCP9600::readADC(void)
// {
//     bool isACK = false;
//     uint8_t data[3] = {0, 0, 0};
//     int32_t rawData = 0;

//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_RAWDATAADC);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(data, 3);
//     rawData = (data[0] << 16) | (data[1] << 8) | data[2];

//     // extend 24 bits to 32
//     if ((data[0] & 0x80) == 0x80) // negative sign
//         rawData |= 0xFF000000;

//     return rawData;
// }

// void MCP9600::setAmbientResolution(Ambient_Resolution res_value)
// {
//     bool isACK = false;
//     uint8_t data = 0;
//     // read back device config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_DEVICECONFIG);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(&data, 1);

//     // Clear ambient resolution bit[2:0] and set new ambient resolution
//     data = (data & ~0x80) | (res_value << 7);

//     // write device config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_DEVICECONFIG);
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(data);
//     _mI2C_dev->endTranmission();
// }

// uint8_t MCP9600::getStatus(void)
// {
//     bool isACK = false;
//     uint8_t data = 0;
//     // read back device config register
//     _mI2C_dev->beginTranmission(_i2c_adrr);
//     isACK = _mI2C_dev->write_start();
//     if (isACK)
//         isACK = _mI2C_dev->write_byte(MCP9600_STATUS);
//     _mI2C_dev->endTranmission();
//     if (isACK)
//         isACK = _mI2C_dev->read(&data, 1);
//     return data;
// }