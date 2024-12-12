#include <Arduino.h>
#include "I2CDriver.h"
#include "AD5593R.h"
#include "PWM.h"
#include "PI.h"
#include "Filter.h"

PWMChannel tc5Isr(pwmConfigs[6]);
volatile uint16_t _tc5Count = 0;
volatile bool _tc5isrFlag = false;
volatile uint16_t _tc5CountTop = 25;

#define BUFFER_SIZE 64
char inputBuffer[BUFFER_SIZE]; // Buffer for incoming data
int bufferIndex;               // Current index in the buffer

#define GAINV 7.48

#define CLOSED_LOOP
// #define OPEN_LOOP

const float LP_COEFFICIENT[] = {0.140775364680379, 0.140775364680379, -0.718449270639242};
volatile bool isControlFlag = false;

#define CONTROL_VOLTAGE
#ifdef CONTROL_VOLTAGE
FIRST_ORDER_FILTER VoltageFilter = {{0, 0}, {0, 0}};
#define KP 0.001
#define KI 1
#define TS 0.0109226667 // 91.55Hz
#define D_MAX 0.6       // limit by duty cycle, D_MAX = 0.7 -> Vo_max = Vin * D_MAX = 24 * 0.6 = 14.4 V
#define D_MIN 0
#endif

// Buck Controller
volatile PI_VAL BUCK_PI = PI_DEFAULT;

AD5593R myAD5593R(0x11, &SAMDWire3);
uint16_t sen_adc = 0;
float sen_actual = 0;
float Vset = 0;
float duty_cycle = 0;

void handleCommand(const char *command, const char *args);
void printAllInfo();

void setup()
{
  // put your setup code here, to run once:

  SerialUSB.begin(9600);

  Peltier3.init();
  Peltier3.start();
  Peltier3.setCompare1(0);

  if (!myAD5593R.begin())
  {
    while (1)
    {
      SerialUSB.println("ERROR");
      delay(100);
    }
  }

  // Init PI Controller
#ifdef CONTROL_VOLTAGE
  BUCK_PI.Kp = KP;
  BUCK_PI.Ki = KI;
  BUCK_PI.Ts = TS;
  BUCK_PI.OutMax = D_MAX;
  BUCK_PI.OutMin = D_MIN;
#endif

  tc5Isr.init();
  tc5Isr.start();
  pinMode(13, OUTPUT);
}

unsigned long _last_millis = 0;

void loop()
{
  // put your main code here, to run repeatedly:
  if (millis() - _last_millis > 400)
  {
    _last_millis = millis();
    SerialUSB.print("Voltage: ");
    SerialUSB.println(sen_actual);
  }

  while (SerialUSB.available() > 0)
  {
    char receivedChar = SerialUSB.read();

    // Check for command termination character (e.g., newline)
    if (receivedChar == '\n' || receivedChar == '\r')
    {
      inputBuffer[bufferIndex] = '\0'; // Null-terminate the string
      bufferIndex = 0;                 // Reset the buffer index

      // Split into command and arguments
      char *command = strtok(inputBuffer, ":");
      char *args = strtok(NULL, ""); // Get the rest of the string as arguments

      // Handle the command
      if (command != NULL)
      {
        handleCommand(command, args);
      }
    }
    else if (bufferIndex < BUFFER_SIZE - 1)
    {
      // Add character to the buffer if there's space
      inputBuffer[bufferIndex++] = receivedChar;
    }
  }
}

void TC5_Handler()
{
  TcCount16 *TC = (TcCount16 *)TC5; // get timer struct
  if (TC->INTFLAG.bit.OVF == 1)
  {                          // A overflow caused the interrupt
    TC->INTFLAG.bit.OVF = 1; // writing a one clears the flag ovf flag
    digitalWrite(13, !digitalRead(13));
    sen_adc = myAD5593R.readAdc(5);
    // sen_actual = sen_adc / 4095 * 2.5 * GAINV; 
    sen_actual = sen_adc*0.00456654; 

    Compute_1st_Filter(&VoltageFilter, sen_actual, LP_COEFFICIENT);

#ifdef OPEN_LOOP
    Peltier3.setCompare1(duty_cycle * pwmConfigs[3].topValue);
#endif

#ifdef CLOSED_LOOP
    if (isControlFlag)
    {
      BUCK_PI.Err[0] = Vset - sen_actual;
      Compute_PI(&BUCK_PI);

      Peltier3.setCompare1(BUCK_PI.Out * pwmConfigs[3].topValue);
    }else{
      Peltier3.setCompare1(0);
      // Reset the pi controller
      BUCK_PI.Err[0] = 0;
      BUCK_PI.Err[1] = 0;
      BUCK_PI.Up = 0;
      BUCK_PI.Ui = 0;
      BUCK_PI.Out = 0;
    }
#endif

    digitalWrite(13, !digitalRead(13));
  }
}

void handleCommand(const char *command, const char *args)
{
  if (strcmp(command, "SET") == 0)
  {
    if (args != NULL)
    {
      // Create a mutable copy of args
      char argsCopy[BUFFER_SIZE];
      strncpy(argsCopy, args, BUFFER_SIZE - 1);
      argsCopy[BUFFER_SIZE - 1] = '\0'; // Ensure null termination

      char *variable = strtok(argsCopy, ",");
      char *valueStr = strtok(NULL, ",");
      if (variable != NULL && valueStr != NULL)
      {
        float value = atof(valueStr);
        if (strcmp(variable, "VSET") == 0)
        {
          Vset = ((value > 12) ? 12 : value);
          SerialUSB.print("VSET updated to: ");
          SerialUSB.println(Vset);
        }
        else if (strcmp(variable, "DUTY") == 0)
        {
          duty_cycle = ((value > 0.6) ? 0.6 : value);
          SerialUSB.print("Duty cycle updated to: ");
          SerialUSB.println(duty_cycle);
        }
        else
        {
          SerialUSB.print("Unknown variable: ");
          SerialUSB.println(variable);
        }
      }
      else if (variable != NULL)
      {
        if (strcmp(variable, "START") == 0)
        {
          isControlFlag = true;
          SerialUSB.print("CONTROL updated to: ");
          SerialUSB.println(isControlFlag);
        }

        if (strcmp(variable, "STOP") == 0)
        {
          isControlFlag = false;
          SerialUSB.print("CONTROL updated to: ");
          SerialUSB.println(isControlFlag);
        }
      }
      else
      {
        SerialUSB.println("Invalid SET syntax. Use SET:<var>,<value>.");
      }
    }
  }
  else if (strcmp(command, "GET") == 0)
  {
    if (args != NULL)
    {
      if (strcmp(args, "VSET") == 0)
      {
        SerialUSB.print("VSET is: ");
        SerialUSB.println(Vset);
      }
      else if (strcmp(args, "ADC") == 0)
      {
        SerialUSB.print("ADC value is: ");
        SerialUSB.println(sen_adc);
      }
      else if (strcmp(args, "ACTUAL") == 0)
      {
        SerialUSB.print("Sensor actual value is: ");
        SerialUSB.println(sen_actual);
      }
      else if (strcmp(args, "DUTY") == 0)
      {
        SerialUSB.print("Duty cycle is: ");
        SerialUSB.println(duty_cycle);
      }
      else if (strcmp(args, "FILTER") == 0)
      {
        SerialUSB.print("Filtered sensor value is: ");
        SerialUSB.println(VoltageFilter.y[0]);
      }
      else
      {
        SerialUSB.print("Unknown variable: ");
        SerialUSB.println(args);
      }
    }
  }
  else if (strcmp(command, "PRINT") == 0)
  {
    printAllInfo();
  }
  else
  {
    SerialUSB.println("Unknown command.");
  }
}

void printAllInfo()
{
  SerialUSB.println("===== System Parameters =====");
  SerialUSB.print("CONTROL: ");
  SerialUSB.println(isControlFlag);

  SerialUSB.print("VSET: ");
  SerialUSB.println(Vset);

  SerialUSB.print("ADC Value: ");
  SerialUSB.println(sen_adc);

  SerialUSB.print("Sensor Actual Value: ");
  SerialUSB.println(sen_actual);

  SerialUSB.print("Duty Cycle: ");
  SerialUSB.println(duty_cycle);

  SerialUSB.print("Filtered Sensor Value: ");
  SerialUSB.println(VoltageFilter.y[0]);
  SerialUSB.println("=============================");
}