
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "SerialUART.h"

// ========================================
// UART2 INITIALIZATION
// ========================================

void SerialUART_Init(void)
{
    // PPS mappings are configured in main.c
    //
    // UART2 RX: RB8, physical pin 17
    // UART2 TX: RA3, physical pin 10
    //
    // RB9, physical pin 18, is reserved
    // for the knife/chop button.

    U2MODE = 0;
    U2STA = 0;

    // Configure UART RX as digital input
    ANSELBCLR = (1u << 8);
    TRISBSET = (1u << 8);

    // Configure UART TX as digital output
    ANSELACLR = (1u << 3);
    TRISACLR = (1u << 3);

    // Do NOT configure RB9 as a UART pin.
    // Check4ChopButton() handles its GPIO setup.

    // 9600 baud, 8 data bits, no parity,
    // 1 stop bit.
    // Assumes PBCLK = 20 MHz.

    U2MODEbits.BRGH = 0;
    U2MODEbits.PDSEL = 0;
    U2MODEbits.STSEL = 0;

    U2BRG = 129;

    // Enable UART2
    U2MODEbits.ON = 1;
    U2STAbits.UTXEN = 1;
    U2STAbits.URXEN = 1;
}

// ========================================
// TRANSMIT FUNCTIONS
// ========================================

void SerialUART_WriteChar(char c)
{
    while (U2STAbits.UTXBF)
    {
    }

    U2TXREG = c;
}

void SerialUART_WriteString(const char *text)
{
    while (*text != '\0')
    {
        SerialUART_WriteChar(*text++);
    }
}

void SerialUART_WriteUInt(uint32_t value)
{
    char digits[11];
    uint8_t count = 0u;

    do
    {
        digits[count++] =
            (char)('0' + (value % 10u));

        value /= 10u;
    }
    while (value > 0u);

    while (count > 0u)
    {
        SerialUART_WriteChar(digits[--count]);
    }
}

// ========================================
// RECEIVE FUNCTION
// ========================================

bool SerialUART_ReadChar(char *received)
{
    if (received == 0)
    {
        return false;
    }

    // Clear UART receive overflow
    if (U2STAbits.OERR)
    {
        U2STAbits.OERR = 0;
    }

    // No character available
    if (!U2STAbits.URXDA)
    {
        return false;
    }

    *received = (char)U2RXREG;

    return true;
}
