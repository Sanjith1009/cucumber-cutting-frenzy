
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "AudioService.h"

static uint8_t MyPriority;
// TODO THIS IS CURRENTLY JUST A PLACEHOLDER IT NEEDS TO BE PROPERLY IMPLEMENTED LATER

bool InitAudioService(uint8_t Priority)
{
    MyPriority = Priority;

    // Initialize buzzer/PWM hardware here later

    ES_Event_t InitEvent = {ES_INIT, 0};

    return ES_PostToService(MyPriority, InitEvent);
}

bool PostAudioService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunAudioService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
        {
            // Audio hardware initialization
            break;
        }

        case ES_AUDIO_321GO:
        {
            // TODO: Play 3-2-1-GO countdown
            break;
        }

        case ES_AUDIO_CHOP:
        {
            // TODO: Play chopping sound
            break;
        }

        case ES_AUDIO_HANDLE_RELEASE_IDLE:
        {
            // TODO: Play handle-release warning
            break;
        }

        case ES_AUDIO_NOT_HIGH_ENOUGH:
        {
            // TODO: Play height warning
            break;
        }

        case ES_AUDIO_ENDGAME:
        {
            // TODO: Play end-game sound
            break;
        }

        default:
            break;
    }

    return ReturnEvent;
}
