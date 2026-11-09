#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "../ProjectHeaders/LEDService.h"
#include "../ProjectHeaders/DM_Display.h"
#include <stdio.h>
#include <stdbool.h>


// Module-level variable to store the service priority
static uint8_t MyPriority;

bool InitLEDService(uint8_t Priority)
{
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    ThisEvent.EventType = ES_INIT;
    ThisEvent.EventParam = 0;
    
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunLEDService(ES_Event_t ThisEvent)
{
    // Framework requires returning ES_NO_EVENT to avoid crashing the event loop
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};
    
    switch (ThisEvent.EventType)
    {
        case ES_INIT:
            if (DM_TakeInitDisplayStep() == false)
            {
                ES_PostToService(MyPriority, ThisEvent); 
            }
            break;
        case ES_NEW_KEY:
            DM_ScrollDisplayBuffer(4);
            DM_AddChar2DisplayBuffer(ThisEvent.EventParam);
            if (DM_TakeDisplayUpdateStep() == false) 
            {
                ES_Event_t UpdateStep;
                UpdateStep.EventType = ES_Update_Step;
                UpdateStep.EventParam = 0;
                ES_PostToService(MyPriority, UpdateStep); 
            } else {
                ES_Event_t UpdateEvent;
                UpdateEvent.EventType = ES_Update_Complete;
                UpdateEvent.EventParam = 0;
                ES_PostToService(MyPriority, UpdateEvent);
            }
            break;
        case ES_Update_Step:
            if (DM_TakeDisplayUpdateStep() == false) 
            {
                ES_PostToService(MyPriority, ThisEvent); 
            } else {
                ES_Event_t UpdateEvent;
                UpdateEvent.EventType = ES_Update_Complete;
                UpdateEvent.EventParam = 0;
                ES_PostToService(MyPriority, UpdateEvent);
            }
            break;
            
            
            
        case ES_Update_Complete:
            DB_printf("\rUpdate Complete, display HAL ready to update%d\r\n", MyPriority);
            break;
    }
    
    return ReturnEvent;
}

bool PostLEDService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}
