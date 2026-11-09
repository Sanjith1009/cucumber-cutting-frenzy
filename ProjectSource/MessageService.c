// This module
#include "../ProjectHeaders/MessageService.h"

// debugging printf()

// Hardware
#include <xc.h>
//#include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"

static uint8_t MyPriority;
static const char MessageBuffer[] = "The quick brown fox jumped over the lazy dog. Then, it waddled away.";
static uint8_t index = 0;
static uint8_t MessageLength = sizeof(MessageBuffer)/sizeof(MessageBuffer[0]) - 1;

bool InitMessageService(uint8_t Priority)
{
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    
    ThisEvent.EventType = ES_INIT;
    ThisEvent.EventParam = 0;
    
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunMessageService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
            ES_Timer_InitTimer(MESSAGE_TIMER, 1000);
        case ES_TIMEOUT:
            // Ensure we are responding to the correct timer
            if (ThisEvent.EventParam == MESSAGE_TIMER)
            {
                if (index > MessageLength)
                {
                    index = 0;
                }
                else
                {
                    ES_Event_t NextCharEvent;
                    NextCharEvent.EventType = ES_NEW_KEY; 
                    NextCharEvent.EventParam = MessageBuffer[index];
                    
                    index++;
                    
                    
                    PostLEDService(NextCharEvent);
                    
                    ES_Event_t Timer_reset;
                    Timer_reset.EventType = ES_INIT;
                    Timer_reset.EventParam = 0;
                    ES_PostToService(MyPriority, Timer_reset);
                }
            }
            break;
    }
    
    return ReturnEvent;
}



bool PostMessageService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}