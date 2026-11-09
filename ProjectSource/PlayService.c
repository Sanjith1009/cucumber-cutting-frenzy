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


// Module-level variable to store the service priority
static uint8_t MyPriority;
// Static variables to maintain state
static bool isPlaying = false;
static bool highEnough = false;
static uint16_t chopCounter = 0;
static uint16_t treatCount = 0;
static uint16_t cutThreshold = 5; // placeholder

void DecreaseLitLEDs(void);
void FlashAllLEDs(void);
void TurnOnAllLEDs(void);


bool InitPlayService(uint8_t Priority)
{
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    ThisEvent.EventType = ES_INIT;
    ThisEvent.EventParam = 0;
    
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunPlayService(ES_Event_t ThisEvent)
{
    // Framework requires returning ES_NO_EVENT to avoid crashing the event loop
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};
    
    switch (ThisEvent.EventType)
    {
        case ES_PLAY:
            isPlaying = true;
            break;

        case ES_BEAM_UNBROKEN:
            if (isPlaying == true)
            {
                highEnough = true;
            }
            break;

        case ES_CHOP:
            if (isPlaying == true)
            {
                if (highEnough == true)
                {
                    if (chopCounter < cutThreshold)
                    {
                        chopCounter++;
                        DecreaseLitLEDs(); // LED buffer -1 lit LED
                        
                        // Post chop audio cue to audio service
                        ES_Event_t AudioEvent;
                        AudioEvent.EventType = ES_AUDIO_CHOP;
                        AudioEvent.EventParam = 0;
                        ES_PostToService(AUDIO_SERVICE_PRIORITY, AudioEvent);
                    }
                    else
                    {
                        chopCounter = 0;
                        treatCount++;
                        
                        FlashAllLEDs();
                        TurnOnAllLEDs();
                        
                        // Post finishedOne audio cue to audio service
                        ES_Event_t AudioEvent;
                        AudioEvent.EventType = ES_PLAY_FINISHED_AUDIO;
                        AudioEvent.EventParam = 0;
                        ES_PostToService(AUDIO_SERVICE_PRIORITY, AudioEvent);
                    }
                    // Reset highEnough for the next chop
                    highEnough = false;
                }
                else
                {
                    // Post womp womp audio cue to audio service
                    ES_Event_t AudioEvent;
                    AudioEvent.EventType = ES_AUDIO_NOT_HIGH_ENOUGH;
                    AudioEvent.EventParam = 0;
                    ES_PostToService(AUDIO_SERVICE_PRIORITY, AudioEvent);
                }
            }
            break;

        case ES_TIMEOUT:
            if (ThisEvent.EventParam == PLAY_TIMER)
            {
                if (isPlaying == true)
                {
                    isPlaying = false; // Exit play state
                    FlashAllLEDs();
                    
                    // Post TREAT counter to treat service
                    ES_Event_t TreatEvent;
                    TreatEvent.EventType = ES_DISPENSE_TREATS;
                    TreatEvent.EventParam = treatCount; // Pass the counter via EventParam
                    ES_PostToService(TREAT_SERVICE_PRIORITY, TreatEvent);
                    
                    // Post end game audio cue to audio service
                    ES_Event_t AudioEvent;
                    AudioEvent.EventType = ES_AUDIO_ENDGAME;
                    AudioEvent.EventParam = 0;
                    ES_PostToService(AUDIO_SERVICE_PRIORITY, AudioEvent);
                    
                    // Post reset event to run setup service
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

bool PostPlayService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}
