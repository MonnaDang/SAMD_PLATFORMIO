/*
 * PI.h
 *
 *  Created on: Apr 17, 2024
 *      Author: Chuong
 */

#ifndef PI_H_
#define PI_H_

#define PI_DEFAULT {{0, 0}, 0, 0, 0, 0, 0, 0, 0, 0}

typedef struct
{
    float Err[2]; // Error k and k-1
    float Up;     // Output proportional
    float Ui;     // Output integral
    float Kp;     // Proportional gain
    float Ki;     // Integral gain
    float Out;    // Output PI controller
    float Ts;     // Sampling time
    float OutMax; // Maximum output limit
    float OutMin; // Minimum output limit
} PI_VAL;

// Must update Err[0] before calling this function
static inline __attribute__((always_inline)) 
void Compute_PI(volatile PI_VAL *v)
{
    // Proportional term
    v->Up = v->Kp * v->Err[0];

    // Integral term with anti-windup check
    float Ui_new = v->Ki * v->Ts * (v->Err[0] + v->Err[1]) / 2 + v->Ui;

    // Apply anti-windup by checking output limits
    if (Ui_new + v->Up > v->OutMax)
    {
        v->Ui = v->OutMax - v->Up;
    }
    else if (Ui_new + v->Up < v->OutMin)
    {
        v->Ui = v->OutMin - v->Up;
    }
    else
    {
        v->Ui = Ui_new;
    }

    // Compute total output
    v->Out = v->Up + v->Ui;

    // Update the previous error
    v->Err[1] = v->Err[0];
}

static inline __attribute__((always_inline)) 
void Reset_PI(volatile PI_VAL *v)
{
    v->Err[0] = 0;
    v->Err[1] = 0;
    v->Ui = 0;
    v->Up = 0;
    v->Out = 0;
}

#endif /* PI_H_ */