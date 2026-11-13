
#include <xc.h>
#include <stdint.h>

#include "ShiftRegister.h"
#include "PIC32_SPI_HAL.h"

// M74HC595 connections:
// DATA  = RB6  (physical pin 15) - SPI2 SDO
// CLOCK = RB15 (physical pin 26) - SPI2 SCK
// LATCH = RB11 (physical pin 22) - GPIO

#define HC595_LATCH_MASK (1u << 11)

// Approximately 1 ms assuming 40 MHz SYSCLK.
// Kept private to this module.
static void ShiftRegister_Delay1ms(void)
{
    const uint32_t ticksPerMs = 20000u;
    uint32_t start = _CP0_GET_COUNT();

    while ((uint32_t)(_CP0_GET_COUNT() - start)
           < ticksPerMs)
    {
    }
}

// Initialize SPI2 for the M74HC595
void InitShiftRegister(void)
{
    // Reset SPI2 only; SPI1 remains unchanged
    SPI2CON = 0;
    SPI2CON2 = 0;

    SPISetup_BasicConfig(SPI_SPI2);

    // SPI2 master mode
    // Standard buffering, no hardware SS
    SPI2CONbits.MSTEN = 1;
    SPI2CONbits.SMP = 0;
    SPI2CONbits.MSSEN = 0;
    SPI2CONbits.ENHBUF = 0;

    // SCK2: RB15, physical pin 26
    ANSELBCLR = (1u << 15);
    TRISBCLR = (1u << 15);

    // SDO2: RB6, physical pin 15
    SPISetup_MapSDOutput(SPI_SPI2, SPI_RPB6);

    // SPI mode 0:
    // Clock idle LOW, data sampled on rising edge
    SPISetup_SetClockIdleState(SPI_SPI2, 0);
    SPISetup_SetActiveEdge(SPI_SPI2, 1);

    // 100 kHz clock assuming 20 MHz PBCLK
    SPISetup_SetBitTime(SPI_SPI2, 10000);

    // 8-bit transmissions
    SPISetup_SetXferWidth(SPI_SPI2, SPI_8BIT);

    // Latch: RB11, physical pin 22
    LATBCLR = HC595_LATCH_MASK;
    TRISBCLR = HC595_LATCH_MASK;

    // Clear receive overflow
    SPI2STATCLR = _SPI2STAT_SPIROV_MASK;

    // Enable SPI2
    SPISetup_EnableSPI(SPI_SPI2);
}

// Send one byte through SPI2 and latch it
void WriteShiftRegister(uint8_t value)
{
    // Hold latch LOW during transmission
    LATBCLR = HC595_LATCH_MASK;

    // Wait for transmit buffer availability
    while (SPI2STATbits.SPITBF)
    {
    }

    // Send 8 bits
    SPI2BUF = value;

    // Wait until transmission is complete
    while (!SPI2STATbits.SPIRBF)
    {
    }

    // Drain the receive buffer
    volatile uint32_t dummy = SPI2BUF;
    (void)dummy;

    // Pulse latch HIGH for approximately 1 ms
    LATBSET = HC595_LATCH_MASK;
    ShiftRegister_Delay1ms();
    LATBCLR = HC595_LATCH_MASK;
}
