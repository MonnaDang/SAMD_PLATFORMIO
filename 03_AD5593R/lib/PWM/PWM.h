#ifndef _PWM_H_
#define _PWM_H_

#include <stdint.h>
#include <Arduino.h>
#include "wiring_private.h"

#define P1_PWM1 12          // PA19 - TC3/WO[1]  - CC1
#define P1_PWM2 10          // PA18 - TC3/WO[0]  - CC0
#define P2_PWM1 5           // PA15 - TCC0/WO[5] - CC1
#define P2_PWM2 6           // PA20 - TCC0/WO[6] - CC2
#define P3_PWM1 1           // PA10 - TCC1/WO[0] - CC0
#define P3_PWM2 0           // PA11 - TCC1/WO[1] - CC1

#define BUZZ_PWM 38         // PA13 - TCC2/WO[1] - CC1
#define SOL_PWM  16         // PA09 - TC4/WO[1]  - CC1

#define TC_PER   250        // TOP value of TC3
#define TCC_PER  1000       // TOP value of TCC0,1

#define BUZZER_PER  1500    // TOP value of TCC2 - DIV16 -> 1kHz
#define SOLRE_PER   186     // TOP value of TC4  - DIV256-> 1kHz

#define TCC_POLARITY_MASK(compareReg) (1 << ((compareReg) + TCC_WAVE_POL0_Pos))
#define TCC_COMPARE_SYN_MASK(compareReg)  (1 << ((compareReg) + 8))

typedef struct {
    const uint8_t pin;
    const EPioType pinType;
    const uint8_t compareReg;
    bool isValid;
} PWM_Pin;

// Lookup Table for PwmConfig Mappings
typedef struct {
    PWM_Pin pin1,pin2;               // GPIO pin for PWM
    void* timerBase;                 // Pointer to the timer peripheral
    const uint32_t divider;                // Timer divider
    const uint16_t topValue;               // Top value of Timer
} PWM_PwmConfig;


// Predefined PwmConfig Mappings
extern PWM_PwmConfig pwmConfigs[];

class PWMChannel {
private:
    PWM_Pin pin1,pin2;               // GPIO pin for PWM
    void* timerBase;                 // Pointer to the timer peripheral
    uint32_t divider;                // prescaler value
    uint32_t topValue;               // Top value of Timer
    bool isTc;                       // Check TC or TCC module

    void configurePin();             // Configure the pin for PWM
    void configureTimer();           // Configure the timer registers

public:
    // Constructor
    PWMChannel(PWM_PwmConfig pwmConfig);

    // Initialization
    void init();

    // Start and Stop PWM
    void start();
    void stop();

    // Set Duty Cycle
    void setCompare1(uint16_t pin1CmpValue);
    void setCompare2(uint16_t pin2CmpValue);
};

extern PWMChannel Peltier1;
extern PWMChannel Peltier2;
extern PWMChannel Peltier3;

extern PWMChannel Buzz;
extern PWMChannel SOL;

#endif // PWM_H