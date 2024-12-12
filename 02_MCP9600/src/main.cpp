#include <Arduino.h>
#include "I2CDriver.h"
#include "MCP9600.h"

MCP9600 mcp(&SAMDWire3,0x67);

/* Set and print ambient resolution */
Ambient_Resolution ambientRes = RES_ZERO_POINT_0625;

void setup() {
  SerialUSB.begin(9600);
  delay(5000);
  while (!Serial) {
    delay(10);
  }
  SerialUSB.println("MCP9600 HW test");

  /* Initialise the driver with I2C_ADDRESS and the default I2C bus. */
  if (!mcp.begin()) {
    SerialUSB.println("Sensor not found. Check wiring!");
    // while (1)
    //   ;
  }

  SerialUSB.println("Found MCP9600!");

  /* Set and print ambient resolution */
  mcp.setAmbientResolution(ambientRes);
  SerialUSB.print("Ambient Resolution set to: ");
  switch (ambientRes) {
    case RES_ZERO_POINT_25: SerialUSB.println("0.25°C"); break;
    case RES_ZERO_POINT_0625: SerialUSB.println("0.0625°C"); break;
  }

  mcp.setADCresolution(MCP9600_ADCRESOLUTION_18);
  SerialUSB.print("ADC resolution set to ");
  switch (mcp.getADCresolution()) {
    case MCP9600_ADCRESOLUTION_18: SerialUSB.print("18"); break;
    case MCP9600_ADCRESOLUTION_16: SerialUSB.print("16"); break;
    case MCP9600_ADCRESOLUTION_14: SerialUSB.print("14"); break;
    case MCP9600_ADCRESOLUTION_12: SerialUSB.print("12"); break;
  }
  SerialUSB.println(" bits");

  mcp.setThermocoupleType(MCP9600_TYPE_J);
  SerialUSB.print("Thermocouple type set to ");
  switch (mcp.getThermocoupleType()) {
    case MCP9600_TYPE_K: SerialUSB.print("K"); break;
    case MCP9600_TYPE_J: SerialUSB.print("J"); break;
    case MCP9600_TYPE_T: SerialUSB.print("T"); break;
    case MCP9600_TYPE_N: SerialUSB.print("N"); break;
    case MCP9600_TYPE_S: SerialUSB.print("S"); break;
    case MCP9600_TYPE_E: SerialUSB.print("E"); break;
    case MCP9600_TYPE_B: SerialUSB.print("B"); break;
    case MCP9600_TYPE_R: SerialUSB.print("R"); break;
  }
  SerialUSB.println(" type");

  mcp.setFilterCoefficient(3);
  SerialUSB.print("Filter coefficient value set to: ");
  SerialUSB.println(mcp.getFilterCoefficient());

  // mcp.enable(true);

  SerialUSB.println(F("------------------------------"));
  pinMode(13, OUTPUT);
}
uint8_t count = 0;


void loop() {
  digitalWrite(13,!digitalRead(13));
  SerialUSB.print("Hot Junction: "); SerialUSB.println(mcp.readThermocouple());
  SerialUSB.print("Cold Junction: "); SerialUSB.println(mcp.readAmbient());
  SerialUSB.print("ADC: "); SerialUSB.print(mcp.readADC() * 2); SerialUSB.println(" uV");

  delay(200);
}