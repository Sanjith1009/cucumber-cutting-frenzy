
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

// Delay approximately 1 ms, assuming 40 MHz SYSCLK
static void Delay1ms(void)
{
    const uint32_t ticksPerMs = 20000u;
    uint32_t start = _CP0_GET_COUNT();

    while ((uint32_t)(_CP0_GET_COUNT() - start) < ticksPerMs)
    {
    }
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

    // Initialize both servos
    InitServoControl();

    // Set initial positions
    MoveTimerArmServo(20);
    MoveServo(2, 80);

    while (1)
    {
        // Both servos sweep in opposite directions
        SweepServo(1, 20, 80, 5000);
        SweepServo(2, 80, 20, 5000);

        while (IsServoSweeping(1) || IsServoSweeping(2))
        {
            Delay1ms();
            UpdateServos();
        }

        // Reverse both servos
        SweepServo(1, 80, 20, 5000);
        SweepServo(2, 20, 80, 5000);

        while (IsServoSweeping(1) || IsServoSweeping(2))
        {
            Delay1ms();
            UpdateServos();
        }
    }
}
