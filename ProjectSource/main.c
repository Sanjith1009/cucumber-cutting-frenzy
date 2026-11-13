
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
#include "../ProjectHeaders/ShiftRegister.h"

// Delay approximately 1 ms, assuming 40 MHz SYSCLK
static void Delay1ms(void)
{
    const uint32_t ticksPerMs = 20000u;
    uint32_t start = _CP0_GET_COUNT();

    while ((uint32_t)(_CP0_GET_COUNT() - start)
           < ticksPerMs)
    {
    }
}

void main(void)
{
    // Initialize PIC32 hardware
    _HW_PIC32Init();

    // Initialize SPI1 for MAX7219 display
    // Keep this working configuration unchanged
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

    // Initialize shift register using SPI2
    InitShiftRegister();

    // Initialize servos and potentiometer
    InitServoControl();
    InitPotentiometer();

    // Keep treat servo stationary
    MoveServo(2, 50);

    while (1)
    {
        // Read potentiometer
        uint16_t potValue = ReadPotentiometer();

        // Map ADC reading to servo position 20-80
        uint8_t servoPosition =
            20u + (uint8_t)(((uint32_t)potValue * 60u)
                            / 1023u);

        // Timer arm follows potentiometer
        MoveTimerArmServo(servoPosition);

        // Approximately 10 ms delay
        for (uint8_t i = 0; i < 10; i++)
        {
            Delay1ms();
        }

        // Test SPI2 output using pattern 0xB2
        WriteShiftRegister(0xB2);
    }
}
