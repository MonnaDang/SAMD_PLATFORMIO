/*
 * Filter.h
 *
 *  Created on: Jan 28, 2024
 *      Author: CHUONG
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <math.h>
#define PIx2 6.28318530718

typedef struct{
    float y[3],x[3];
}SECOND_ORDER_FILTER;

static inline void Compute_2nd_Filter(volatile SECOND_ORDER_FILTER *Filter, float x_k,const float *FILTER_COEFF) {

    Filter->x[0] = x_k;         //x(k)
    // Compute y(k) = B0*x(k) + B1*x(k-1) + B2*x(k-2) - A1*y(k-1) - A2*y(k-2)
    Filter->y[0] =  FILTER_COEFF[0] * Filter->x[0]        //B0*x(k)
                 +  FILTER_COEFF[1] * Filter->x[1]        //B1*x(k-1)
                 +  FILTER_COEFF[2] * Filter->x[2]        //B2*x(k-2)
                 -  FILTER_COEFF[3] * Filter->y[1]        //A1*y(k-1)
                 -  FILTER_COEFF[4] * Filter->y[2];       //A2*y(k-2)

    // Store values for next iteration
    Filter->x[2] = Filter->x[1];
    Filter->x[1] = Filter->x[0];
    Filter->y[2] = Filter->y[1];
    Filter->y[1] = Filter->y[0];
}

typedef struct{
    float y[2],x[2];
}FIRST_ORDER_FILTER;

static inline void Compute_1st_Filter(volatile FIRST_ORDER_FILTER *Filter, float x_k,const float *FILTER_COEFF) {

    Filter->x[0] = x_k;         //x(k)
    // Compute y(k) =  B0*x(k) + B1*x(k-1) - A1*y(k-1)
    Filter->y[0] =  FILTER_COEFF[0] * Filter->x[0]        //B0*x(k)
                 +  FILTER_COEFF[1] * Filter->x[1]        //B1*x(k-1)
                 -  FILTER_COEFF[2] * Filter->y[1];       //A1*y(k-1)

    // Store values for next iteration
    Filter->x[1] = Filter->x[0];
    Filter->y[1] = Filter->y[0];
}

static inline void Integrator_mod_2_pi(volatile FIRST_ORDER_FILTER *Filter, float x_k,const float *FILTER_COEFF) {

    Filter->x[0] = x_k;         //x(k)
    // Compute y(k)
    Filter->y[0] =  FILTER_COEFF[0] * Filter->x[0]        //B0*x(k)
                 +  FILTER_COEFF[1] * Filter->x[1]        //B1*x(k-1)
                 -  FILTER_COEFF[2] * Filter->y[1];       //A1*y(k-1)
    Filter->y[0] = fmod(Filter->y[0],PIx2);

    // Store values for next iteration
    Filter->x[1] = Filter->x[0];
    Filter->y[1] = Filter->y[0];
}


#endif /* FILTER_H_ */