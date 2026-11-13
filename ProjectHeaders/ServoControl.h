#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

// Initialize our servo PWM hardware
void InitServoControl(void);

void MoveTimerArmServo(uint8_t position);

void SweepTimerArmServo(uint8_t startPosition, uint8_t endPosition, uint16_t durationMs);

void UpdateTimerArmServo(void);

bool IsTimerArmServoSweeping(void);
#endif