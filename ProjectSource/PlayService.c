
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "PlayService.h"
#include "SetupService.h"
#include "AudioService.h"
#include "LEDService.h"
#include "ServoControl.h"
#include "EventCheckers.h"  // IsBeamUnbroken()

// we start with our timer arm configuration
// the timer arm positions still need to be fine tuned

#define ARM_START_POSITION 90u
#define ARM_END_POSITION   10u
#define ARM_UPDATE_MS      50u

// PLAY_DURATION_MS is defined in PlayService.h
// and should be 60000u (60 seconds).

// then we have our module level variables

static uint8_t MyPriority;

static bool isPlaying = false;
static bool highEnough = false;

static uint16_t chopCounter = 0u;
static uint16_t treatCount = 0u;
static uint16_t cutThreshold = 5u;

static uint32_t armElapsedMs = 0u;
static uint8_t armPosition = ARM_END_POSITION;

// ========================================
// PRIVATE FUNCTION DECLARATIONS
// ========================================

static void StartTimerArm(void);
static void UpdateTimerArm(void);
static void StopTimerArm(void);
static void EndGame(void);

// ========================================
// AUDIO HELPER
// ========================================

static void PostAudioCue(ES_EventType_t cue)
{
    ES_Event_t event = {cue, 0};
    PostAudioService(event);
}

// ========================================
// INITIALIZATION
// ========================================

bool InitPlayService(uint8_t Priority)
{
    MyPriority = Priority;

    ES_Event_t InitEvent = {ES_INIT, 0};

    return ES_PostToService(MyPriority, InitEvent);
}

// ========================================
// EVENT HANDLER
// ========================================

ES_Event_t RunPlayService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
        {
            isPlaying = false;
            highEnough = false;
            chopCounter = 0u;
            treatCount = 0u;
            break;
        }

        // --------------------------------
        // GAME START
        // --------------------------------

        case ES_PLAY:
        {
            isPlaying = true;

            chopCounter = 0u;
            treatCount = 0u;

            // Receive selected threshold from SetupService
            cutThreshold = (uint16_t)ThisEvent.EventParam;

            if (cutThreshold < 3u || cutThreshold > 10u)
            {
                cutThreshold = 3u;
            }

            // Determine initial knife position
            highEnough = IsBeamUnbroken();

            // Start with all six progress LEDs on
            LEDService_ShowCutProgress(0u, cutThreshold);

            // Begin 60-second timer-arm sweep
            StartTimerArm();

            // PLAY_TIMER is started by SetupService
            break;
        }

        // --------------------------------
        // GAME ABORTED
        // --------------------------------

        case ES_NOT_PLAYING:
        {
            isPlaying = false;
            highEnough = false;

            // Stop the main gameplay timer
            ES_Timer_StopTimer(PLAY_TIMER);

            // Stop the servo sweep
            StopTimerArm();

            break;
        }

        // --------------------------------
        // KNIFE RAISED / BEAM CLEAR
        // --------------------------------

        case ES_BEAM_UNBROKEN:
        {
            if (isPlaying)
            {
                highEnough = true;
            }
            break;
        }

        // --------------------------------
        // KNIFE CHOP
        // --------------------------------

        case ES_CHOP:
        {
            if (!isPlaying)
            {
                break;
            }

            // Knife must be raised between valid chops
            if (!highEnough)
            {
                PostAudioCue(ES_AUDIO_NOT_HIGH_ENOUGH);
                break;
            }

            // Count the successful chop
            chopCounter++;

            // Require beam rearming before next chop
            highEnough = false;

            // Check whether cucumber is complete
            if (chopCounter >= cutThreshold)
            {
                // One complete cucumber
                treatCount++;

                // Reset progress for next cucumber
                chopCounter = 0u;

                // Refill all six progress LEDs
                LEDService_ShowCutProgress(
                    0u, cutThreshold
                );

                PostAudioCue(ES_PLAY_FINISHED_AUDIO);
            }
            else
            {
                // Display progress after this chop
                LEDService_ShowCutProgress(
                    chopCounter,
                    cutThreshold
                );

                PostAudioCue(ES_AUDIO_CHOP);
            }

            break;
        }

        // --------------------------------
        // TIMER EVENTS
        // --------------------------------

        case ES_TIMEOUT:
        {
            if (ThisEvent.EventParam == TIMER_ARM_TIMER)
            {
                UpdateTimerArm();
            }
            else if (ThisEvent.EventParam == PLAY_TIMER)
            {
                if (isPlaying)
                {
                    EndGame();
                }
            }
            break;
        }

        default:
            break;
    }

    return ReturnEvent;
}

// ========================================
// END OF GAME
// ========================================

static void EndGame(void)
{
    isPlaying = false;
    highEnough = false;

    // Stop servo countdown and park arm
    StopTimerArm();

    // Send number of earned treats
    ES_Event_t TreatEvent = {
        ES_DISPENSE_TREATS, treatCount
    };

    ES_PostToService(
        TREAT_SERVICE_PRIORITY,
        TreatEvent
    );

    // Play endgame sound
    PostAudioCue(ES_AUDIO_ENDGAME);

    // Return to setup
    ES_Event_t ResetEvent = {
        ES_RESET, 0
    };

    PostSetupService(ResetEvent);
}

// ========================================
// TIMER-ARM SERVO CONTROL
// ========================================

static void StartTimerArm(void)
{
    armElapsedMs = 0u;
    armPosition = ARM_START_POSITION;

    MoveTimerArmServo(armPosition);

    ES_Timer_InitTimer(
        TIMER_ARM_TIMER,
        ARM_UPDATE_MS
    );
}

static void UpdateTimerArm(void)
{
    if (!isPlaying)
    {
        return;
    }

    // Advance time by one update interval
    armElapsedMs += ARM_UPDATE_MS;

    if (armElapsedMs > PLAY_DURATION_MS)
    {
        armElapsedMs = PLAY_DURATION_MS;
    }

    uint32_t travel =
        ARM_START_POSITION - ARM_END_POSITION;

    // Linear interpolation from start to end
    uint8_t newPosition = (uint8_t)(
        ARM_START_POSITION -
        ((armElapsedMs * travel) / PLAY_DURATION_MS)
    );

    if (newPosition != armPosition)
    {
        armPosition = newPosition;
        MoveTimerArmServo(armPosition);
    }

    // Continue updating until game finishes
    if (armElapsedMs < PLAY_DURATION_MS)
    {
        ES_Timer_InitTimer(
            TIMER_ARM_TIMER,
            ARM_UPDATE_MS
        );
    }
}

static void StopTimerArm(void)
{
    ES_Timer_StopTimer(TIMER_ARM_TIMER);

    armPosition = ARM_END_POSITION;
    MoveTimerArmServo(armPosition);
}

// ========================================
// POST FUNCTION
// ========================================

bool PostPlayService(ES_Event_t ThisEvent)
{
    return ES_PostToService(
        MyPriority, ThisEvent
    );
}