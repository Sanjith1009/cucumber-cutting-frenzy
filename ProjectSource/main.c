
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

#include "../ProjectHeaders/ShiftRegister.h"
#include "../ProjectHeaders/ServoControl.h"
#include "../ProjectHeaders/Potentiometer.h"
#include "../ProjectHeaders/SerialUART.h"
#include "../ProjectHeaders/GameFSM.h"

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
    // TX: RB9, physical pin 18
    U2RXR = 0b0100;
    RPB9R = 0b0010;

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

    // Initialize game state machine
    GameFSM_Init();

    // Main checkpoint test loop
    while (1)
    {
        char received;

        // Read all available serial characters
        while (SerialUART_ReadChar(&received))
        {
            GameFSM_HandleKey(received);
        }

        // Update game state
        GameFSM_Update(_CP0_GET_COUNT());
    }
}
