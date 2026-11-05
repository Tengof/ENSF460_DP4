/*
 * File:   Timer.c
 * Author: Basema Khan, Michelle Yoon, Vincent Fong
 *
 * Created on November 4, 2026
 */

#include "Timer.h"

/* newClk(500) -> timers count at 250 kHz
 * Timer 2: pre-scaler 1:8   -> 1 count = 32 us
 * Timer 3: pre-scaler 1:256 -> 1 count = 1.024 ms
 */

void TimerInit(void) {
    
    //T2 configuration (100 ms interval for ADC readings)
    T2CONbits.T32 = 0;      // Operate Timer 2 as a 16-bit timer
    T2CONbits.TCKPS = 1;    // Set pre-scaler to 1:8
    T2CONbits.TCS = 0;      // Use internal clock
    T2CONbits.TSIDL = 0;    // Continue operating in idle mode
    
    IPC1bits.T2IP = 5;      // Set Timer 2 interrupt priority
    IFS0bits.T2IF = 0;      // Clear Timer 2 interrupt flag
    IEC0bits.T2IE = 1;      // Enable Timer 2 interrupt
    
    PR2 = 3125;             // Set Timer 2 period to ~100ms
    TMR2 = 0;               // Reset Timer 2 counter
    
    // Timer 2 is turned on in main()
    
    // Timer 3 not used in Part 1
    T3CONbits.TCKPS = 3;    // Set pre-scaler to 1:256
    T3CONbits.TCS = 0;      // Use internal clock
    T3CONbits.TSIDL = 0;    // Continue operating in idle mode
    
    IPC2bits.T3IP = 2;      // Set Timer 3 interrupt priority
    IFS0bits.T3IF = 0;      // Clear Timer 3 interrupt flag
    IEC0bits.T3IE = 0;      // Disable Timer 3 interrupt
    
    TMR3 = 0;               // Reset Timer 3 counter
    
    // Timer 3 not used in Part 1
}

// pr3 = 244 (0.25 s), 977 (1 s) or 2930 (3 s)
void Timer3_Start(uint16_t pr3) {
    T3CONbits.TON = 0;
    TMR3 = 0;
    PR3 = pr3;
    T3CONbits.TON = 1;
}

void Timer3_Stop(void) {
    T3CONbits.TON = 0;
}
