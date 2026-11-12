#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "../ProjectHeaders/SetupService.h"
#include "../ProjectHeaders/DM_Display.h"
#include <stdio.h>
#include <stdbool.h>

#include "PlayService.h"
#include "Potentiometer.h"
#include "ServoControl.h"
#include "LEDService.h"


// Module-level variable to store the service priority
static uint8_t MyPriority;
static uint16_t chopCount = 0;
static uint16_t treatCount = 0;
static bool highEnough = false;
static uint16_t cutThreshold = 0;
static uint8_t difficultyLED = 0;

void MoveTimerArmServo(uint8_t position);
uint16_t CalculateDifficulty(uint16_t potValue);

bool InitSetupService(uint8_t Priority)
{
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    ThisEvent.EventType = ES_INIT;
    ThisEvent.EventParam = 0;
    
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunSetupService(ES_Event_t ThisEvent)
{
    // Framework requires returning ES_NO_EVENT to avoid crashing the event loop
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};
    
    switch (ThisEvent.EventType)
    {
        case ES_START_BUTTON:
        case ES_RESET:
            chopCount = 0;
            treatCount = 0;            
            MoveTimerArmServo(0);
                
            // Post idle event to itself
            ES_Event_t IdleEvent;
            IdleEvent.EventType = ES_IDLE;
            IdleEvent.EventParam = 0;
            ES_PostToService(MyPriority, IdleEvent);
            break;

        case ES_IDLE:
            SetLEDsPulsing();
            break;

        case ES_POTENTIOMETER:
            // Set selectTimer to 3 seconds (3000 ms)
            ES_Timer_InitTimer(SELECT_TIMER, 3000);
                
            // Set LED brightness PWM based on potentiometer
            // Assuming the potentiometer value is passed in the event parameter
            SetLEDBrightness(ThisEvent.EventParam);
            break;

        case ES_HANDLE_TOUCH:
            // Set countdownTimer (Assuming 3 seconds / 3000 ms for a 3-2-1 countdown)
            ES_Timer_InitTimer(COUNTDOWN_TIMER, 3000);
                
            // Post 3-2-1-GO event to audio service
            ES_Event_t AudioEvent;
            AudioEvent.EventType = ES_AUDIO_321GO;
            AudioEvent.EventParam = 0;
            ES_PostToService(AUDIO_SERVICE_PRIORITY, AudioEvent);
            
            difficultyLED = 0;
                
            // Reset deadmanTimer
            ES_Timer_InitTimer(DEADMAN_TIMER, 3000);
            break;

        case ES_TIMEOUT:
            // (idle state or selectTimer timeout)
            if (ThisEvent.EventParam == SELECT_TIMER)
            {
                SetLEDsPulsing();
            }
            
            // countdownTimer timeout (play start state)
            else if (ThisEvent.EventParam == COUNTDOWN_TIMER)
            {
                // Set playTimer to 60 seconds (60000 ms)
                ES_Timer_InitTimer(PLAY_TIMER, 60000);
                    
                highEnough = false;
                    
                // Threshold for number of cuts = read potentiometer and calculate 
                uint16_t currentPotValue = ReadPotentiometer();
                cutThreshold = CalculateDifficulty(currentPotValue);
                    
                // Move servo for timer arm to 0 position
                MoveTimerArmServo(0);
                    
                // Post play event to play service
                ES_Event_t PlayEvent;
                PlayEvent.EventType = ES_PLAY;
                PlayEvent.EventParam = 0;
                PostPlayService(PlayEvent);
            }
            break;
    }
    return ReturnEvent;
}

bool PostSetupService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

uint16_t CalculateDifficulty(uint16_t potValue)
{
    if (potValue > 1023)
    {
        potValue = 1023;
    }

    return 3 + ((uint32_t)potValue * 7) / 1023;
}
