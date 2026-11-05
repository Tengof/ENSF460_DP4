/*
 * File:   ADC.c
 * Author: Basema Khan, Michelle Yoon, Vincent Fong
 *
 * Created on November 4, 2026
 */

#include <xc.h>
#include <stdint.h>
#include "ADC.h"
#include "UART2.h"


// Configures and enables the ADC
void ADCinit(void){
    // ADC Initialization
    AD1CON1bits.ADON = 0;   // Turn ADC off before configuring its registers
    
    AD1CON1 = 0;
    AD1CON2 = 0;
    AD1CON3 = 0;
    AD1CHS = 0;
    AD1CSSL = 0;
    
    AD1PCFG = 0xFFFF;       // Configure all analog-capable pins as digital
    AD1PCFGbits.PCFG3 = 0;      // Configure RA2 as analog input
    TRISAbits.TRISA2 = 1;       // Configure RA2 as an input
    
    AD1CON1bits.FORM = 0;       // Output data as unsigned integer
    AD1CON1bits.SSRC = 0b111;       // Internal counter ends sampling and starts conversion
    AD1CON1bits.ASAM = 0;       // Sampling begins when SAMP bit is set
    
    AD1CON2bits.VCFG = 0b000;       // Select AVDD and AVSS as voltage references
    AD1CON2bits.CSCNA = 0;      // Disable input scanning
    AD1CON2bits.ALTS = 0;       // Always use MUX A input settings
    
    AD1CON3bits.ADRC = 0;       // Use system clock for ADC
    AD1CON3bits.SAMC = 31;      // Set automatic sampling time to 31 TAD
    AD1CON3bits.ADCS = 15;       // Set ADC conversion clock to 17 TCY
    
    AD1CHSbits.CH0SA = 3;       // Select RA2 as ADC input
    AD1CHSbits.CH0NA = 0;       // Select AVSS as negative input 
    
    AD1CON1bits.ADON = 1;       // Turn on ADC module
}

// Samples the analog input, performs one conversion, and returns the result
uint16_t do_ADC(void){
    uint16_t ADCvalue;          // Stores the converted ADC reading
    
    AD1CON1bits.DONE = 0;       // Clear previous conversion status
    AD1CON1bits.SAMP = 1;       // Start sampling analog input
    
    // Wait until ADC conversion is complete
    while(AD1CON1bits.DONE == 0){
    }
    
    ADCvalue = ADC1BUF0;        // Store converted digital value
    
    AD1CON1bits.SAMP = 0;       // Stop sampling
    
    return ADCvalue;
    
    
}

