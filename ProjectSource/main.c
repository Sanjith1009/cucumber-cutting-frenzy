
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

#include "../ProjectHeaders/DM_Display.h"
#include "../ProjectHeaders/PIC32_SPI_HAL.h"
#include "../ProjectHeaders/ServoControl.h"
#include "../ProjectHeaders/Potentiometer.h"

// Delay approximately 1 ms, assuming 40 MHz SYSCLK
static void Delay1ms(void)
{
    const uint32_t ticksPerMs = 20000u;
    uint32_t start = _CP0_GET_COUNT();

    while ((uint32_t)(_CP0_GET_COUNT() - start) < ticksPerMs)
    {
    }
}

#define HC595_LATCH_MASK (1u << 11)  // RB11, pin 22

static void InitShiftRegister(void)
{
    // Reset SPI2 only; SPI1 remains unchanged.
    SPI2CON = 0;
    SPI2CON2 = 0;

    SPISetup_BasicConfig(SPI_SPI2);

    // Set SPI2 as master without using the HAL's
    // incorrect clock-pin direction configuration.
    SPI2CONbits.MSTEN = 1;
    SPI2CONbits.SMP = 0;
    SPI2CONbits.MSSEN = 0;
    SPI2CONbits.ENHBUF = 0;

    // SCK2: RB15, physical pin 26
    ANSELBCLR = (1u << 15);
    TRISBCLR = (1u << 15);

    // SDO2: RB6, physical pin 15
    SPISetup_MapSDOutput(SPI_SPI2, SPI_RPB6);

    // M74HC595 shifts on rising clock edges.
    // SPI mode 0: clock idle LOW, data changes
    // on falling edges.
    SPISetup_SetClockIdleState(SPI_SPI2, 0);
    SPISetup_SetActiveEdge(SPI_SPI2, 1);

    // 100 kHz clock assuming 20 MHz PBCLK
    SPISetup_SetBitTime(SPI_SPI2, 10000);

    // Shift register needs 8-bit transfers
    SPISetup_SetXferWidth(SPI_SPI2, SPI_8BIT);

    // Latch: RB11, physical pin 22
    LATBCLR = HC595_LATCH_MASK;
    TRISBCLR = HC595_LATCH_MASK;

    // Clear any receive-overflow condition
    SPI2STATCLR = _SPI2STAT_SPIROV_MASK;

    SPISetup_EnableSPI(SPI_SPI2);
}

static void WriteShiftRegister(uint8_t value)
{
    LATBCLR = HC595_LATCH_MASK;

    // Wait for space in the transmit buffer
    while (SPI2STATbits.SPITBF)
    {
    }

    // Transmit 8 bits
    SPI2BUF = value;

    // Wait until transmission finishes
    while (!SPI2STATbits.SPIRBF)
    {
    }

    // Clear receive buffer flag
    volatile uint32_t dummy = SPI2BUF;
    (void)dummy;

    // Pulse latch HIGH for 1 ms
    // Intentionally long for oscilloscope testing
    LATBSET = HC595_LATCH_MASK;
    Delay1ms();
    LATBCLR = HC595_LATCH_MASK;
}

void main(void)
{
    // Initialize PIC32 hardware
    _HW_PIC32Init();

    // Initialize SPI for MAX7219 display
    SPISetup_BasicConfig(SPI_SPI1);
    SPISetup_SetLeader(SPI_SPI1, 0);
    SPISetup_SetBitTime(SPI_SPI1, 100);
    SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0);
    SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1);
    SPISetup_SetClockIdleState(SPI_SPI1, 1);
    SPISetup_SetActiveEdge(SPI_SPI1, 0);
    SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
    SPISetEnhancedBuffer(SPI_SPI1, true);
    SPISetup_EnableSPI(SPI_SPI1);

    // Initialize MAX7219 display
    DM_ClearDisplayBuffer();

    while (DM_TakeInitDisplayStep() == false)
    {
    }

    // Testing the Shift sgister initlaization
    InitShiftRegister();

    
    // Initialize hardware
    InitServoControl();
    InitPotentiometer();

    // Keep treat servo stationary
    MoveServo(2, 50);

    while (1)
    {
        // Read ADC: 0 to 1023
        uint16_t potValue = ReadPotentiometer();

        // Map ADC value to a conservative
        // servo position range: 20 to 80
        uint8_t servoPosition =
            20u + (uint8_t)(((uint32_t)potValue * 60u)
                             / 1023u);

        // Timer-arm servo follows potentiometer
        MoveTimerArmServo(servoPosition);

        // Approximately 10 ms between updates
        for (uint8_t i = 0; i < 10; i++)
        {
            Delay1ms();
        }
        WriteShiftRegister(0x81);  // Example pattern to test the shift register

    }
}
