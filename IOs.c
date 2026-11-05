/*
 * File:   IOs.c
 * Author: Basema Khan, Michelle Yoon, Vincent Fong
 *
 * Created on November 4, 2026
 */

#include "IOs.h"
#include "Timer.h"
#include "UART2.h"

volatile uint16_t TMR2flag = 0;


void IOinit(void) {
    TRISBbits.TRISB7 = 1;       // Configure RB7 (PB1)as an input
    CNPU2bits.CN23PUE = 1;      // Enable internal pull-up resistor for PB1
}

void IOcheck(void) {
    
    // pressed button reads 0 because of the pull-ups
    uint16_t PB1_pressed = (PORTBbits.RB7 == 0);
    uint16_t PB2_pressed = (PORTBbits.RB4 == 0);
    uint16_t PB3_pressed = (PORTAbits.RA4 == 0);
    
    
    if (PB1_pressed && PB2_pressed && PB3_pressed) {
        Timer3_Stop();
        LATBbits.LATB9 = 1;
        Disp2String("\rAll PBs pressed          ");
    }
    else if (PB1_pressed && PB2_pressed) {
        Timer3_Stop();
        LATBbits.LATB9 = 1;
        Disp2String("\rPB1 and PB2 are pressed  ");
    }
    else if (PB1_pressed && PB3_pressed) {
        Timer3_Stop();
        LATBbits.LATB9 = 1;
        Disp2String("\rPB1 and PB3 are pressed  ");
    }
    else if (PB2_pressed && PB3_pressed) {
        Timer3_Stop();
        LATBbits.LATB9 = 1;
        Disp2String("\rPB2 and PB3 are pressed  ");
    }
    else if (PB1_pressed) {
        LATBbits.LATB9 = 1;
        Timer3_Start(244);  // 0.25 s
        Disp2String("\rPB1 is pressed           ");
    }
    else if (PB2_pressed) {
        LATBbits.LATB9 = 1;
        Timer3_Start(977);  // 1 s
        Disp2String("\rPB2 is pressed           ");
    }
    else if (PB3_pressed) {
        LATBbits.LATB9 = 1;
        Timer3_Start(2930); // 3 s
        Disp2String("\rPB3 is pressed           ");
    }
    else {
        Timer3_Stop();
        LATBbits.LATB9 = 0;
        Disp2String("\rNothing pressed          ");
    }
}
