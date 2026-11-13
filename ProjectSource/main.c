
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

#include "../ProjectHeaders/ShiftRegister.h"

// Assumes SYSCLK = 40 MHz, PBCLK = 20 MHz
#define FLASH_TICKS 10000000u  // 500 ms

static void ConfigureSerialAndSPIPins(void)
{
    uint32_t previousStatus =
        __builtin_disable_interrupts();

    // Unlock Peripheral Pin Select (PPS)
    SYSKEY = 0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    CFGCONbits.IOLOCK = 0;

    // SPI2 data: SDO2 -> RB6 (pin 15)
    RPB6R = 0b0100;

    // UART2 receive: RB8 (pin 17)
    U2RXR = 0b0100;

    // UART2 transmit: RB9 (pin 18)
    RPB9R = 0b0010;

    // Lock PPS
    CFGCONbits.IOLOCK = 1;
    SYSKEY = 0;

    if (previousStatus & 1u)
    {
        __builtin_enable_interrupts();
    }
}

static void UART2_Init(void)
{
    U2MODE = 0;
    U2STA = 0;

    // Configure pin directions
    TRISBSET = (1u << 8); // RB8 input
    TRISBCLR = (1u << 9); // RB9 output

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

int main(void)
{
    _HW_PIC32Init();

    // Map SPI2 and UART2 before PPS gets locked
    ConfigureSerialAndSPIPins();

    // Initialize existing SPI2 shift register driver
    InitShiftRegister();

    // Initialize UART2
    UART2_Init();

    // Start with all LEDs off
    WriteShiftRegister(0x00);

    SerialWriteString("\r\nPIC32 LED Controller Ready!\r\n");
    SerialWriteString("a = All ON\r\n");
    SerialWriteString("0 = All OFF\r\n");
    SerialWriteString("1-6 = Individual LED\r\n");
    SerialWriteString("f = Flash all LEDs\r\n");

    bool flashing = false;
    bool flashState = false;
    uint32_t lastFlash = _CP0_GET_COUNT();

    while (1)
    {
        // Recover from UART receive overflow
        if (U2STAbits.OERR)
        {
            U2STAbits.OERR = 0;
        }

        // Check for incoming serial characters
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
                    lastFlash = _CP0_GET_COUNT();
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
                    // Ignore newline and unknown commands
                    break;
            }
        }

        // Nonblocking 500 ms flash interval
        if (flashing)
        {
            uint32_t now = _CP0_GET_COUNT();

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
    }
}
