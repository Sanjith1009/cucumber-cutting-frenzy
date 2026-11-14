
#ifndef SERIAL_UART_H
#define SERIAL_UART_H

#include <stdint.h>
#include <stdbool.h>

// Initialize UART2 at 9600 baud
void SerialUART_Init(void);

// Transmit functions
void SerialUART_WriteChar(char c);
void SerialUART_WriteString(const char *text);
void SerialUART_WriteUInt(uint32_t value);

// Nonblocking receive function
bool SerialUART_ReadChar(char *received);

#endif
