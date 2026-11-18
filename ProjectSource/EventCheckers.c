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
#include "KnifeCapSensor.h"

// PIC32MX170F256B:
// RB9 pin 18: knife/chop button, active LOW. DO NOT map UART2 TX here.
// RB10 pin 21: break-beam input, assumed HIGH when unbroken.
// RB12 pin 23: potentiometer wiper, ADC AN12.
// RB13 pin 24: break-beam IR EMITTER power, driven HIGH by InitBreakBeam().
//              The old limit-switch checker also used RB13, so it must stay
//              disabled (or be moved to another pin).
#define ENABLE_PHYSICAL_HANDLE_SWITCH 0
#if ENABLE_PHYSICAL_HANDLE_SWITCH
#error "RB13 powers the break-beam emitter; move the limit switch to another pin first"
#endif
// Disable h/r simulation while the physical capacitive handle is active.
#define ENABLE_SERIAL_HANDLE_SIMULATION 0

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

    if (key == 'h' || key == 'r')
    {
#if ENABLE_SERIAL_HANDLE_SIMULATION
        return PostHandleState(key == 'h');
#else
        return false;  // Physical sensing owns the handle state.
#endif
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
    static bool checkerAnnounced = false;
    if (!checkerAnnounced)
    {
        checkerAnnounced = true;
        SerialUART_WriteString(
            "Potentiometer checker active\r\n");
    }
    static uint16_t LastDebugTime = 0u;

    uint16_t now = ES_Timer_GetTime();

    // Sample potentiometer every 50 ms
    if ((uint16_t)(now - LastPotCheckTime)
        < POT_CHECK_INTERVAL_MS)
    {
        return false;
    }

    LastPotCheckTime = now;

    // Read ADC from RB12 / AN12 (pin 23)
    uint16_t adc = ReadPotentiometer();

    if (adc > 1023u)
    {
        adc = 1023u;
    }

    // Convert ADC reading to difficulty 1-5
    uint8_t level = 1u + (uint8_t)(
        ((uint32_t)adc * NUM_DIFFICULTY_LEVELS)
        / 1024u
    );

    // Print sensor status every 500 ms
    if ((uint16_t)(now - LastDebugTime) >= 500u)
    {
        LastDebugTime = now;

        SerialUART_WriteString("POT ADC: ");
        SerialUART_WriteUInt(adc);

        SerialUART_WriteString(" | Level: ");
        SerialUART_WriteUInt(level);

        SerialUART_WriteString(" | Last posted: ");
        SerialUART_WriteUInt(LastDifficulty);

        SerialUART_WriteString(" | AN12 analog: ");
        SerialUART_WriteUInt(ANSELBbits.ANSB12);

        SerialUART_WriteString(" | ADC channel: ");
        SerialUART_WriteUInt(AD1CHSbits.CH0SA);

        SerialUART_WriteString(" | ADC enabled: ");
        SerialUART_WriteUInt(AD1CON1bits.ON);


        SerialUART_WriteString("\r\n");
    }

    // Only post when difficulty changes
    if (level == LastDifficulty)
    {
        return false;
    }

    ES_Event_t event = {
        ES_POTENTIOMETER, level
    };

    if (PostSetupService(event))
    {
        LastDifficulty = level;

        SerialUART_WriteString(
            ">>> POT EVENT POSTED: Level ");

        SerialUART_WriteUInt(level);
        SerialUART_WriteString("\r\n");

        return true;
    }

    SerialUART_WriteString(
        "WARNING: POT event post failed\r\n");

    return false;
}

bool CheckLEDAnimation(void)
{
    LEDService_Update();
    return false;  // updates hardware; does not post an event
}

// Power the IR emitter (RB13 output HIGH) and make RB10 the receiver input.
// Called from main before the framework starts so the beam is stable by
// the time Check4BeamUnbroken takes its first reading.
void InitBreakBeam(void)
{
    // Emitter: RB13 (pin 24) digital output, driven HIGH
    ANSELBCLR = (1u << 13);
    LATBSET   = (1u << 13);
    TRISBCLR  = (1u << 13);

    // Receiver: RB10 (pin 21) digital input (external pull-up)
    TRISBSET  = (1u << 10);
}

// Set to 0 to remove the break-beam debug output.
#define BEAM_DEBUG 1
#define BEAM_DEBUG_PERIOD_MS 500u

bool Check4BeamUnbroken(void)
{
    static bool initialized = false;
    static uint8_t candidate = 0u;
    static uint8_t stable = 0u;
    static uint16_t changedAt = 0u;
#if BEAM_DEBUG
    static uint16_t lastDebugTime = 0u;
    static uint16_t rawEdges = 0u;   // raw pin changes since last report
#endif

    if (!initialized)
    {
        TRISBSET = (1u << 10);
        // Sensor is assumed to have an external pull-up.
        candidate = stable = (uint8_t)PORTBbits.RB10;
        changedAt = ES_Timer_GetTime();
        initialized = true;
#if BEAM_DEBUG
        lastDebugTime = changedAt;
        SerialUART_WriteString("BEAM init: RB10 raw=");
        SerialUART_WriteUInt(stable);
        SerialUART_WriteString(stable == BEAM_UNBROKEN_LEVEL ?
            " (UNBROKEN)\r\n" : " (BROKEN)\r\n");
#endif
        return false;
    }

    uint16_t now = ES_Timer_GetTime();
    uint8_t raw = (uint8_t)PORTBbits.RB10;

    if (raw != candidate)
    {
        candidate = raw;
        changedAt = now;
#if BEAM_DEBUG
        ++rawEdges;
#endif
    }

#if BEAM_DEBUG
    // Periodic status: shows the live pin level even if it never changes,
    // and how many raw edges occurred (catches flicker shorter than the
    // 20 ms debounce that would never become a stable change).
    if ((uint16_t)(now - lastDebugTime) >= BEAM_DEBUG_PERIOD_MS)
    {
        lastDebugTime = now;
        SerialUART_WriteString("BEAM raw=");
        SerialUART_WriteUInt(raw);
        SerialUART_WriteString(" stable=");
        SerialUART_WriteUInt(stable);
        SerialUART_WriteString(" edges=");
        SerialUART_WriteUInt(rawEdges);
        SerialUART_WriteString("\r\n");
        rawEdges = 0u;
    }
#endif

    if (candidate != stable &&
        (uint16_t)(now - changedAt) >= BEAM_DEBOUNCE_MS)
    {
        stable = candidate;

#if BEAM_DEBUG
        SerialUART_WriteString(stable == BEAM_UNBROKEN_LEVEL ?
            "BEAM -> UNBROKEN\r\n" : "BEAM -> BROKEN\r\n");
#endif

        // Post only BROKEN -> UNBROKEN transitions.
        if (stable == BEAM_UNBROKEN_LEVEL)
        {
            ES_Event_t event = {ES_BEAM_UNBROKEN, 0u};
            bool posted = PostPlayService(event);
#if BEAM_DEBUG
            if (!posted)
            {
                SerialUART_WriteString("BEAM post to PlayService FAILED\r\n");
            }
#endif
            return posted;
        }
    }
    return false;
}

bool IsBeamUnbroken(void)
{
    return (PORTBbits.RB10 == BEAM_UNBROKEN_LEVEL);
}

// Capacitive knife handle: physical events with diagnostic UART output.
// This checker is called by ES_Framework; it never delays for 30 readings.
static void PrintSignedDelta(int32_t delta)
{
    if (delta < 0)
    {
        SerialUART_WriteChar('-');
        SerialUART_WriteUInt((uint32_t)(-delta));
    }
    else
    {
        SerialUART_WriteUInt((uint32_t)delta);
    }
}

bool Check4CapacitiveHandle(void)
{
    static bool calibrationAnnounced = false;
    static uint8_t windowsSinceReport = 0u;
    KnifeCapSensor_Reading_t reading;

    if (!KnifeCapSensor_Poll(ES_Timer_GetTime(), &reading))
    {
        return false;
    }

    if (!reading.calibrated)
    {
        return false;
    }

    if (!calibrationAnnounced)
    {
        calibrationAnnounced = true;
        SerialUART_WriteString("KNIFE HANDLE CALIBRATED: baseline=");
        SerialUART_WriteUInt(reading.baseline);
        SerialUART_WriteString("\r\n");
    }

    // Diagnostics around every 300 ms, plus every state transition.
    ++windowsSinceReport;
    if (windowsSinceReport >= 5u || reading.stateChanged)
    {
        windowsSinceReport = 0u;
        SerialUART_WriteString("Knife mean: ");
        SerialUART_WriteUInt(reading.mean);
        SerialUART_WriteString(" | var: ");
        SerialUART_WriteUInt(reading.variance);
        SerialUART_WriteString(" | delta: ");
        PrintSignedDelta(reading.delta);
        SerialUART_WriteString(" | ");
        SerialUART_WriteString(reading.held ? "HELD" : "RELEASED");
        SerialUART_WriteString("\r\n");
    }

    // This also retries if posting failed on the prior completed window.
    if (reading.held != HandleTouched)
    {
        return PostHandleState(reading.held);
    }
    return false;
}
