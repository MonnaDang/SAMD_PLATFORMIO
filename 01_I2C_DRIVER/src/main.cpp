#include <Arduino.h>
#include "I2CDriver.h"

I2CDriver myI2C(i2cConfigs[3]);

void setup() {
  // put your setup code here, to run once:
  myI2C.init();
  SerialUSB.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int nDevices = 0;

  SerialUSB.println("Scanning...");

  for (byte address = 1; address < 127; ++address) {
    myI2C.beginTranmission(address);
    bool isACK = myI2C.write_start();

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
  myI2C.endTranmission();
  if (nDevices == 0) {
    SerialUSB.println("No I2C devices found\n");
  } else {
    SerialUSB.println("done\n");
  }
  delay(500); // Wait 5 seconds for next scan
}
