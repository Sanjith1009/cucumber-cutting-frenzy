
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "SetupService.h"
#include "PlayService.h"
#include "Potentiometer.h"
#include "ServoControl.h"
#include "LEDService.h"
#include "AudioService.h"
#include "SerialUART.h"

// ===========================================
// CONFIGURATION
// ===========================================

#define COUNTDOWN_DURATION_MS 3000u
#define TIMER_ARM_PARK_POS    10u

// PLAY_DURATION_MS is defined in PlayService.h
// and should equal 60000u.

// ===========================================
// SETUP PHASES
// ===========================================

typedef enum
{
    SETUP_SELECTING,
    SETUP_COUNTING_DOWN,
    SETUP_IN_GAME
} SetupPhase_t;

// ===========================================
// MODULE VARIABLES
// ===========================================

static uint8_t MyPriority;

static SetupPhase_t SetupPhase = SETUP_SELECTING;

static uint8_t difficultyLED = 1u;
static uint16_t cutThreshold = 3u;

// ===========================================
// PRIVATE FUNCTION DECLARATIONS
// ===========================================

static uint8_t PotToDifficultyLevel(uint16_t adc);
static uint16_t CalculateDifficulty(uint16_t level);

static void SetDifficulty(uint8_t level);
static void EnterSelection(void);
static void StartCountdown(void);
static void StartGameplay(void);

// ===========================================
// DIFFICULTY CALCULATIONS
// ===========================================

// Convert raw ADC value (0-1023)
// into difficulty level (1-5).
//
// This is the same conversion used in
// EventCheckers.c / PotentiometerRead().

static uint8_t PotToDifficultyLevel(uint16_t adc)
{
    if (adc > 1023u)
    {
        adc = 1023u;
    }

    return 1u + (uint8_t)(
        ((uint32_t)adc * 5u) / 1024u
    );
}

// Sanjith's difficulty-to-chop mapping:
//
// Difficulty 1 -> 3 chops
// Difficulty 2 -> 4 chops
// Difficulty 3 -> 6 chops
// Difficulty 4 -> 8 chops
// Difficulty 5 -> 10 chops

static uint16_t CalculateDifficulty(uint16_t level)
{
    if (level < 1u)
    {
        level = 1u;
    }
    else if (level > 5u)
    {
        level = 5u;
    }

    return 3u + ((level - 1u) * 7u) / 4u;
}

// ===========================================
// UPDATE SELECTED DIFFICULTY
// ===========================================

static void SetDifficulty(uint8_t level)
{
    if (level < 1u || level > 5u)
    {
        return;
    }

    // Store difficulty level
    difficultyLED = level;

    // Store the corresponding number of chops
    cutThreshold = CalculateDifficulty(level);

    // Illuminate LEDs 1 through difficulty level
    LEDService_SetLitCount(difficultyLED);

    // Display difficulty over UART2
    SerialUART_WriteString("\r\nDifficulty: ");
    SerialUART_WriteUInt(difficultyLED);

    SerialUART_WriteString(" / 5");

    SerialUART_WriteString(" | Required chops: ");
    SerialUART_WriteUInt(cutThreshold);

    SerialUART_WriteString("\r\n");
}

// ===========================================
// ENTER DIFFICULTY SELECTION
// ===========================================

static void EnterSelection(void)
{
    // If resetting an active game, stop it.
    if (SetupPhase == SETUP_IN_GAME)
    {
        ES_Timer_StopTimer(PLAY_TIMER);

        ES_Event_t StopEvent = {
            ES_NOT_PLAYING, 0u
        };

        PostPlayService(StopEvent);
    }

    // Cancel any unfinished starting countdown
    ES_Timer_StopTimer(COUNTDOWN_TIMER);

    SetupPhase = SETUP_SELECTING;

    // Park timer-arm servo
    MoveTimerArmServo(TIMER_ARM_PARK_POS);

    // Welcome message
    SerialUART_WriteString("\r\n");
    SerialUART_WriteString(
        "================================\r\n");
    SerialUART_WriteString(
        "WELCOME TO CUCUMBER CUTTING FRENZY!\r\n");
    SerialUART_WriteString(
        "================================\r\n");

    SerialUART_WriteString(
        "Rotate potentiometer to select difficulty.\r\n");

    SerialUART_WriteString(
        "Difficulty ranges from 1 to 5.\r\n");

    SerialUART_WriteString(
        "LEDs indicate selected difficulty.\r\n");

    SerialUART_WriteString(
        "Touch the handle or press 'h' to start.\r\n");

    // Immediately read the potentiometer.
    // Do not wait for an event from EventCheckers.
    uint16_t adc = ReadPotentiometer();

    SetDifficulty(PotToDifficultyLevel(adc));
}

// ===========================================
// START 3-2-1 COUNTDOWN
// ===========================================

static void StartCountdown(void)
{
    if (SetupPhase != SETUP_SELECTING)
    {
        return;
    }

    // Lock difficulty selection
    SetupPhase = SETUP_COUNTING_DOWN;

    SerialUART_WriteString("\r\n");
    SerialUART_WriteString(
        "DIFFICULTY LOCKED!\r\n");

    SerialUART_WriteString("Difficulty: ");
    SerialUART_WriteUInt(difficultyLED);

    SerialUART_WriteString(
        " | Chops required: ");
    SerialUART_WriteUInt(cutThreshold);

    SerialUART_WriteString("\r\n");
    SerialUART_WriteString(
        "Starting game in 3... 2... 1...\r\n");

    // Play 3-2-1-GO audio cue
    ES_Event_t AudioEvent = {
        ES_AUDIO_321GO, 0u
    };

    PostAudioService(AudioEvent);

    // Start the 3-second framework timer
    ES_Timer_InitTimer(
        COUNTDOWN_TIMER,
        COUNTDOWN_DURATION_MS
    );
}

// ===========================================
// START GAMEPLAY
// ===========================================

static void StartGameplay(void)
{
    if (SetupPhase != SETUP_COUNTING_DOWN)
    {
        return;
    }

    SetupPhase = SETUP_IN_GAME;

    SerialUART_WriteString("\r\n");
    SerialUART_WriteString("GO! GAME STARTED!\r\n");

    SerialUART_WriteString(
        "Game duration: 60 seconds\r\n");

    SerialUART_WriteString(
        "Required chops per cucumber: ");

    SerialUART_WriteUInt(cutThreshold);
    SerialUART_WriteString("\r\n");

    // Start main 60-second gameplay timer.
    // Timeout is routed to PlayService.
    ES_Timer_InitTimer(
        PLAY_TIMER,
        PLAY_DURATION_MS
    );

    // Tell PlayService to begin.
    // Pass difficulty-dependent cut threshold.
    ES_Event_t PlayEvent = {
        ES_PLAY, cutThreshold
    };

    PostPlayService(PlayEvent);

    // PlayService will:
    // - Read initial break-beam state
    // - Illuminate all six progress LEDs
    // - Start the timer-arm servo sweep
    // - Count valid chops
    // - Track earned treats
}

// ===========================================
// SERVICE INITIALIZATION
// ===========================================

bool InitSetupService(uint8_t Priority)
{
    MyPriority = Priority;

    ES_Event_t InitEvent = {
        ES_INIT, 0u
    };

    return ES_PostToService(
        MyPriority,
        InitEvent
    );
}

// ===========================================
// MAIN SERVICE EVENT HANDLER
// ===========================================

ES_Event_t RunSetupService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent = {
        ES_NO_EVENT, 0u
    };

    switch (ThisEvent.EventType)
    {
        // -----------------------------------
        // INITIALIZE SETUP
        // -----------------------------------

        case ES_INIT:
        {
            // Hold treat servo at center.
            MoveServo(2u, 50u);

            EnterSelection();
            break;
        }

        // -----------------------------------
        // RESET / START BUTTON
        // -----------------------------------

        case ES_RESET:
        case ES_START_BUTTON:
        {
            EnterSelection();
            break;
        }

        // -----------------------------------
        // IDLE
        // -----------------------------------

        case ES_IDLE:
        {
            // Reserved for future idle animations.
            // SetLEDsPulsing() is not yet
            // implemented as a brightness effect.
            break;
        }

        // -----------------------------------
        // POTENTIOMETER CHANGE
        // -----------------------------------

        case ES_POTENTIOMETER:
        {
            // Ignore potentiometer changes while
            // counting down or during gameplay.
            if (SetupPhase != SETUP_SELECTING)
            {
                break;
            }

            // EventParam is difficulty level 1-5,
            // NOT the raw ADC reading.
            uint8_t newLevel =
                (uint8_t)ThisEvent.EventParam;

            if (newLevel < 1u || newLevel > 5u)
            {
                break;
            }

            // Only update if difficulty changed.
            if (newLevel != difficultyLED)
            {
                SetDifficulty(newLevel);
            }

            break;
        }

        // -----------------------------------
        // HANDLE TOUCHED
        // -----------------------------------

        case ES_HANDLE_TOUCH:
        {
            // Begin countdown only from selection.
            StartCountdown();
            break;
        }

        // -----------------------------------
        // HANDLE RELEASED
        // -----------------------------------

        case ES_HANDLE_RELEASE:
        {
            // Cancel pregame countdown if released.
            if (SetupPhase == SETUP_COUNTING_DOWN)
            {
                ES_Timer_StopTimer(COUNTDOWN_TIMER);

                SetupPhase = SETUP_SELECTING;

                SerialUART_WriteString(
                    "Handle released!\r\n");

                SerialUART_WriteString(
                    "Starting countdown canceled.\r\n");

                SerialUART_WriteString(
                    "Touch handle to try again.\r\n");
            }

            // During gameplay, DeadmanHandling
            // manages the release behavior.
            break;
        }

        // -----------------------------------
        // FRAMEWORK TIMER EVENTS
        // -----------------------------------

        case ES_TIMEOUT:
        {
            if (ThisEvent.EventParam == COUNTDOWN_TIMER)
            {
                // Only start gameplay if the
                // countdown is still active.
                if (SetupPhase == SETUP_COUNTING_DOWN)
                {
                    StartGameplay();
                }
            }

            break;
        }

        default:
            break;
    }

    return ReturnEvent;
}

// ===========================================
// POST FUNCTION
// ===========================================

bool PostSetupService(ES_Event_t ThisEvent)
{
    return ES_PostToService(
        MyPriority,
        ThisEvent
    );
}
