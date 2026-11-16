#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "EventCheckers.h"

#include "SetupService.h"
#include "PlayService.h"
#include "DeadmanHandling.h"
#include "Potentiometer.h"
#include "LEDService.h"
#include "SerialUART.h"

// PIC32MX170F256B:
// RB9 pin 18: knife/chop button, active LOW. DO NOT map UART2 TX here.
// RB10 pin 21: break-beam input, assumed HIGH when unbroken.
// RB12 pin 23: potentiometer wiper, ADC AN12.
// RB13 pin 24: Sanjith's proposed handle switch; may conflict with
//              your break-beam emitter control. Disabled until resolved.
#define ENABLE_PHYSICAL_HANDLE_SWITCH 0

#define POT_CHECK_INTERVAL_MS 50u
#define NUM_DIFFICULTY_LEVELS 5u
#define CHOP_DEBOUNCE_MS 20u
#define HANDLE_DEBOUNCE_MS 20u
#define BEAM_DEBOUNCE_MS 20u
#define BEAM_UNBROKEN_LEVEL 1u

static uint8_t LastDifficulty = 0u;
static uint16_t LastPotCheckTime = 0u;
static bool HandleTouched = false;

// Send handle events only to the services that need them (not every queue).
static bool PostHandleState(bool touched)
{
    if (touched == HandleTouched)
    {
        return false;
    }

    ES_Event_t event = {
        touched ? ES_HANDLE_TOUCH : ES_HANDLE_RELEASE, 0u
    };

    bool setupPosted = PostSetupService(event);
    bool deadmanPosted = PostDeadmanHandling(event);

    if (setupPosted && deadmanPosted)
    {
        HandleTouched = touched;
        SerialUART_WriteString(touched ?
            "Handle TOUCH\r\n" : "Handle RELEASE\r\n");
        return true;
    }

    // Queue overflow/failure. Keep current state for a future retry.
    return false;
}

bool Check4Keystroke(void)
{
    char key;
    if (!SerialUART_ReadChar(&key))
    {
        return false;
    }

    if (key == 'h')
    {
        return PostHandleState(true);
    }
    if (key == 'r')
    {
        return PostHandleState(false);
    }

    if (key == '\r' || key == '\n')
    {
        return false;
    }

    // Preserve general keyboard events for future GameFSM/PlayService use.
    ES_Event_t event = {ES_NEW_KEY, (uint16_t)(uint8_t)key};
    return PostPlayService(event);
}

bool Check4LimitSwitch(void)
{
#if ENABLE_PHYSICAL_HANDLE_SWITCH
    // RB13 was Sanjith's suggestion. Verify pin allocation before enabling.
    static bool initialized = false;
    static uint8_t candidate = 1u;
    static uint8_t stable = 1u;
    static uint16_t changedAt = 0u;

    if (!initialized)
    {
        ANSELBCLR = (1u << 13);
        TRISBSET = (1u << 13);
        CNPUBSET = (1u << 13);  // active-low switch to GND
        candidate = stable = (uint8_t)PORTBbits.RB13;
        changedAt = ES_Timer_GetTime();
        initialized = true;
        return false;
    }

    uint16_t now = ES_Timer_GetTime();
    uint8_t raw = (uint8_t)PORTBbits.RB13;

    if (raw != candidate)
    {
        candidate = raw;
        changedAt = now;
    }

    if (candidate != stable &&
        (uint16_t)(now - changedAt) >= HANDLE_DEBOUNCE_MS)
    {
        stable = candidate;
        return PostHandleState(stable == 0u);
    }
#endif
    return false;
}

bool Check4ChopButton(void)
{
    static bool initialized = false;
    static uint8_t candidate = 1u;
    static uint8_t stable = 1u;
    static uint16_t changedAt = 0u;

    if (!initialized)
    {
        ANSELBCLR = (1u << 9);
        TRISBSET = (1u << 9);
        CNPUBSET = (1u << 9);   // input HIGH until pressed to GND
        candidate = stable = (uint8_t)PORTBbits.RB9;
        changedAt = ES_Timer_GetTime();
        initialized = true;
        return false;
    }

    uint16_t now = ES_Timer_GetTime();
    uint8_t raw = (uint8_t)PORTBbits.RB9;

    if (raw != candidate)
    {
        candidate = raw;
        changedAt = now;
    }

    if (candidate != stable &&
        (uint16_t)(now - changedAt) >= CHOP_DEBOUNCE_MS)
    {
        stable = candidate;

        // Only generate ES_CHOP when button becomes pressed (LOW).
        if (stable == 0u)
        {
            ES_Event_t event = {ES_CHOP, 0u};
            return PostPlayService(event);
        }
    }
    return false;
}

bool PotentiometerRead(void)
{
    uint16_t now = ES_Timer_GetTime();
    if ((uint16_t)(now - LastPotCheckTime) < POT_CHECK_INTERVAL_MS)
    {
        return false;
    }
    LastPotCheckTime = now;

    uint16_t adc = ReadPotentiometer();
    if (adc > 1023u)
    {
        adc = 1023u;
    }

    // 0..1023 maps to levels 1..5, matching Sanjith's checker.
    uint8_t level = 1u + (uint8_t)(
        ((uint32_t)adc * NUM_DIFFICULTY_LEVELS) / 1024u);

    if (level == LastDifficulty)
    {
        return false;
    }
    LastDifficulty = level;

    // EventParam is a LEVEL, not a raw ADC reading.
    ES_Event_t event = {ES_POTENTIOMETER, level};
    return PostSetupService(event);
}

bool CheckLEDAnimation(void)
{
    LEDService_Update();
    return false;  // updates hardware; does not post an event
}

bool Check4BeamUnbroken(void)
{
    static bool initialized = false;
    static uint8_t candidate = 0u;
    static uint8_t stable = 0u;
    static uint16_t changedAt = 0u;

    if (!initialized)
    {
        TRISBSET = (1u << 10);
        // Sensor is assumed to have an external pull-up.
        candidate = stable = (uint8_t)PORTBbits.RB10;
        changedAt = ES_Timer_GetTime();
        initialized = true;
        return false;
    }

    uint16_t now = ES_Timer_GetTime();
    uint8_t raw = (uint8_t)PORTBbits.RB10;

    if (raw != candidate)
    {
        candidate = raw;
        changedAt = now;
    }

    if (candidate != stable &&
        (uint16_t)(now - changedAt) >= BEAM_DEBOUNCE_MS)
    {
        stable = candidate;

        // Post only BROKEN -> UNBROKEN transitions.
        if (stable == BEAM_UNBROKEN_LEVEL)
        {
            ES_Event_t event = {ES_BEAM_UNBROKEN, 0u};
            return PostPlayService(event);
        }
    }
    return false;
}

bool IsBeamUnbroken(void)
{
    return (PORTBbits.RB10 == BEAM_UNBROKEN_LEVEL);
}
