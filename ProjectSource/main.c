
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

#include "../ProjectHeaders/ShiftRegister.h"
#include "../ProjectHeaders/ServoControl.h"
#include "../ProjectHeaders/Potentiometer.h"
#include "../ProjectHeaders/SerialUART.h"
#include "../ProjectHeaders/GameFSM.h"
#include "../ProjectHeaders/PIC32_SPI_HAL.h"

#include "PWM_PIC32.h"
#include "AudioService.h"
#include "PIC32_AD_Lib.h"

// ========================================
// PERIPHERAL PIN MAPPING
// ========================================

static void ConfigurePinMappings(void)
{
    uint32_t previousStatus =
        __builtin_disable_interrupts();

    SYSKEY = 0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    CFGCONbits.IOLOCK = 0;

    // SPI2 data
    // RB6, physical pin 15
    RPB6R = 0b0100;

    // UART2
    // RX: RB8, physical pin 17
    // TX: RA3, physical pin 10 (moved off RB9, which is the chop button)
    U2RXR = 0b0100;
    RPA3R = 0b0010;

    // Servo outputs
    // OC1: RB3, physical pin 7
    // OC2: RB5, physical pin 14
    RPB3R = 0b0101;
    RPB5R = 0b0101;

    CFGCONbits.IOLOCK = 1;
    SYSKEY = 0;

    if (previousStatus & 1u)
    {
        __builtin_enable_interrupts();
    }
}

// ========================================
// MAIN
// ========================================

int main(void)
{
    ES_Return_t ErrorType = Success;

    // Initialize PIC32
    _HW_PIC32Init();

    // Configure peripheral mappings
    ConfigurePinMappings();

    // Initialize hardware modules
    InitShiftRegister();
    SerialUART_Init();
    InitServoControl();
    InitPotentiometer();

    // Keep treat servo stationary
    MoveServo(2, 50);

    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize(ES_Timer_RATE_1mS);
    if (ErrorType == Success)
    {
        ErrorType = ES_Run();
    }
    //if we got to here, there was an error
    switch (ErrorType)
    {
        case FailedPost:
        {
        ErrorType = 0; // a hack to get a readable assert message
        assert( FailedPost == ErrorType );
        //DB_printf("Failed on attempt to Post\n");
        }
        break;
        case FailedPointer:
        {
        ErrorType = 0; // a hack to get a readable assert message
        assert( FailedPointer  == ErrorType );
    //      DB_printf("Failed on NULL pointer\n");
        }
        break;
        case FailedInit:
        {
        ErrorType = 0; // a hack to get a readable assert message
        assert( FailedInit  == ErrorType );
    //      DB_printf("Failed Initialization\n");
        }
        break;
        default:
        {
        ErrorType = 0; // a hack to get a readable assert message
        assert( FailedOther  == ErrorType );
    //      DB_printf("Other Failure\n");
        }
        break;
    }
    for ( ; ;)
    {
        ;
    }
}
