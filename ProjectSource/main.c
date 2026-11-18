
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

static void WriteHex32(uint32_t value)
{
    static const char hexDigits[] = "0123456789ABCDEF";
    SerialUART_WriteString("0x");
    for (int8_t shift = 28; shift >= 0; shift -= 4)
    {
        SerialUART_WriteChar(hexDigits[(value >> shift) & 0xFu]);
    }
}

// Print why the PIC last reset, then clear the flags so the next reset
// reports only its own cause.
static void PrintResetCause(void)
{
    SerialUART_WriteString("Reset cause:");

    if (RCONbits.POR)
    {
        // BOR is also set on every power-on, so only report it alone.
        SerialUART_WriteString(" POWER-ON");
    }
    else if (RCONbits.BOR)
    {
        SerialUART_WriteString(" BROWN-OUT (supply dipped)");
    }
    if (RCONbits.EXTR)
    {
        SerialUART_WriteString(" MCLR-PIN/PROGRAMMER");
    }
    if (RCONbits.SWR)
    {
        SerialUART_WriteString(" SOFTWARE");
    }
    if (RCONbits.WDTO)
    {
        SerialUART_WriteString(" WATCHDOG");
    }
    if (RCONbits.CMR)
    {
        SerialUART_WriteString(" CONFIG-MISMATCH");
    }
    SerialUART_WriteString("\r\n");

    RCONCLR = _RCON_POR_MASK | _RCON_BOR_MASK | _RCON_EXTR_MASK |
              _RCON_SWR_MASK | _RCON_WDTO_MASK | _RCON_CMR_MASK;
}

// Replaces the default exception handler (an empty infinite loop).
// Silences the buzzer and reports the cause on UART2, then halts.
void __attribute__((nomips16, noreturn)) _general_exception_handler(void)
{
    uint32_t cause = _CP0_GET_CAUSE();
    uint32_t epc = _CP0_GET_EPC();
    uint32_t excCode = (cause >> 2) & 0x1Fu;

    SilenceBuzzer();

    SerialUART_WriteString("\r\n!!! CPU EXCEPTION code ");
    SerialUART_WriteUInt(excCode);
    switch (excCode)
    {
        case 4:  SerialUART_WriteString(" (bad address on load/fetch)"); break;
        case 5:  SerialUART_WriteString(" (bad address on store)"); break;
        case 6:  SerialUART_WriteString(" (bus error on fetch)"); break;
        case 7:  SerialUART_WriteString(" (bus error on load/store)"); break;
        case 10: SerialUART_WriteString(" (illegal instruction)"); break;
        case 12: SerialUART_WriteString(" (arithmetic overflow)"); break;
        case 13: SerialUART_WriteString(" (trap, e.g. divide by zero)"); break;
        default: break;
    }
    SerialUART_WriteString("\r\nEPC (faulting address): ");
    WriteHex32(epc);
    SerialUART_WriteString("\r\nHalted. Look up EPC in the .map/.lst file.\r\n");

    for (;;)
    {
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
    SerialUART_WriteString(
    "\r\n=== FULL GAME POT DEBUG VERSION 2 ===\r\n"
    );
    PrintResetCause();
    InitServoControl();
    InitPotentiometer();

    // CapSense shares ADC1 with the potentiometer; both are polled
    // cooperatively, never from an ADC interrupt.
    CapSense_Init();
    KnifeCapSensor_Init(); // calibrate with the handle untouched

    SerialUART_WriteString("Knife sensor: leave handle untouched during calibration.\r\n");

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
    // Silence the buzzer and report on UART2 (asserts print on UART1).
    SilenceBuzzer();
    SerialUART_WriteString("\r\n!!! FRAMEWORK STOPPED, ES_Return_t = ");
    SerialUART_WriteUInt((uint32_t)ErrorType);
    SerialUART_WriteString("\r\n");

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