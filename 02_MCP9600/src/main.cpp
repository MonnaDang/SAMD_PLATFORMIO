#include <Arduino.h>
#include "I2CDriver.h"
#include "MCP9600.h"
#include "AD5593R.h"

MCP9600 mcp_bot(&SAMDWire3, 0x60);
MCP9600 mcp_mid(&SAMDWire3, 0x67);

/* Set and print ambient resolution */
Ambient_Resolution ambientRes = RES_ZERO_POINT_0625;

AD5593R myAD5593R(0x11, &SAMDWire3);

void setup()
{
  SerialUSB.begin(9600);
  delay(5000);
  while (!Serial)
  {
    delay(10);
  }
  SerialUSB.println("MCP9600 HW test");

  if (!myAD5593R.begin())
  {
    while (1)
    {
      SerialUSB.println("ERROR");
      delay(100);
    }
  }

  // TEM SENSOR
  SAMDWire3.setClock(100000);
  if (!mcp_bot.begin() && !mcp_mid.begin())
  {
    while (1)
    {
      SerialUSB.println("Sensor not found. Check wiring!");
    }
  }
  /* Set and print ambient resolution */
  mcp_bot.setAmbientResolution(ambientRes);
  mcp_mid.setAmbientResolution(ambientRes);

  mcp_bot.setADCresolution(MCP9600_ADCRESOLUTION_18);
  mcp_mid.setADCresolution(MCP9600_ADCRESOLUTION_18);

  mcp_bot.setThermocoupleType(MCP9600_TYPE_J);
  mcp_mid.setThermocoupleType(MCP9600_TYPE_J);

  mcp_bot.setFilterCoefficient(1);
  mcp_mid.setFilterCoefficient(1);
  SAMDWire3.setClock(400000);
  // // TEM SENSOR

  // mcp.enable(true);

  SerialUSB.println(F("------------------------------"));
  pinMode(13, OUTPUT);
}
uint8_t count = 0;

void loop()
{
  digitalWrite(13, !digitalRead(13));
  SAMDWire3.setClock(100000);
  SerialUSB.print("Mid: ");
  SerialUSB.println(mcp_mid.readThermocouple());
  SerialUSB.print("Bot: ");
  SerialUSB.println(mcp_bot.readThermocouple());
  SerialUSB.print("Mid: ");
  SerialUSB.print(mcp_mid.readADC() * 2);
  SerialUSB.println(" uV");
  SerialUSB.print("bot: ");
  SerialUSB.print(mcp_bot.readADC() * 2);
  SerialUSB.println(" uV");
  SAMDWire3.setClock(400000);

  SerialUSB.print("Voltage: "); SerialUSB.println(myAD5593R.readAdc(5));

  delay(200);
}