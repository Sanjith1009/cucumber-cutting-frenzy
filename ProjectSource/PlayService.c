#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "../ProjectHeaders/DeadmanHandling.h"
#include "../ProjectHeaders/DM_Display.h"
#include "../ProjectHeaders/GameFSM.h"
#include <stdio.h>
#include <stdbool.h>

#include "SetupService.h"
#include "PlayService.h"
#include "AudioService.h"

// Hardware
#include <xc.h>
#include "PWM_PIC32.h"
#include "ServoControl.h"

// Timer-arm servo countdown (moved from GameFSM)
#define ARM_START_POSITION 90u   // servo position at the start of the game
#define ARM_END_POSITION   10u   // servo position when time is up
#define ARM_UPDATE_MS      50u   // how often the arm is stepped

static void StartTimerArm(void);
static void UpdateTimerArm(void);
static void StopTimerArm(void);

static uint32_t armElapsedMs = 0;   // time since the game started
static uint8_t  armPosition = ARM_END_POSITION;

// Module-level variable to store the service priority
static uint8_t MyPriority;
// Static variables to maintain state
static bool isPlaying = false;
static bool highEnough = false;
static uint16_t chopCounter = 0;
static uint16_t treatCount = 0;
static uint16_t cutThreshold = 5; // placeholder

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
            StartTimerArm();   // begin the timer-arm countdown sweep
            break;

        case ES_NOT_PLAYING:
            isPlaying = false;
            StopTimerArm();    // game aborted (e.g. deadman), park the arm
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
                        //DecreaseLitLEDs(); // LED buffer -1 lit LED
                        
                        // Post chop audio cue to audio service
                        ES_Event_t AudioEvent;
                        AudioEvent.EventType = ES_AUDIO_CHOP;
                        AudioEvent.EventParam = 0;
                        PostAudioService(AudioEvent);
                    }
                    else
                    {
                        chopCounter = 0;
                        treatCount++;
                        
                        //FlashAllLEDs();
                        //TurnOnAllLEDs();
                        
                        // Post finishedOne audio cue to audio service
                        ES_Event_t AudioEvent;
                        AudioEvent.EventType = ES_PLAY_FINISHED_AUDIO; //need to update when we update different audio cues
                        AudioEvent.EventParam = 0;
                        PostAudioService(AudioEvent);
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
                    PostAudioService(AudioEvent);
                }
            }
            break;

        case ES_TIMEOUT:
            if (ThisEvent.EventParam == TIMER_ARM_TIMER)
            {
                UpdateTimerArm();      // step the timer-arm servo
            }
            else if (ThisEvent.EventParam == PLAY_TIMER)
            {
                StopTimerArm();        // time's up, arm to end position
                if (isPlaying == true)
                {
                    isPlaying = false; // Exit play state
                    //FlashAllLEDs();
                    
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

            case ES_INIT:
                GameFSM_Init();
            break;
            
            case ES_NEW_KEY:
                GameFSM_HandleKey((char)ThisEvent.EventParam);
            break;
        // case ES_DISPENSE_TREATS:
        //     for (size_t i = 0; i < ThisEvent.EventParam; i++) {
        //         MoveTreatServo();
        //     }
        //     break;
    }
    return ReturnEvent;
}

bool PostPlayService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/*--------------------------- Timer-Arm Servo ------------------------------*/
// Start the countdown: arm to start position, begin stepping every 50 ms
static void StartTimerArm(void)
{
    armElapsedMs = 0;
    armPosition = ARM_START_POSITION;
    MoveTimerArmServo(armPosition);
    ES_Timer_InitTimer(TIMER_ARM_TIMER, ARM_UPDATE_MS);
}

// Called on each TIMER_ARM_TIMER timeout. Moves the arm linearly from
// ARM_START_POSITION to ARM_END_POSITION over PLAY_DURATION_MS.
static void UpdateTimerArm(void)
{
    if (!isPlaying)
    {
        return;   // stale tick after the game ended; don't restart
    }

    armElapsedMs += ARM_UPDATE_MS;
    if (armElapsedMs > PLAY_DURATION_MS)
    {
        armElapsedMs = PLAY_DURATION_MS;
    }

    uint8_t newPosition = (uint8_t)(ARM_START_POSITION -
        ((armElapsedMs * (ARM_START_POSITION - ARM_END_POSITION)) /
         PLAY_DURATION_MS));

    // Only command the servo when the position actually changes
    if (newPosition != armPosition)
    {
        armPosition = newPosition;
        MoveTimerArmServo(armPosition);
    }

    // Keep ticking until the arm reaches the end; PLAY_TIMER ends the game
    if (armElapsedMs < PLAY_DURATION_MS)
    {
        ES_Timer_InitTimer(TIMER_ARM_TIMER, ARM_UPDATE_MS);
    }
}

// Stop the countdown and park the arm at the end position
static void StopTimerArm(void)
{
    ES_Timer_StopTimer(TIMER_ARM_TIMER);
    armPosition = ARM_END_POSITION;
    MoveTimerArmServo(armPosition);
}
