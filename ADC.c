#include <xc.h>
#include "ADC.h"

/*
 * Configures the ADC to read the potentiometer pin, performs ONE
 * sample/conversion, and returns the 10-bit result (0 to 1023).
 */
uint16_t do_ADC(void)
{
    uint16_t ADCvalue;

    //Configure the ADC
    AD1CON1bits.ADON  = 0;        // ADC off while configuring
    AD1CON1bits.FORM  = 0b00;     // integer result: 0 to 1023
    AD1CON1bits.SSRC  = 0b111;    // auto-convert after sampling time ends
    AD1CON1bits.ASAM  = 0;        // sampling starts only when SAMP = 1

    AD1CON2bits.VCFG  = 0b000;    // Vref+ = AVDD, Vref- = AVSS
    AD1CON2bits.CSCNA = 0;        // no input scanning
    AD1CON2bits.SMPI  = 0b0000;   // one conversion per result
    AD1CON2bits.BUFM  = 0;
    AD1CON2bits.ALTS  = 0;        // always use MUX A

    AD1CON3bits.ADRC  = 0;        // ADC clock from system clock
    AD1CON3bits.SAMC  = 0b11111;  // sample time = 31 Tad
   // AD1CON3bits.ADCS  = 0b00111;  // Tad = 8 x Tcy 

    AD1CHSbits.CH0NA  = 0;               // negative input = Vref- (0 V)
    AD1CHSbits.CH0SA  = POT_ADC_CHANNEL; // positive input = potentiometer pin

    AD1PCFG &= ~(1u << POT_ADC_CHANNEL); // make only this pin analog (0 = analog)
    AD1CSSL = 0;                         // nothing selected for scanning

    
    TRISAbits.TRISA2 = 1;         

    
    AD1CON1bits.ADON = 1;         // ADC on
    AD1CON1bits.SAMP = 1;         // start sampling (conversion starts automatically)
    while (!AD1CON1bits.DONE);    // poll until conversion is finished

    ADCvalue = ADC1BUF0;          // read result

    AD1CON1bits.SAMP = 0;
    AD1CON1bits.ADON = 0;         // ADC off between readings to save power

    return ADCvalue;
}
