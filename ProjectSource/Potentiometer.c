
#include "Potentiometer.h"
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

static bool PotInitialized = false;

void InitPotentiometer(void)
{
    PotInitialized = false;

    // Disable ADC before configuring it
    AD1CON1bits.ON = 0;

    // RB12 (physical pin 23) as analog input
    TRISBbits.TRISB12 = 1;
    ANSELBbits.ANSB12 = 1;

    // Reset ADC configuration
    AD1CON1 = 0;
    AD1CON2 = 0;
    AD1CON3 = 0;
    AD1CHS = 0;
    AD1CSSL = 0;

    // 10-bit unsigned integer output
    AD1CON1bits.FORM = 0;

    // Internal counter ends sampling and starts conversion
    AD1CON1bits.SSRC = 7;

    // Sampling starts when SAMP is set
    AD1CON1bits.ASAM = 0;

    // Voltage references: AVDD and AVSS
    AD1CON2bits.VCFG = 0;

    // ADC timing (assumes PBCLK = 20 MHz)
    AD1CON3bits.ADRC = 0;
    AD1CON3bits.ADCS = 31;
    AD1CON3bits.SAMC = 16;

    // Select analog channel AN12
    AD1CHSbits.CH0SA = 12;

    // Negative input is AVSS
    AD1CHSbits.CH0NA = 0;

    // Enable ADC
    AD1CON1bits.ON = 1;

    PotInitialized = true;
}

uint16_t ReadPotentiometer(void)
{
    if (!PotInitialized)
    {
        return 0;
    }

    // Start sampling
    AD1CON1bits.SAMP = 1;

    // Wait for sampling and conversion to finish
    while (!AD1CON1bits.DONE)
    {
    }

    // Return 10-bit ADC result (0 to 1023)
    return (uint16_t)ADC1BUF0;
}
