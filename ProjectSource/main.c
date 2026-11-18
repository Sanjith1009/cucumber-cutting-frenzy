
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
#include "../ProjectHeaders/KnifeCapSensor.h"
#include "../ProjectHeaders/CapSense.h"
#include "../ProjectHeaders/EventCheckers.h"
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
// CRASH / RESET DIAGNOSTICS
// ========================================

// Stop the buzzer PWM (OC4 on RA2) and hold the pin low. The PWM runs in
// hardware, so without this a crash leaves the last note playing forever.
static void SilenceBuzzer(void)
{
    OC4RS = 0u;
    OC4CONbits.ON = 0;
    LATACLR = (1u << 2);
}

// Replaces the default exception handler (an empty infinite loop).
// Silences the buzzer so a crash doesn't leave a tone playing, then halts.
void __attribute__((nomips16, noreturn)) _general_exception_handler(void)
{
    SilenceBuzzer();

    for (;;)
    {
    }
}

// ========================================
// MAIN
// ========================================
// Set to 1 to run only the knife sensor test (the game does not start).
#define KNIFE_SENSOR_TEST 1

#if KNIFE_SENSOR_TEST
static void WriteSigned(int32_t value)
{
    if (value < 0)
    {
        SerialUART_WriteChar('-');
        value = -value;
    }
    SerialUART_WriteUInt((uint32_t)value);
}

static void RunKnifeSensorTest(void)
{
    KnifeCapSensor_Reading_t reading;
    bool calibrationReported = false;

    // Start the 1 ms tick so ES_Timer_GetTime() advances without ES_Run().
    ES_Timer_Init(ES_Timer_RATE_1mS);

    SerialUART_WriteString("\r\nKNIFE TEST: leave handle untouched...\r\n");

    for (;;)
    {
        if (!KnifeCapSensor_Poll(ES_Timer_GetTime(), &reading))
        {
            continue;   // window not finished yet
        }

        if (!reading.calibrated)
        {
            continue;   // still collecting the 5 calibration windows
        }

        if (!calibrationReported)
        {
            calibrationReported = true;
            SerialUART_WriteString("Calibrated, baseline=");
            SerialUART_WriteUInt(reading.baseline);
            SerialUART_WriteString("\r\n");
        }

        SerialUART_WriteString(reading.held ? "HELD     " : "RELEASED ");
        SerialUART_WriteString("mean=");
        SerialUART_WriteUInt(reading.mean);
        SerialUART_WriteString(" var=");
        SerialUART_WriteUInt(reading.variance);
        SerialUART_WriteString(" delta=");
        WriteSigned(reading.delta);
        if (reading.stateChanged)
        {
            SerialUART_WriteString("  <-- CHANGED");
        }
        SerialUART_WriteString("\r\n");
    }
}
#endif

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

    // CapSense shares ADC1 with the potentiometer; both are polled
    // cooperatively, never from an ADC interrupt.
    CapSense_Init();
    KnifeCapSensor_Init(); // calibrate with the handle untouched
    #if KNIFE_SENSOR_TEST
    RunKnifeSensorTest();   // never returns
    #endif


    // Power the break-beam IR emitter (RB13) and set up its receiver (RB10)
    InitBreakBeam();

    // Move treat servo to closed position
    MoveTreatServo(0u);

    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize(ES_Timer_RATE_1mS);
    if (ErrorType == Success)
    {
        ErrorType = ES_Run();
    }
    //if we got to here, there was an error
    // Silence the buzzer so a stopped framework doesn't leave a tone playing.
    SilenceBuzzer();

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