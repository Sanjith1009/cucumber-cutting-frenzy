
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TREATService.h"

static uint8_t MyPriority;
// TODO THIS IS CURRENTLY JUST A PLACEHOLDER TO GET OUR STUFF TO COMPULE IT NEEDS TO BE IMPLEMENTED LATER

bool InitTREATService(uint8_t Priority)
{
    MyPriority = Priority;

    // TODO: Initialize treat dispenser servo

    ES_Event_t InitEvent = {ES_INIT, 0};

    return ES_PostToService(MyPriority, InitEvent);
}

bool PostTREATService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunTREATService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
        {
            // TODO: Set dispenser servo to closed
            break;
        }

        case ES_DISPENSE_TREATS:
        {
            // TODO: Open treat dispenser servo
            // TODO: Schedule servo closing

            break;
        }

        case ES_RESET:
        {
            // TODO: Return dispenser servo to
            // its closed position
            break;
        }

        default:
            break;
    }

    return ReturnEvent;
}
