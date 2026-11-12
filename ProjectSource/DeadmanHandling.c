#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "../ProjectHeaders/DeadmanHandling.h"
#include "../ProjectHeaders/DM_Display.h"
#include <stdio.h>
#include <stdbool.h>

#include "SetupService.h"


// Module-level variable to store the service priority
static uint8_t MyPriority;
// Static variable to track the handle state (initialized as untouched)
static bool IsHandleTouched = false;


bool InitDeadmanHandling(uint8_t Priority)
{
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    ThisEvent.EventType = ES_INIT;
    ThisEvent.EventParam = 0;
    
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunDeadmanHandling(ES_Event_t ThisEvent)
{
    // Framework requires returning ES_NO_EVENT to avoid crashing the event loop
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};
    
    switch (ThisEvent.EventType)
    {
        case ES_HANDLE_TOUCHED:
            IsHandleTouched = true;
            break;

        case ES_HANDLE_RELEASED:
            if (IsHandleTouched == true)
            {
                // Set deadmanTimer
                ES_Timer_InitTimer(DEADMAN_TIMER, 3000);
                IsHandleTouched = false;
            }
            break;

        case ES_TIMEOUT:
            if (ThisEvent.EventParam == DEADMAN_TIMER)
            {
                if (IsHandleTouched == true)
                {
                    // Handle was touched again before the 3 seconds were up
                    // Do nothing
                }
                else
                {
                    // Post audio cue to audio service
                    ES_Event_t AudioEvent;
                    AudioEvent.EventType = ES_AUDIO_HANDLE_RELEASE_IDLE; 
                    AudioEvent.EventParam = 0;
                    ES_PostToService(AUDIO_SERVICE_PRIORITY, AudioEvent);

                    // Post reset event to SetupService
                    ES_Event_t ResetEvent;
                    ResetEvent.EventType = ES_RESET;
                    ResetEvent.EventParam = 0;
                    PostSetupService(ResetEvent);
                }
            }
            break;
    }
    return ReturnEvent;
}

bool PostDeadmanHandling(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}
