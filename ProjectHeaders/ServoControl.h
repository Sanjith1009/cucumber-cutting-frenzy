#include <stdint.h>
#include <stdbool.h>

void InitServoControl(void);

void MoveServo(uint8_t servo, uint8_t position);
void MoveTimerArmServo(uint8_t position);
void MoveTreatServo(uint8_t position);

void SweepServo(uint8_t servo,
                uint8_t startPosition,
                uint8_t endPosition,
                uint16_t durationMs);

void UpdateServos(void);

bool IsServoSweeping(uint8_t servo);

// Original timer-arm interface
void SweepTimerArmServo(uint8_t startPosition,
                        uint8_t endPosition,
                        uint16_t durationMs);

void UpdateTimerArmServo(void);
bool IsTimerArmServoSweeping(void);
