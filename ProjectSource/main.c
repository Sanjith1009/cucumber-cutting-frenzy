
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

#include "../ProjectHeaders/ShiftRegister.h"
#include "../ProjectHeaders/ServoControl.h"
#include "../ProjectHeaders/Potentiometer.h"

// Assumes SYSCLK = 40 MHz and PBCLK = 20 MHz
// PIC32 CP0 increments at SYSCLK / 2 = 20 MHz

#define FLASH_TICKS   10000000u  // 500 ms
#define POT_TICKS      1000000u  // 50 ms
#define REPORT_TICKS   5000000u  // 250 ms

// ====================================================
// PIN MAPPING
// ====================================================

static void ConfigurePinMappings(void)
{
    uint32_t previousStatus =
        __builtin_disable_interrupts();

    SYSKEY = 0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    CFGCONbits.IOLOCK = 0;

    // SPI2 data to shift register
    // RB6, physical pin 15
    RPB6R = 0b0100;   // SDO2

    // UART2 for D-SUN USB-to-TTL adapter
    // RB8 RX, physical pin 17
    // RB9 TX, physical pin 18
    U2RXR = 0b0100;
    RPB9R = 0b0010;

    // Servo PWM outputs
    // RB3, physical pin 7
    // RB5, physical pin 14
    RPB3R = 0b0101;   // OC1
    RPB5R = 0b0101;   // OC2

    CFGCONbits.IOLOCK = 1;
    SYSKEY = 0;

    if (previousStatus & 1u)
    {
        __builtin_enable_interrupts();
    }
}

// ====================================================
// UART2 SERIAL COMMUNICATION
// ====================================================

static void UART2_Init(void)
{
    U2MODE = 0;
    U2STA = 0;

    // Configure digital UART pins
    ANSELBCLR = (1u << 8) | (1u << 9);
    TRISBSET = (1u << 8);
    TRISBCLR = (1u << 9);

    // 9600 baud, 8N1, PBCLK = 20 MHz
    U2MODEbits.BRGH = 0;
    U2MODEbits.PDSEL = 0;
    U2MODEbits.STSEL = 0;
    U2BRG = 129;

    U2MODEbits.ON = 1;
    U2STAbits.UTXEN = 1;
    U2STAbits.URXEN = 1;
}

static void SerialWriteChar(char c)
{
    while (U2STAbits.UTXBF)
    {
    }

    U2TXREG = c;
}

static void SerialWriteString(const char *str)
{
    while (*str)
    {
        SerialWriteChar(*str++);
    }
}

// Print an unsigned integer without printf
static void SerialWriteUInt(uint16_t value)
{
    char digits[6];
    uint8_t count = 0;

    do
    {
        digits[count++] =
            '0' + (char)(value % 10u);

        value /= 10u;
    }
    while (value > 0u);

    while (count > 0u)
    {
        SerialWriteChar(digits[--count]);
    }
}

// ====================================================
// MAIN HARDWARE TEST
// ====================================================

int main(void)
{
    _HW_PIC32Init();

    // Set all peripheral pin mappings together
    ConfigurePinMappings();

    // Initialize hardware modules
    InitShiftRegister();
    UART2_Init();
    InitServoControl();
    InitPotentiometer();

    // Treat servo remains stationary
    MoveServo(2, 50);

    // Start with LEDs OFF
    WriteShiftRegister(0x00);

    SerialWriteString("\r\n");
    SerialWriteString("============================\r\n");
    SerialWriteString("PIC32 HARDWARE TEST HARNESS\r\n");
    SerialWriteString("============================\r\n");
    SerialWriteString("a = All LEDs ON\r\n");
    SerialWriteString("0 = All LEDs OFF\r\n");
    SerialWriteString("1-6 = Individual LED\r\n");
    SerialWriteString("f = Flash all LEDs\r\n");
    SerialWriteString("\r\n");
    SerialWriteString("Potentiometer controls timer-arm servo\r\n");
    SerialWriteString("ADC and servo commands reported every 250 ms\r\n");

    bool flashing = false;
    bool flashState = false;

    uint16_t potValue = 0u;
    uint8_t servoPosition = 50u;

    uint32_t now = _CP0_GET_COUNT();
    uint32_t lastFlash = now;
    uint32_t lastPot = now;
    uint32_t lastReport = now;

    while (1)
    {
        now = _CP0_GET_COUNT();

        // =========================================
        // SERIAL LED COMMANDS
        // =========================================

        if (U2STAbits.OERR)
        {
            U2STAbits.OERR = 0;
        }

        if (U2STAbits.URXDA)
        {
            char received = (char)U2RXREG;

            switch (received)
            {
                case 'a':
                    flashing = false;
                    WriteShiftRegister(0x3F);
                    SerialWriteString("All LEDs ON\r\n");
                    break;

                case '0':
                    flashing = false;
                    WriteShiftRegister(0x00);
                    SerialWriteString("All LEDs OFF\r\n");
                    break;

                case 'f':
                    flashing = true;
                    flashState = true;
                    lastFlash = now;
                    WriteShiftRegister(0x3F);
                    SerialWriteString("Flashing LEDs\r\n");
                    break;

                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                    flashing = false;

                    WriteShiftRegister(
                        (uint8_t)(1u << (received - '1'))
                    );

                    SerialWriteString("LED selected: ");
                    SerialWriteChar(received);
                    SerialWriteString("\r\n");
                    break;

                default:
                    break;
            }
        }

        // =========================================
        // NONBLOCKING LED FLASHING
        // =========================================

        if (flashing)
        {
            if ((uint32_t)(now - lastFlash)
                >= FLASH_TICKS)
            {
                lastFlash = now;
                flashState = !flashState;

                WriteShiftRegister(
                    flashState ? 0x3F : 0x00
                );
            }
        }

        // =========================================
        // POTENTIOMETER + SERVO CONTROL
        // =========================================

        if ((uint32_t)(now - lastPot)
            >= POT_TICKS)
        {
            lastPot = now;

            // Read ADC: 0 to 1023
            potValue = ReadPotentiometer();

            // Map ADC to tested servo command range
            uint8_t newPosition =
                20u + (uint8_t)(
                    ((uint32_t)potValue * 60u) / 1023u
                );

            // Only update PWM if position changes
            if (newPosition != servoPosition)
            {
                servoPosition = newPosition;
                MoveTimerArmServo(servoPosition);
            }
        }

        // =========================================
        // SERIAL POSITION REPORTING
        // =========================================

        if ((uint32_t)(now - lastReport)
            >= REPORT_TICKS)
        {
            lastReport = now;

            SerialWriteString("Pot ADC: ");
            SerialWriteUInt(potValue);

            SerialWriteString(" / 1023");

            SerialWriteString("  |  Servo command: ");
            SerialWriteUInt(servoPosition);

            SerialWriteString(" / 100\r\n");
        }
    }
}
