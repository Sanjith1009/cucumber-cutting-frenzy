#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PlayService.h"
#include "SetupService.h"
#include "DeadmanHandling.h"
#include "TREATService.h"
#include "AudioService.h"
#include "LEDService.h"
#include "ServoControl.h"
#include "EventCheckers.h"
#include "SerialUART.h"

#define ARM_START_POSITION 90u
#define ARM_END_POSITION   10u
#define ARM_UPDATE_MS      50u

static uint8_t MyPriority;
static bool isPlaying = false;
static bool handleHeld = false;
static bool highEnough = false;
static uint16_t chopCounter = 0u;
static uint16_t treatCount = 0u;
static uint16_t cutThreshold = 5u;
static uint32_t armElapsedMs = 0u;
static uint8_t armPosition = ARM_END_POSITION;

static void StartTimerArm(void);
static void UpdateTimerArm(void);
static void StopTimerArm(void);
static void EndGame(void);

static void PostAudioCue(ES_EventType_t cue)
{
    ES_Event_t event = {cue, 0u};
    PostAudioService(event);
}

bool InitPlayService(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t initEvent = {ES_INIT, 0u};
    return ES_PostToService(MyPriority, initEvent);
}

ES_Event_t RunPlayService(ES_Event_t ThisEvent)
{
    ES_Event_t returnEvent = {ES_NO_EVENT, 0u};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
        {
            isPlaying = false;
            handleHeld = false;
            highEnough = false;
            chopCounter = 0u;
            treatCount = 0u;
            break;
        }

        case ES_PLAY:
        {
            isPlaying = true;
            // Countdown only finishes if the handle stayed held.
            handleHeld = true;
            highEnough = IsBeamUnbroken();
            chopCounter = 0u;
            treatCount = 0u;
            cutThreshold = ThisEvent.EventParam;
            if (cutThreshold < 5u || cutThreshold > 25u)
            {
                cutThreshold = 5u;
            }

            LEDService_ShowCutProgress(0u, cutThreshold);
            StartTimerArm();
            SerialUART_WriteString("Gameplay started, initial highEnough=");
            SerialUART_WriteUInt(highEnough);
            SerialUART_WriteString("\r\n");
            // SetupService owns starting PLAY_TIMER.
            break;
        }

        case ES_NOT_PLAYING:
        {
            isPlaying = false;
            handleHeld = false;
            highEnough = false;
            ES_Timer_StopTimer(PLAY_TIMER);
            StopTimerArm();
            break;
        }

        case ES_HANDLE_TOUCH:
        {
            if (isPlaying)
            {
                handleHeld = true;
                highEnough = IsBeamUnbroken();
            }
            break;
        }

        case ES_HANDLE_RELEASE:
        {
            handleHeld = false;
            highEnough = false;
            break;
        }

        case ES_BEAM_UNBROKEN:
        {
            if (isPlaying && handleHeld)
            {
                highEnough = true;
                SerialUART_WriteString("Play: BEAM_UNBROKEN -> highEnough=1\r\n");
            }
            else
            {
                SerialUART_WriteString("Play: BEAM_UNBROKEN ignored, playing=");
                SerialUART_WriteUInt(isPlaying);
                SerialUART_WriteString(" handleHeld=");
                SerialUART_WriteUInt(handleHeld);
                SerialUART_WriteString("\r\n");
            }
            break;
        }

        case ES_CHOP:
        {
            if (!isPlaying || !handleHeld)
            {
                SerialUART_WriteString("Play: CHOP ignored, playing=");
                SerialUART_WriteUInt(isPlaying);
                SerialUART_WriteString(" handleHeld=");
                SerialUART_WriteUInt(handleHeld);
                SerialUART_WriteString("\r\n");
                break;
            }

            if (!highEnough)
            {
                SerialUART_WriteString("Play: CHOP but highEnough=0 -> womp\r\n");
                PostAudioCue(ES_AUDIO_NOT_HIGH_ENOUGH);
                break;
            }
            SerialUART_WriteString("Play: CHOP counted\r\n");

            ++chopCounter;
            highEnough = false;

            if (chopCounter >= cutThreshold)
            {
                ++treatCount;
                chopCounter = 0u;
                LEDService_ShowCutProgress(0u, cutThreshold);
                LEDService_FlashForMs(1000u);
                PostAudioCue(ES_PLAY_FINISHED_AUDIO);
                SerialUART_WriteString("Cucumber completed! Treat count: ");
                SerialUART_WriteUInt(treatCount);
                SerialUART_WriteString("\r\n");
            }
            else
            {
                LEDService_ShowCutProgress(chopCounter, cutThreshold);
                PostAudioCue(ES_AUDIO_CHOP);
            }
            break;
        }

        case ES_TIMEOUT:
        {
            if (ThisEvent.EventParam == TIMER_ARM_TIMER)
            {
                UpdateTimerArm();
            }
            else if (ThisEvent.EventParam == PLAY_TIMER && isPlaying)
            {
                EndGame();
            }
            break;
        }

        default:
            break;
    }

    return returnEvent;
}

static void EndGame(void)
{
    isPlaying = false;
    handleHeld = false;
    highEnough = false;
    StopTimerArm();

    ES_Event_t disarm = {ES_NOT_PLAYING, 0u};
    PostDeadmanHandling(disarm);

    SerialUART_WriteString("Game over. Earned treats: ");
    SerialUART_WriteUInt(treatCount);
    SerialUART_WriteString("\r\n");

    PostAudioCue(ES_AUDIO_ENDGAME);

    // IMPORTANT: No immediate ES_RESET here. TREATService will post ES_RESET
    // after the final treat is dispensed (or immediately if treatCount is 0).
    
    
    ES_Event_t dispense = {ES_DISPENSE_TREATS, treatCount};
    if (!PostTREATService(dispense))
    {
        // Avoid a permanently stuck end-of-game state if the treat queue
        // is unexpectedly full. No treats are actuated in this fallback.
        ES_Event_t reset = {ES_RESET, 0u};
        PostSetupService(reset);
    }
}

static void StartTimerArm(void)
{
    armElapsedMs = 0u;
    armPosition = ARM_START_POSITION;
    MoveTimerArmServo(armPosition);
    ES_Timer_InitTimer(TIMER_ARM_TIMER, ARM_UPDATE_MS);
}

static void UpdateTimerArm(void)
{
    if (!isPlaying)
    {
        return;
    }

    armElapsedMs += ARM_UPDATE_MS;
    if (armElapsedMs > PLAY_DURATION_MS)
    {
        armElapsedMs = PLAY_DURATION_MS;
    }

    uint8_t nextPosition = (uint8_t)(ARM_START_POSITION -
        ((armElapsedMs * (ARM_START_POSITION - ARM_END_POSITION)) /
         PLAY_DURATION_MS));

    if (nextPosition != armPosition)
    {
        armPosition = nextPosition;
        MoveTimerArmServo(armPosition);
    }

    if (armElapsedMs < PLAY_DURATION_MS)
    {
        ES_Timer_InitTimer(TIMER_ARM_TIMER, ARM_UPDATE_MS);
    }
}

static void StopTimerArm(void)
{
    ES_Timer_StopTimer(TIMER_ARM_TIMER);
    armPosition = ARM_END_POSITION;
    MoveTimerArmServo(armPosition);
}

bool PostPlayService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}
