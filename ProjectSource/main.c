/****************************************************************************
 Module
     main.c
 Description
     starter main() function for Events and Services Framework applications
 Notes

*****************************************************************************/
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

// Delay approximately 1 ms, assuming a 40 MHz system clock.
// Core timer counts at SYSCLK / 2.
static void Delay1ms(void)
{
    const uint32_t ticksPerMs = 20000u;

    uint32_t start = _CP0_GET_COUNT();

    while ((uint32_t)(_CP0_GET_COUNT() - start) <
           ticksPerMs)
    {
    }
}

void main(void)
{
  ES_Return_t ErrorType = Success;

  _HW_PIC32Init(); // basic PIC hardware init
  // Your hardware initialization function calls go here
  // Initialize SPI peripheral
  SPISetup_BasicConfig(SPI_SPI1);
  SPISetup_SetLeader(SPI_SPI1, 0);
  SPISetup_SetBitTime(SPI_SPI1, 100); // 100ns = 10MHz clock
  SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0);
  SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1);
  SPISetup_SetClockIdleState(SPI_SPI1, 1);
  SPISetup_SetActiveEdge(SPI_SPI1, 0);
  SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
  SPISetEnhancedBuffer(SPI_SPI1, true);
  SPISetup_EnableSPI(SPI_SPI1);

  // Force MAX7219 to clear its internal memory and shut off random LEDs
  DM_ClearDisplayBuffer();
  while (DM_TakeInitDisplayStep() == false) {
      // Blocks here for a fraction of a millisecond until all 4 modules are blanked
  }

  // Initialize timer-arm servo PWM hardware
  InitServoControl();

  // Set timer-arm servo to center position
  MoveTimerArmServo(20);

  while (1)
    {
        // Sweep from 20 to 80 over 5 seconds
        SweepTimerArmServo(20, 80, 5000);

        while (IsTimerArmServoSweeping())
        {
            Delay1ms();
            UpdateTimerArmServo();
        }

        // Sweep back from 80 to 20 over 5 seconds
        SweepTimerArmServo(80, 20, 5000);

        while (IsTimerArmServoSweeping())
        {
            Delay1ms();
            UpdateTimerArmServo();
        }
    }
}

