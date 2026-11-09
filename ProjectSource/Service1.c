#include "ES_Configure.h"
#include "ES_Framework.h"
#include "../ProjectHeaders/Service1.h"
#include <stdio.h>
#include <stdbool.h>

static uint8_t MyPriority;

bool InitService1(uint8_t Priority)
{
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    ThisEvent.EventType = ES_INIT;
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunService1(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent1;
    ES_Event_t ReturnEvent2;
    
    // Default return event if no specific conditions are met
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0}; 

    if (ThisEvent.EventType == ES_NEW_KEY)
    {
        ReturnEvent1.EventType = ES_NEW_EVENT1;
        ReturnEvent2.EventType = ES_NEW_EVENT2;

        if (ThisEvent.EventParam == 'a')
        {
            if (ES_PostToService(MyPriority, ReturnEvent1) == true)
            {
                DB_printf("ES_NEW_KEY 'a' pressed in Service 1\r");
                //return ReturnEvent1;
            }
        }
        else if (ThisEvent.EventParam == 'b')
        {
            if (ES_PostToService(MyPriority, ReturnEvent2) == true)
            {
                DB_printf("ES_NEW_KEY 'b' pressed in Service 1\r");
                //return ReturnEvent2;
            }
        }
    }
    
    return ReturnEvent;
}