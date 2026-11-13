
#include "ServoControl.h"
#include "PWM_PIC32.h"
#include <stdint.h>
#include <stdbool.h>

// Servo PWM configuration
#define SERVO_CHANNEL      1u
#define SERVO_OUTPUT_PIN   PWM_RPB3

// Timer tick = 0.4 microseconds
#define SERVO_MIN_TICKS    2500u  // 1.0 ms
#define SERVO_CENTER_TICKS 3750u  // 1.5 ms
#define SERVO_MAX_TICKS    5000u  // 2.0 ms

static bool ServoInitialized = false;

void InitServoControl(void)
{
    ServoInitialized = false;

    // Configure one PWM channel.
    if (!PWMSetup_BasicConfig(1))
    {
        return;
    }

    // Use Timer2 for OC1.
    if (!PWMSetup_AssignChannelToTimer(
            SERVO_CHANNEL, _Timer2_))
    {
        return;
    }

    // Initialize at center (1.5 ms pulse).
    if (!PWMOperate_SetPulseWidthOnChannel(
            SERVO_CENTER_TICKS, SERVO_CHANNEL))
    {
        return;
    }

    // Route OC1 to RB3.
    if (!PWMSetup_MapChannelToOutputPin(
            SERVO_CHANNEL, SERVO_OUTPUT_PIN))
    {
        return;
    }

    ServoInitialized = true;
}

void MoveTimerArmServo(uint8_t position)
{
    if (!ServoInitialized)
    {
        return;
    }

    // Limit position to 0-100.
    if (position > 100u)
    {
        position = 100u;
    }

    // Convert 0-100 to 2500-5000 timer ticks.
    uint16_t pulseTicks =
        SERVO_MIN_TICKS +
        (uint16_t)(((uint32_t)position *
        (SERVO_MAX_TICKS - SERVO_MIN_TICKS)) / 100u);

    PWMOperate_SetPulseWidthOnChannel(
        pulseTicks, SERVO_CHANNEL);
}

// Sweep state
static uint8_t sweepStart = 0;
static uint8_t sweepEnd = 0;
static uint16_t sweepDuration = 0;
static uint16_t sweepElapsed = 0;
static bool sweepActive = false;

/**
 * Starts a servo sweep.
 *
 * startPosition: Starting position (0-100)
 * endPosition:   Ending position (0-100)
 * durationMs:    Total time to complete the sweep
 */
void SweepTimerArmServo(uint8_t startPosition,
                        uint8_t endPosition,
                        uint16_t durationMs)
{
    // Clamp positions to valid range
    if (startPosition > 100)
        startPosition = 100;

    if (endPosition > 100)
        endPosition = 100;

    sweepStart = startPosition;
    sweepEnd = endPosition;
    sweepDuration = durationMs;
    sweepElapsed = 0;

    // Move to starting position
    MoveTimerArmServo(sweepStart);

    // Handle zero-duration sweeps
    if (durationMs == 0)
    {
        MoveTimerArmServo(sweepEnd);
        sweepActive = false;
        return;
    }

    sweepActive = true;
}

/**
 * Call this once every 1 ms.
 * Updates the commanded servo position.
 */
void UpdateTimerArmServo(void)
{
    if (!sweepActive)
        return;

    if (sweepElapsed < sweepDuration)
        sweepElapsed++;

    // Signed math supports both directions.
    int32_t difference =
        (int32_t)sweepEnd - (int32_t)sweepStart;

    int32_t position =
        (int32_t)sweepStart +
        (difference * sweepElapsed) / sweepDuration;

    MoveTimerArmServo((uint8_t)position);

    // Stop exactly at the requested endpoint.
    if (sweepElapsed >= sweepDuration)
    {
        MoveTimerArmServo(sweepEnd);
        sweepActive = false;
    }
}

bool IsTimerArmServoSweeping(void)
{
    return sweepActive;
}
