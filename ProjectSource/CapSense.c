
#include <xc.h>
#include <stdint.h>
#include "CapSense.h"

// Assumes 40 MHz system clock:
// CP0 counts at 20 MHz.
#define CP0_TICKS_PER_US 20u

// Adjustable for electrode capacitance.
#define CTMU_CHARGE_US 40u

static void DelayUs(uint32_t us)
{
    uint32_t start = _CP0_GET_COUNT();
    uint32_t ticks = us * CP0_TICKS_PER_US;

    while ((uint32_t)(_CP0_GET_COUNT() - start)
           < ticks)
    {
    }
}

void CapSense_Init(void)
{
    // RB2 = AN4 = physical pin 6
    TRISBSET = (1u << 2);
    ANSELBSET = (1u << 2);

    // Configure CTMU
    CTMUCON = 0;

    // 0.55 uA nominal current source
    CTMUCONbits.IRNG = 1;
    CTMUCONbits.ITRIM = 0;

    CTMUCONbits.TGEN = 0;
    CTMUCONbits.EDGEN = 1;
    CTMUCONbits.EDG1STAT = 0;
    CTMUCONbits.EDG2STAT = 0;

    CTMUCONbits.ON = 1;
}

uint16_t CapSense_ReadRaw(void)
{
    // The potentiometer also uses ADC1.
    // Save its channel and conversion mode.
    uint8_t savedChannel =
        (uint8_t)AD1CHSbits.CH0SA;

    uint8_t savedSSRC =
        (uint8_t)AD1CON1bits.SSRC;

    // Stop sampling and select handle channel.
    AD1CON1bits.SAMP = 0;
    AD1CHSbits.CH0SA = 4;  // AN4 = RB2
    AD1CON1bits.SSRC = 0;  // Manual conversion

    // clear the completion status left by the previous potentiometer or CTMU reading
    AD1CON1bits.DONE = 0;

    // Connect ADC sample capacitor to sensor.
    AD1CON1bits.SAMP = 1;

    // Discharge the sensor and ADC capacitance.
    CTMUCONbits.EDG1STAT = 0;
    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.IDISSEN = 1;

    DelayUs(20);

    // Stop discharging.
    CTMUCONbits.IDISSEN = 0;

    // Apply constant current for fixed time.
    CTMUCONbits.EDG1STAT = 1;

    DelayUs(CTMU_CHARGE_US);

    // Stop charging.
    CTMUCONbits.EDG1STAT = 0;

    // End sampling and perform conversion.
    AD1CON1bits.SAMP = 0;

    while (!AD1CON1bits.DONE)
    {
    }

    uint16_t result = (uint16_t)ADC1BUF0;

    // Restore ADC settings for potentiometer.
    AD1CHSbits.CH0SA = savedChannel;
    AD1CON1bits.SSRC = savedSSRC;

    return result;
}
