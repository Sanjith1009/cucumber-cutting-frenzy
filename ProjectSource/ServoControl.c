
#include "ServoControl.h"
#include "PWM_PIC32.h"
#include <stdint.h>
#include <stdbool.h>

// Servo configuration
#define NUM_SERVOS          2u

#define TIMER_ARM_SERVO     1u
#define SECOND_SERVO        2u

// PWM timer ticks (0.4 us per tick)
#define SERVO_MIN_TICKS     2500u
#define SERVO_CENTER_TICKS  3750u
#define SERVO_MAX_TICKS     5000u

static bool ServoInitialized = false;

// Independent sweep state for each servo
typedef struct
{
    uint8_t start;
    uint8_t end;
    uint16_t duration;
    uint16_t elapsed;
    bool active;
} ServoSweep_t;

static ServoSweep_t sweeps[NUM_SERVOS];

void InitServoControl(void)
{
    ServoInitialized = false;

    if (!PWMSetup_BasicConfig(NUM_SERVOS))
    {
        return;
    }

    // Both servos use Timer2
    if (!PWMSetup_AssignChannelToTimer(1, _Timer2_))
    {
        return;
    }

    if (!PWMSetup_AssignChannelToTimer(2, _Timer2_))
    {
        return;
    }

    // Both servos begin at center
    if (!PWMOperate_SetPulseWidthOnChannel(
            SERVO_CENTER_TICKS, 1))
    {
        return;
    }

    if (!PWMOperate_SetPulseWidthOnChannel(
            SERVO_CENTER_TICKS, 2))
    {
        return;
    }

    // Servo 1: OC1 -> RB3
    if (!PWMSetup_MapChannelToOutputPin(1, PWM_RPB3))
    {
        return;
    }

    // Servo 2: OC2 -> RB5
    if (!PWMSetup_MapChannelToOutputPin(2, PWM_RPB5))
    {
        return;
    }

    // Reset sweep state
    for (uint8_t i = 0; i < NUM_SERVOS; i++)
    {
        sweeps[i].active = false;
    }

    ServoInitialized = true;
}

// Move either servo immediately to a position
void MoveServo(uint8_t servo, uint8_t position)
{
    if (!ServoInitialized || servo < 1 || servo > NUM_SERVOS)
    {
        return;
    }

    if (position > 100)
    {
        position = 100;
    }

    uint16_t pulseTicks =
        SERVO_MIN_TICKS +
        (uint16_t)(((uint32_t)position *
        (SERVO_MAX_TICKS - SERVO_MIN_TICKS)) / 100u);

    PWMOperate_SetPulseWidthOnChannel(pulseTicks, servo);
}

// Preserve original function
void MoveTimerArmServo(uint8_t position)
{
    MoveServo(TIMER_ARM_SERVO, position);
}

// Control second servo independently
void MoveTreatServo(uint8_t position)
{
    MoveServo(SECOND_SERVO, position);
}

// Start a timed sweep on either servo
void SweepServo(uint8_t servo,
                uint8_t startPosition,
                uint8_t endPosition,
                uint16_t durationMs)
{
    if (!ServoInitialized || servo < 1 || servo > NUM_SERVOS)
    {
        return;
    }

    if (startPosition > 100)
    {
        startPosition = 100;
    }

    if (endPosition > 100)
    {
        endPosition = 100;
    }

    uint8_t index = servo - 1;

    sweeps[index].start = startPosition;
    sweeps[index].end = endPosition;
    sweeps[index].duration = durationMs;
    sweeps[index].elapsed = 0;
    sweeps[index].active = false;

    MoveServo(servo, startPosition);

    if (durationMs == 0)
    {
        MoveServo(servo, endPosition);
        return;
    }

    sweeps[index].active = true;
}

// Update both servos once every 1 ms
void UpdateServos(void)
{
    if (!ServoInitialized)
    {
        return;
    }

    for (uint8_t i = 0; i < NUM_SERVOS; i++)
    {
        if (!sweeps[i].active)
        {
            continue;
        }

        ServoSweep_t *sweep = &sweeps[i];

        if (sweep->elapsed < sweep->duration)
        {
            sweep->elapsed++;
        }

        int32_t difference =
            (int32_t)sweep->end - (int32_t)sweep->start;

        int32_t position =
            (int32_t)sweep->start +
            (difference * sweep->elapsed) / sweep->duration;

        MoveServo(i + 1, (uint8_t)position);

        if (sweep->elapsed >= sweep->duration)
        {
            MoveServo(i + 1, sweep->end);
            sweep->active = false;
        }
    }
}

// Returns true if selected servo is sweeping
bool IsServoSweeping(uint8_t servo)
{
    if (servo < 1 || servo > NUM_SERVOS)
    {
        return false;
    }

    return sweeps[servo - 1].active;
}

// Compatibility with your original timer-arm functions
void SweepTimerArmServo(uint8_t startPosition,
                        uint8_t endPosition,
                        uint16_t durationMs)
{
    SweepServo(TIMER_ARM_SERVO,
               startPosition,
               endPosition,
               durationMs);
}

void UpdateTimerArmServo(void)
{
    UpdateServos();
}

bool IsTimerArmServoSweeping(void)
{
    return IsServoSweeping(TIMER_ARM_SERVO);
}
