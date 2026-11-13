
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

#include "../ProjectHeaders/ShiftRegister.h"
#include "../ProjectHeaders/ServoControl.h"
#include "../ProjectHeaders/Potentiometer.h"
#include "../ProjectHeaders/LEDService.h"

int main(void)
{
    // Initialize PIC32 hardware
    _HW_PIC32Init();

    // Initialize shift register (SPI2)
    InitShiftRegister();

    // Initialize servos
    InitServoControl();

    // Initialize potentiometer (AN12 / RB12)
    InitPotentiometer();

    // Set treat servo to starting position
    MoveServo(2, 50);

    // Turn on all six game LEDs
    FlashAllLEDs();

    // Initialize and run Events & Services Framework
    if (ES_Initialize(ES_Timer_RATE_1mS) == Success)
    {
        (void)ES_Run();
    }

    // Stay here if initialization fails or ES_Run returns
    while (1)
    {
    }
}
