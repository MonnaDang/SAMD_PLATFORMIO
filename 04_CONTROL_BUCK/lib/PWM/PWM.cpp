#include "PWM.h"

// PWM configuration table
PWM_PwmConfig pwmConfigs[] = {
    // -------- PIN1 -------- || -------- PIN2 -------- || --- TIMER BASE --- || --- DIVIDER --- || --- TOP ---
    {},                                                                                                               // No-define
    {{P1_PWM1, PIO_TIMER, 1, true}, {P1_PWM2, PIO_TIMER, 0, true}, (TcCount8 *)TC3, TC_CTRLA_PRESCALER_DIV8, TC_PER}, // Peltier 1
    {{P2_PWM1, PIO_TIMER_ALT, 1, true}, {P2_PWM2, PIO_TIMER_ALT, 2, true}, TCC0, TCC_CTRLA_PRESCALER_DIV1, TCC_PER},  // Peltier 2
    {{P3_PWM1, PIO_TIMER, 0, true}, {P3_PWM2, PIO_TIMER, 1, true}, TCC1, TCC_CTRLA_PRESCALER_DIV1, TCC_PER},          // Peltier 3
    {{BUZZ_PWM, PIO_TIMER, 1, true}, {0, PIO_TIMER, 0, false}, TCC2, TCC_CTRLA_PRESCALER_DIV16, BUZZER_PER},          // Buzzer
    {{SOL_PWM, PIO_TIMER, 1, true}, {0, PIO_TIMER, 0, false}, (TcCount8 *)TC4, TC_CTRLA_PRESCALER_DIV256, SOLRE_PER}, // Solid Relay
    {{0, PIO_TIMER, 1, false}, {0, PIO_TIMER, 0, false}, (TcCount16 *)TC5, TC_CTRLA_PRESCALER_DIV8, 0},             // Interrupt
};

const Tc *tcValid[3] = {TC3, TC4, TC5};      // Valid Timer/Counter modules
const Tcc *tccValid[3] = {TCC0, TCC1, TCC2}; // Valid Timer/Counter Control modules

static bool config_clock = false;

// Constructor
PWMChannel::PWMChannel(PWM_PwmConfig pwmConfig)
    : pin1(pwmConfig.pin1), pin2(pwmConfig.pin2),
      timerBase(pwmConfig.timerBase),
      divider(pwmConfig.divider),
      topValue(pwmConfig.topValue)
{
    // Validate TC/TCC module
    bool found = false;
    for (int i = 0; i < 3; i++)
    {
        if (timerBase == tcValid[i])
        {
            isTc = true;
            found = true;
            break;
        }
        else if (timerBase == tccValid[i])
        {
            isTc = false;
            found = true;
            break;
        }
    }

    // Error handling for invalid modules
    if (!found)
    {
        while (true)
            ;
    }

    // Configure clock if not already enabled
    if (!config_clock)
    {
        // Enable GCLK0 for TCC0 and TCC1
        REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN |
                                      GCLK_CLKCTRL_GEN_GCLK0 |
                                      GCLK_CLKCTRL_ID(GCM_TCC0_TCC1));
        while (GCLK->STATUS.bit.SYNCBUSY)
            ;

        // Enable GCLK0 for TCC2 and TC3
        REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN |
                                      GCLK_CLKCTRL_GEN_GCLK0 |
                                      GCLK_CLKCTRL_ID(GCM_TCC2_TC3));
        while (GCLK->STATUS.bit.SYNCBUSY)
            ;

        // Enable GCLK0 for TC4  TC5
        REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN |
                                      GCLK_CLKCTRL_GEN_GCLK0 |
                                      GCLK_CLKCTRL_ID(GCM_TC4_TC5));
        while (GCLK->STATUS.bit.SYNCBUSY)
            ;

        config_clock = true;
    }
}

// Configure PWM pin
void PWMChannel::configurePin()
{
    if (pin1.isValid)
    {
        pinMode(pin1.pin, OUTPUT);
        digitalWrite(pin1.pin, LOW);
        pinPeripheral(pin1.pin, pin1.pinType); // Set pin to use timer
    }

    if (pin2.isValid)
    {
        pinMode(pin2.pin, OUTPUT);
        digitalWrite(pin2.pin, LOW);
        pinPeripheral(pin2.pin, pin2.pinType); // Set pin to use timer
    }
}

// Configure timer
void PWMChannel::configureTimer()
{
    if (isTc && timerBase != TC5)
    { // Timer/Counter Module
        TcCount8 *TC = (TcCount8 *)timerBase;

        TC->CTRLA.reg &= ~TC_CTRLA_ENABLE; // Disable timer
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT8; // Set to 8-bit mode
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_NPWM; // Set PWM mode
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        TC->CTRLA.reg |= divider; // Set prescaler
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        TC->PER.reg = topValue; // Set top value
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        if (pin1.isValid)
            TC->CC[pin1.compareReg].reg = 0;
        if (pin2.isValid)
            TC->CC[pin2.compareReg].reg = 0;
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        TC->INTENSET.reg = 0; // Disable interrupts
    }
    else
    { // Timer/Counter Control Module
        Tcc *TC = (Tcc *)timerBase;

        TC->CTRLA.reg &= ~TCC_CTRLA_ENABLE; // Disable timer
        while (TC->SYNCBUSY.bit.ENABLE)
            ;

        TC->CTRLA.reg |= divider; // Set prescaler
        TC->WAVE.reg |= TCC_WAVE_WAVEGEN_DSTOP | TCC_POLARITY_MASK(pin1.compareReg) | TCC_POLARITY_MASK(pin2.compareReg);
        while (TC->SYNCBUSY.bit.WAVE)
            ;

        TC->PER.reg = topValue; // Set top value
        while (TC->SYNCBUSY.bit.PER)
            ;

        if (pin1.isValid)
        {
            TC->CC[pin1.compareReg].reg = 0;
            while ((TC->SYNCBUSY.reg & TCC_COMPARE_SYN_MASK(pin1.compareReg)))
                ;
        }

        if (pin2.isValid)
        {
            TC->CC[pin2.compareReg].reg = 0;
            while ((TC->SYNCBUSY.reg & TCC_COMPARE_SYN_MASK(pin2.compareReg)))
                ;
        }

        TC->INTENSET.reg = 0; // Disable interrupts
    }

    if (timerBase == TC5)
    {

        TcCount16 *TC = (TcCount16 *)timerBase;

        TC->CTRLA.reg &= ~TC_CTRLA_ENABLE; // Disable timer
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16; // Set to 8-bit mode
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_NFRQ; // Set PWM mode
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV8; // Set prescaler
        while (TC->STATUS.bit.SYNCBUSY)
            ;

        // Interrupts
        TC->INTENSET.reg = 0;     // disable all interrupts
        TC->INTENSET.bit.OVF = 1; // enable overfollow

        // Enable InterruptVector
        NVIC_EnableIRQ(TC5_IRQn);
    }
}

// Initialize PWM channel
void PWMChannel::init()
{
    configurePin();
    configureTimer();
}

// Start PWM
void PWMChannel::start()
{
    if (isTc)
    {
        TcCount8 *TC = (TcCount8 *)timerBase;
        TC->CTRLA.reg |= TC_CTRLA_ENABLE;
        while (TC->STATUS.bit.SYNCBUSY)
            ;
    }
    else
    {
        Tcc *TC = (Tcc *)timerBase;
        TC->CTRLA.reg |= TCC_CTRLA_ENABLE;
        while (TC->SYNCBUSY.bit.ENABLE)
            ;
    }
}

// Stop PWM
void PWMChannel::stop()
{
    if (isTc)
    {
        TcCount8 *TC = (TcCount8 *)timerBase;
        TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
        while (TC->STATUS.bit.SYNCBUSY)
            ;
    }
    else
    {
        Tcc *TC = (Tcc *)timerBase;
        TC->CTRLA.reg &= ~TCC_CTRLA_ENABLE;
        while (TC->SYNCBUSY.bit.ENABLE)
            ;
    }
}

// Set compare value for pin 1
void PWMChannel::setCompare1(uint16_t pin1CmpValue)
{
    if (isTc && pin1.isValid)
    {
        TcCount8 *TC = (TcCount8 *)timerBase;
        TC->CC[pin1.compareReg].reg = pin1CmpValue;
        while (TC->STATUS.bit.SYNCBUSY)
            ;
    }
    else if (pin1.isValid)
    {
        Tcc *TC = (Tcc *)timerBase;
        TC->CC[pin1.compareReg].reg = pin1CmpValue;
        while ((TC->SYNCBUSY.reg & TCC_COMPARE_SYN_MASK(pin1.compareReg)))
            ;
    }
}

// Set compare value for pin 2
void PWMChannel::setCompare2(uint16_t pin2CmpValue)
{
    if (isTc && pin2.isValid)
    {
        TcCount8 *TC = (TcCount8 *)timerBase;
        TC->CC[pin2.compareReg].reg = pin2CmpValue;
        while (TC->STATUS.bit.SYNCBUSY)
            ;
    }
    else if (pin2.isValid)
    {
        Tcc *TC = (Tcc *)timerBase;
        TC->CC[pin2.compareReg].reg = pin2CmpValue;
        while ((TC->SYNCBUSY.reg & TCC_COMPARE_SYN_MASK(pin2.compareReg)))
            ;
    }
}

// PWM channel instances
PWMChannel Peltier1(pwmConfigs[1]);
PWMChannel Peltier2(pwmConfigs[2]);
PWMChannel Peltier3(pwmConfigs[3]);
PWMChannel Buzz(pwmConfigs[4]);
PWMChannel SOL(pwmConfigs[5]);
