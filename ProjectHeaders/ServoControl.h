#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <stdint.h>

void InitServoControl(void);
void MoveTimerArmServo(uint8_t position);

#endif