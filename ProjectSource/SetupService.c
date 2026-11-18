#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SetupService.h"
#include "PlayService.h"
#include "DeadmanHandling.h"
#include "TREATService.h"
#include "AudioService.h"
#include "Potentiometer.h"
#include "ServoControl.h"
#include "LEDService.h"
#include "SerialUART.h"

#define COUNTDOWN_DURATION_MS 3000u
#define TIMER_ARM_PARK_POS    10u

typedef enum
{
    SETUP_SELECTING,
    SETUP_COUNTING_DOWN,
    SETUP_IN_GAME
} SetupPhase_t;

static uint8_t MyPriority;
static SetupPhase_t SetupPhase = SETUP_SELECTING;
static uint8_t difficultyLED = 1u;
static uint16_t cutThreshold = 20u;

static uint8_t PotToDifficultyLevel(uint16_t adc)
{
    if (adc > 1023u)
    {
        adc = 1023u;
    }
    return 1u + (uint8_t)(((uint32_t)adc * 5u) / 1024u);
}


static uint16_t CalculateDifficulty(uint16_t level)
{
    // 20 chops per treat per level: level 1 = 20 ... level 5 = 100
    if (level < 1u)
    {
        level = 1u;
    }

    if (level > 5u)
    {
        level = 5u;
    }

    return 20u * level;
}


static void SetDifficulty(uint8_t level)
{
    if (level < 1u || level > 5u)
    {
        return;
    }
    difficultyLED = level;
    cutThreshold = CalculateDifficulty(level);
    LEDService_SetLitCount(level);
}

static void EnterSelection(void)
{
    ES_Timer_StopTimer(COUNTDOWN_TIMER);
    ES_Timer_StopTimer(PLAY_TIMER);

    // Disarm any gameplay and cancel dispensing on external reset.
    ES_Event_t stop = {ES_NOT_PLAYING, 0u};
    PostPlayService(stop);
    PostDeadmanHandling(stop);

    ES_Event_t cancelTreats = {ES_RESET, 0u};
    PostTREATService(cancelTreats);

    SetupPhase = SETUP_SELECTING;
    MoveTimerArmServo(TIMER_ARM_PARK_POS);

    SetDifficulty(PotToDifficultyLevel(ReadPotentiometer()));
}

static void StartCountdown(void)
{
    if (SetupPhase != SETUP_SELECTING)
    {
        return;
    }

    SetupPhase = SETUP_COUNTING_DOWN;

    ES_Event_t sound = {ES_AUDIO_321GO, 0u};
    PostAudioService(sound);
    ES_Timer_InitTimer(COUNTDOWN_TIMER, COUNTDOWN_DURATION_MS);
}

static void StartGameplay(void)
{
    if (SetupPhase != SETUP_COUNTING_DOWN)
    {
        return;
    }
    SetupPhase = SETUP_IN_GAME;

    ES_Timer_InitTimer(PLAY_TIMER, PLAY_DURATION_MS);

    ES_Event_t play = {ES_PLAY, cutThreshold};
    PostPlayService(play);
    ES_Event_t armDeadman = {ES_PLAY, 0u};
    PostDeadmanHandling(armDeadman);
}

bool InitSetupService(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t initEvent = {ES_INIT, 0u};
    return ES_PostToService(MyPriority, initEvent);
}

ES_Event_t RunSetupService(ES_Event_t ThisEvent)
{
    ES_Event_t returnEvent = {ES_NO_EVENT, 0u};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
        {
            MoveTreatServo(50u);
            EnterSelection();
            break;
        }

        case ES_RESET:
        case ES_START_BUTTON:
        {
            EnterSelection();
            break;
        }

        
        case ES_POTENTIOMETER:
        {
            if (SetupPhase == SETUP_SELECTING)
            {
                uint8_t level =
                    (uint8_t)ThisEvent.EventParam;

                if (level >= 1u &&
                    level <= 5u &&
                    level != difficultyLED)
                {
                    // Update required chops and LEDs
                    SetDifficulty(level);

                    // Play difficulty-change beep
                    ES_Event_t sound = {
                        ES_AUDIO_DIFFICULTY, 0u
                    };

                    PostAudioService(sound);
                }
            }

            break;
        }


        case ES_HANDLE_TOUCH:
        {
            StartCountdown();
            break;
        }

        case ES_HANDLE_RELEASE:
        {
            if (SetupPhase == SETUP_COUNTING_DOWN)
            {   
                ES_Timer_StopTimer(COUNTDOWN_TIMER);
                SetupPhase = SETUP_SELECTING;
                ES_Event_t stopAudio = {ES_AUDIO_STOP, 0u};
                PostAudioService(stopAudio);
            }
            // During gameplay, DeadmanHandling manages the grace period.
            break;
        }

        case ES_TIMEOUT:
        {
            if (ThisEvent.EventParam == COUNTDOWN_TIMER &&
                SetupPhase == SETUP_COUNTING_DOWN)
            {
                StartGameplay();
            }
            break;
        }

        default:
            break;
    }

    return returnEvent;
}

bool PostSetupService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}
