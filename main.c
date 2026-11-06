/*
 * File:   main.c
 * Author: UPDATE THIS WITH YOUR GROUP MEMBER NAMES OR POTENTIALLY LOSE POINTS
 *
 * Created on: USE THE INFORMATION FROM THE HEADER MPLAB X IDE GENERATES FOR YOU
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


#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include "ADC.h"
#include "UART2.h"      
 

#define FCY              4000000UL   //  (Fosc / 2)
#define TICK_MS          100UL       // one tick every 100 ms
#define STREAM_SECONDS   10UL
#define SAMPLES_PER_RUN  ((STREAM_SECONDS * 1000UL) / TICK_MS)   // 100 samples
#define BAR_MAX          32          // longest bar (plus 1 star minimum)
 
//Push buttons from your kit (active low, pressed = 0) 
#define PB1_PRESSED()  (PORTBbits.RB7 == 0)
#define PB2_PRESSED()  (PORTAbits.RA4 == 0)
 
typedef enum { MODE0, MODE1_WAIT, MODE1_STREAM } state_t;
 
volatile uint8_t tick = 0;   // set by Timer2 ISR
 
//Timer2: periodic wake-up tick
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;       // clear flag or the ISR repeats forever
    tick = 1;
}
 
void initTimer2(void)
{
    T2CON = 0;
    T2CONbits.TCKPS = 0b11;                        // prescaler 1:256
    TMR2 = 0;
    PR2 = (uint16_t)((FCY / 256UL) * TICK_MS / 1000UL);  // period = TICK_MS
    IPC1bits.T2IP = 3;                             // interrupt priority
    IFS0bits.T2IF = 0;                             // clear flag before enabling
    IEC0bits.T2IE = 1;                             // enable Timer2 interrupt
    T2CONbits.TON = 1;                             // start timer
}
 
/* ---------- Buttons ---------- */
void initIO(void)
{
    AD1PCFG = 0xFFFF;          // all pins digital; do_ADC() makes its own pin analog
 
    TRISBbits.TRISB7 = 1;      // PB1 input
    TRISAbits.TRISA4 = 1;      // PB2 input
    CNPU2bits.CN23PUE = 1;     // pull-up on RB7 (PB1)  
    CNPU1bits.CN0PUE  = 1;     // pull-up on RA4 (PB2)
}
 
//Mode 0: bar graph + hex on one line
void showBarGraph(uint16_t value)
{
    char line[64];
    uint8_t stars = (uint8_t)(((uint32_t)value * BAR_MAX) / 1023UL) + 1;  // 1 to 33
    uint8_t i, n = 0;
 
    n += sprintf(&line[n], "\rMode 0: ");          // \r = go back to start of line
    for (i = 0; i < stars; i++) line[n++] = '*';
    n += sprintf(&line[n], " 0x%03X", value);
    for (i = stars; i < BAR_MAX + 1; i++) line[n++] = ' ';   // erase leftovers
    line[n] = '\0';
 
    Disp2String(line);         
}
 
//Mode 1: one value per line for Python
void sendValue(uint16_t value)
{
    char line[12];
    sprintf(line, "%u\r\n", value);
    Disp2String(line);
}
 
int main(void)
{
    state_t state = MODE0;
    uint8_t pb1Prev = 0, pb2Prev = 0;
    uint16_t sampleCount = 0;
 
    initIO();
    InitUART2();               
    initTimer2();
 
    while (1)
    {
        if (tick)
        {
            tick = 0;
 
            // Edge detection: act once per press, not while held 
            uint8_t pb1 = PB1_PRESSED();
            uint8_t pb2 = PB2_PRESSED();
            uint8_t pb1Edge = pb1 && !pb1Prev;
            uint8_t pb2Edge = pb2 && !pb2Prev;
            pb1Prev = pb1;
            pb2Prev = pb2;
 
            switch (state)
            {
                case MODE0:
                    if (pb1Edge) {
                        state = MODE1_WAIT;
                        Disp2String("\r\nMode 1: close terminal, run Python, press PB2 to stream\r\n");
                        break;
                    }
                    showBarGraph(do_ADC());
                    break;
 
                case MODE1_WAIT:
                    if (pb1Edge) {
                        state = MODE0;
                        Disp2String("\r\n");
                        break;
                    }
                    if (pb2Edge) {
                        sampleCount = 0;
                        state = MODE1_STREAM;
                    }
                    break;
 
                case MODE1_STREAM:
                    if (pb1Edge) {
                        state = MODE0;
                        Disp2String("\r\n");
                        break;
                    }
                    sendValue(do_ADC());
                    if (++sampleCount >= SAMPLES_PER_RUN) {
                        Disp2String("END\r\n");     // tells Python the run is over
                        state = MODE1_WAIT;
                    }
                    break;
            }
        }
 
        Idle();   // CPU sleeps until the next interrupt (Timer2 tick)
    }
 
    return 0;
}
