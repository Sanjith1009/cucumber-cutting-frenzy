
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
        return 0u;
    }

    // Stop any sampling and disable ADC
    // before changing its configuration.
    AD1CON1bits.SAMP = 0;
    AD1CON1bits.ON = 0;

    // Restore RB12 / AN12 as an analog input.
    // Physical pin 23.
    TRISBSET = (1u << 12);
    ANSELBSET = (1u << 12);

    // Select potentiometer channel.
    AD1CHSbits.CH0SA = 12;
    AD1CHSbits.CH0NA = 0;

    // Manual sampling and manual conversion.
    // Clearing SAMP starts the conversion.
    AD1CON1bits.SSRC = 0;
    AD1CON1bits.ASAM = 0;

    // Disable channel scanning.
    AD1CON2bits.CSCNA = 0;

    // Re-enable ADC.
    AD1CON1bits.ON = 1;

    // Clear the previous conversion-complete flag.
    AD1CON1bits.DONE = 0;

    // Start sampling the potentiometer.
    AD1CON1bits.SAMP = 1;

    // Allow 50 microseconds for acquisition.
    // Assumes SYSCLK = 40 MHz and CP0 = 20 MHz.
    uint32_t start = _CP0_GET_COUNT();

    while ((uint32_t)(_CP0_GET_COUNT() - start)
           < 1000u)
    {
    }

    // End sampling and START a fresh conversion.
    AD1CON1bits.SAMP = 0;

    // Wait for this conversion to complete.
    while (!AD1CON1bits.DONE)
    {
    }

    // Return the 10-bit ADC result.
    return (uint16_t)(ADC1BUF0 & 0x03FFu);
}




