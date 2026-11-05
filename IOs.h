/*
 * File:   IOs.h
 * Author: Basema Khan, Michelle Yoon, Vincent Fong
 *
 * Created on November 4, 2026
 */

#ifndef IOS_H
#define IOS_H

#include <xc.h>
#include <stdint.h>

extern volatile uint16_t TMR2flag; // set by the timer 2 interrupt

void IOinit(void);
void IOcheck(void);

#endif // IOS_H
