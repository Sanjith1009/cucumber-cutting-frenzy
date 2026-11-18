#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "DeadmanHandling.h"
#include "PlayService.h"
#include "SetupService.h"
#include "AudioService.h"
#include "SerialUART.h"

#define DEADMAN_GRACE_MS 3000u

static uint8_t MyPriority;
static bool HandleTouched = false;
static bool GameActive = false;
static bool GraceTimerActive = false;

bool InitDeadmanHandling(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t initEvent = {ES_INIT, 0u};
    return ES_PostToService(MyPriority, initEvent);
}

ES_Event_t RunDeadmanHandling(ES_Event_t ThisEvent)
{
    ES_Event_t returnEvent = {ES_NO_EVENT, 0u};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
        {
            GameActive = false;
            GraceTimerActive = false;
            HandleTouched = false;
            ES_Timer_StopTimer(DEADMAN_TIMER);
            break;
        }

        case ES_PLAY:
        {
            // SetupService notifies us when the 3-2-1 countdown ends.
            GameActive = true;
            GraceTimerActive = false;
            ES_Timer_StopTimer(DEADMAN_TIMER);
            break;
        }

        case ES_NOT_PLAYING:
        case ES_RESET:
        {
            // Disarm the deadman timeout between games; keep the physical
            // touch state, since the player might still be holding the handle.
            GameActive = false;
            GraceTimerActive = false;
            ES_Timer_StopTimer(DEADMAN_TIMER);
            break;
        }

        case ES_HANDLE_TOUCH:
        {
            HandleTouched = true;
            GraceTimerActive = false;
            ES_Timer_StopTimer(DEADMAN_TIMER);

            if (GameActive)
            {
                // Restore chop eligibility, but not an extra chop credit.
                ES_Event_t event = {ES_HANDLE_TOUCH, 0u};
                PostPlayService(event);
            }
            break;
        }

        case ES_HANDLE_RELEASE:
        {
            HandleTouched = false;
            if (GameActive)
            {
                // Disallow chops immediately. The game clock continues
                // during Sanjith's 3-second re-grab grace period.
                ES_Event_t event = {ES_HANDLE_RELEASE, 0u};
                PostPlayService(event);
                GraceTimerActive = true;
                ES_Timer_InitTimer(DEADMAN_TIMER, DEADMAN_GRACE_MS);
            }
            break;
        }

        case ES_TIMEOUT:
        {
            if (ThisEvent.EventParam != DEADMAN_TIMER)
            {
                break;
            }
            if (GameActive && GraceTimerActive && !HandleTouched)
            {
                GraceTimerActive = false;
                GameActive = false;

                ES_Event_t stop = {ES_NOT_PLAYING, 0u};
                PostPlayService(stop);

                ES_Event_t sound = {ES_AUDIO_HANDLE_RELEASE_IDLE, 0u};
                PostAudioService(sound);

                ES_Event_t reset = {ES_RESET, 0u};
                PostSetupService(reset);
            }
            break;
        }

        default:
            break;
    }

    return returnEvent;
}

bool PostDeadmanHandling(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}
