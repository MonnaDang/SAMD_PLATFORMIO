#include <Arduino.h>
#include "I2CDriver.h"
#include "AD5593R.h"
#include "PWM.h"

PWMChannel tc5Isr(pwmConfigs[6]);

volatile uint16_t _tc5Count = 0;
volatile bool _tc5isrFlag = false;
volatile uint16_t _tc5CountTop = 25;

AD5593R myADC;

void setup()
{
  // put your setup code here, to run once:
  SerialUSB.begin(9600);
  delay(5000);

  Peltier3.init();
  Peltier3.start();

  tc5Isr.init();
  tc5Isr.start();

  myADC.begin();
  pinMode(13, OUTPUT);
}

uint16_t pwmCount = 0;

void loop()
{
  // put your main code here, to run repeatedly:
  uint16_t adc,adc1;

  // digitalWrite(13, !digitalRead(13));
  adc = myADC.readAdc(4);
  adc1 = myADC.readAdc(5);
  // pwmCount += 10;
  // pwmCount %= 1010;
  Peltier3.setCompare1(200);


  SerialUSB.print("ADC value: ");
  SerialUSB.println(adc);

  SerialUSB.print("ADC1 value: ");
  SerialUSB.println(adc1);

  delay(300);
}

void TC5_Handler()
{
  TcCount16 *TC = (TcCount16 *)TC5; // get timer struct
  if (TC->INTFLAG.bit.OVF == 1)
  {                          // A overflow caused the interrupt
    TC->INTFLAG.bit.OVF = 1; // writing a one clears the flag ovf flag

    digitalWrite(13, !digitalRead(13));


  }
}
