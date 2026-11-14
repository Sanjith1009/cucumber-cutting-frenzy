
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
    // RB8 = UART2 RX, physical pin 17
    // RA3 = UART2 TX, physical pin 10
    // (RB9 is left alone - it is the chop button input)

    U2MODE = 0;
    U2STA = 0;

    // Configure digital UART pins
    ANSELBCLR = (1u << 8);
    ANSELACLR = (1u << 3);

    TRISBSET = (1u << 8);  // RX input
    LATASET  = (1u << 3);  // idle TX high before enabling output
    TRISACLR = (1u << 3);  // TX output

    // 9600 baud, 8N1
    // Assumes PBCLK = 20 MHz
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

    // Read received character
    *received = (char)U2RXREG;

    return true;
}
