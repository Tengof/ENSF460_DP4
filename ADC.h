/*
 * File:   ADC.h
 * Author: Basema Khan, Michelle Yoon, Vincent Fong
 *
 * Created on November 4, 2026
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void ADCinit(void);
uint16_t do_ADC(void);      // Performs one ADC conversion and returns the digital reading

#endif  
