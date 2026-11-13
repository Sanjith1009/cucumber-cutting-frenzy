
#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <stdint.h>

// Initialize SPI2 and the M74HC595 latch pin
void InitShiftRegister(void);

// Transmit an 8-bit value and pulse the latch
void WriteShiftRegister(uint8_t value);

#endif // SHIFT_REGISTER_H
