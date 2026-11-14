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
#include "../ProjectHeaders/ServoControl.h"
#include "LEDService.h"
#include "PIC32_AD_Lib.h"
#include "AudioService.h"

// Module-level variable to store the service priority
static uint8_t MyPriority;
static uint16_t chopCount = 0;
static uint16_t treatCount = 0;
static bool highEnough = false;
static uint16_t cutThreshold = 0;
static uint8_t difficultyLED = 0;

void MoveTimerArmServo(uint8_t position);
uint16_t CalculateDifficulty(uint16_t level);
static uint8_t PotToDifficultyLevel(uint16_t potValue);

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
        case ES_INIT:
            chopCount = 0;
            treatCount = 0;            
            MoveServo(2, 50);
                
            // Post idle event to itself
            ES_Event_t IdleEvent;
            IdleEvent.EventType = ES_IDLE;
            IdleEvent.EventParam = 0;
            ES_PostToService(MyPriority, IdleEvent);
            break;

        case ES_IDLE:
            //SetLEDsPulsing(); probably something with shift register
            break;

        case ES_POTENTIOMETER:
                
            // Set LED brightness PWM based on potentiometer
            // Assuming the potentiometer value is passed in the event parameter
            //SetLEDBrightness(ThisEvent.EventParam);

            // EventParam is the difficulty level 1-5 (from PotentiometerRead)
            difficultyLED = (uint8_t)ThisEvent.EventParam;
            cutThreshold = CalculateDifficulty(ThisEvent.EventParam);

            // for testing
            
            ES_Event_t testAudio;
            testAudio.EventType = ES_AUDIO_CHOP;
            testAudio.EventParam = 0;   // tune 0 (must be a valid tune index)
            PostAudioService(testAudio);
            break;

        case ES_HANDLE_TOUCH:
            // Set countdownTimer (Assuming 3 seconds / 3000 ms for a 3-2-1 countdown)
            ES_Timer_InitTimer(COUNTDOWN_TIMER, 3000);
                
            // Post 3-2-1-GO event to audio service
            ES_Event_t AudioEvent;
            AudioEvent.EventType = ES_AUDIO_321GO;
            AudioEvent.EventParam = 0;
            PostAudioService(AudioEvent);
            
            difficultyLED = 0;
            
            break;

        case ES_TIMEOUT:
            
            // countdownTimer timeout (play start state)
            if (ThisEvent.EventParam == COUNTDOWN_TIMER)
            {
                // Set playTimer to the game length (PLAY_DURATION_MS in PlayService.h)
                ES_Timer_InitTimer(PLAY_TIMER, PLAY_DURATION_MS);
                    
                highEnough = false;
                    
                // Threshold for number of cuts = read potentiometer and calculate 
                uint16_t currentPotValue = ReadPotentiometer();
                cutThreshold = CalculateDifficulty(
                    PotToDifficultyLevel(currentPotValue));
                    
                // Move servo for timer arm to 0 position
                MoveServo(2, 50);
                    
                // Post play event to play service
                ES_Event_t PlayEvent;
                PlayEvent.EventType = ES_PLAY;
                PlayEvent.EventParam = cutThreshold;
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

// Map difficulty level 1-5 to a cut threshold of 3-10 cuts
// (level 1 -> 3, 2 -> 4, 3 -> 6, 4 -> 8, 5 -> 10)
uint16_t CalculateDifficulty(uint16_t level)
{
    if (level < 1u)
    {
        level = 1u;
    }
    else if (level > 5u)
    {
        level = 5u;
    }

    return 3u + ((level - 1u) * 7u) / 4u;
}

// Convert raw pot reading (0-1023) to difficulty level 1-5.
// Must match the formula in PotentiometerRead() in EventCheckers.c
static uint8_t PotToDifficultyLevel(uint16_t potValue)
{
    return 1u + (uint8_t)(((uint32_t)potValue * 5u) / 1024u);
}
