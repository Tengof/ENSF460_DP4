/*
 * File:   main.c
 * Author: Basema Khan, Michelle Yoon, Vincent Fong
 *
 * Created on November 4, 2026
 */

// FBS
#pragma config BWRP = OFF               // Table Write Protect Boot (Boot segment may be written)
#pragma config BSS = OFF                // Boot segment Protect (No boot program Flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Code Flash Write Protection bit (General segment may be written)
#pragma config GCP = OFF                // General Segment Code Flash Code Protection bit (No protection)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC oscillator (FRC))
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-Speed Start-up disabled))

// FOSC
#pragma config POSCMOD = NONE           // Primary Oscillator Configuration bits (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Enable Configuration bit (CLKO output disabled; pin functions as port I/O)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8 MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary oscillator configured for high-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor Selection (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (WDT prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected; windowed WDT disabled)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = ON              // Power-up Timer Enable bit (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default location for SCL1/SDA1 pins)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset set to lowest voltage (1.8V))
#pragma config MCLRE = ON               // MCLR Pin Enable bit (MCLR pin enabled; RA5 input pin disabled)

// FICD
#pragma config ICS = PGx2               // ICD Pin Placement Select bits (PGC2/PGD2 are used for programming and debugging the device)

// FDS
#pragma config DSWDTPS = DSWDTPSF       // Deep Sleep Watchdog Timer Postscale Select bits (1:2,147,483,648 (25.7 Days))
#pragma config DSWDTOSC = LPRC          // DSWDT Reference Clock Select bit (DSWDT uses LPRC as reference clock)
#pragma config RTCOSC = SOSC            // RTCC Reference Clock Select bit (RTCC uses SOSC as reference clock)
#pragma config DSBOREN = ON             // Deep Sleep Zero-Power BOR Enable bit (Deep Sleep BOR enabled in Deep Sleep)
#pragma config DSWDTEN = ON             // Deep Sleep Watchdog Timer Enable bit (DSWDT enabled)

// #pragma config statements should precede project file includes.

#include <xc.h>
#include <p24F16KA101.h>
#include "clkChange.h"
#include "UART2.h"
#include "ADC.h"
#include "Timer.h"
#include "IOs.h"

void displayBarGraph(uint16_t ADCValue);

int main(void) {
    uint16_t ADCValue;              // Stores the converted ADC reading
    uint16_t count = 0;             // Counts the number of 100ms timer intervals
    uint16_t mode = 0;              // Current operating mode: 0 or 1
    uint16_t previousPB1 = 1;       // Previous PB1 state (1 = unpressed)
    
    newClk(500);        // Sets clock to 500 kHz
    InitUART2();        // Initialize UART2
    TimerInit();        // Initialize Timer 2 and 3
    IOinit();           // Initialize PB1
    ADCinit();          // Initialized ADC module

    T2CONbits.TON = 1;      // Start Timer 2
    
    while(1){
        uint16_t currentPB1 = PORTBbits.RB7;
        
        // Detect a new press (transition from 1 to 0)
        if(previousPB1 == 1 && currentPB1 == 0){
            mode = 1 - mode;    // mode will be 1 if pressed and 0 if not pressed
            
            if(mode == 0){
                Disp2String("\r\nSwitched to Mode 0\r\n");
            } else {
                Disp2String("\r\nSwitched to Mode 1\r\n");
            }
        }
        
        previousPB1 = currentPB1;       // Update previous button state every iteration
        
        // Check if Timer 2 has completed a 100ms interval
        if(TMR2flag == 1){
            TMR2flag = 0;       // Clear Timer 2 flag
            count++;        // Increment the 100ms counter
            
            // After 10 intervals, about 1 second has passed
            if(count == 10){
                count = 0;      // Reset counter
                
                ADCValue = do_ADC();        // Read potentiometer voltage
                
                // Mode 0: Display terminal bar graph
                if(mode == 0){
                    displayBarGraph(ADCValue);
                } else {
                    // Mode 1: Send numerical ADC value to Python file
                    Disp2Dec(ADCValue);
                    Disp2String("\r\n");
                }    
            }
            
            TMR2 = 0;       // Reset Timer 2 counter
            T2CONbits.TON = 1;      // Restart Timer 2
        }
    }
    
    return 0;
}

// Timer 2 ISR
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    IFS0bits.T2IF = 0;      // Clear Timer 2 interrupt flag
    T2CONbits.TON = 0;      // Stope Timer 2
    TMR2flag = 1;       // Signal to the loop in main that 100ms has passed
}
 

// Displays a bar graph corresponding to the ADC reading
void displayBarGraph(uint16_t ADCValue){
    uint16_t i;
    
    uint16_t barLength = ((uint32_t)ADCValue * 40) / 1023;      // Scale 10-bit ADC reading to 40 characters
    Disp2String("Mode 0: [");
    
    // Print one * for each unit of bar length
    for(i = 0; i < barLength; i++){
        XmitUART2('*', 1);
    }
    
    // Print spaces to keep the bar graph a fixed length
    for(i = barLength; i < 40; i++){
        XmitUART2(' ', 1);
    }
    
    Disp2String("] ADC:");
    
    // Display ADC value in hexadecimal
    Disp2Hex(ADCValue);
    
    Disp2String("\r\n");
}
