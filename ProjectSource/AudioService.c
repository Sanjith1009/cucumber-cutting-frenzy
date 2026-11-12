// Hardware
#include <xc.h>
#include "PWM_PIC32.h"
//#include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"

#include "AudioService.h"
/*----------------------------- Module Defines ----------------------------*/
// these times assume a 10.000mS/tick timing
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define TWO_SEC (ONE_SEC * 2)
#define FIVE_SEC (ONE_SEC * 5)

#define ENTER_POST     ((MyPriority<<3)|0)
#define ENTER_RUN      ((MyPriority<<3)|1)
#define ENTER_TIMEOUT  ((MyPriority<<3)|2)

//#define TEST_INT_POST
//#define BLINK LED
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

// static void tone(int pitch, int duration, int volume);
static void PlayCurrentNote(void);


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[5 + 1];

// little data strucutre for tones (for now)
// frequency, duration, volume
static int toneTable[6][3] = {
  {100, 500, 0},
  {150, 500, 10},
  {200, 500, 10},
  {250, 500, 10},
  {300, 500, 10},
  {800, 500, 10}
};

static int tuneTable[2][8]={
  {7, 1,0,1,0,1,0,5},
  {3,1,0,4, 0, 0,0,0}
};
static int tuneIndex = 0;
static int toneIndex = 1;
static bool playing = false;

// TODO THIS IS CURRENTLY JUST A PLACEHOLDER IT NEEDS TO BE PROPERLY IMPLEMENTED LATER

bool InitAudioService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    /********************************************
    in here you write your initialization code
    *******************************************/

    // Configure one PWM channel: OC1
    PWMSetup_BasicConfig(1);
    // Use Timer2 for the audio frequency
    PWMSetup_AssignChannelToTimer(1, _Timer2_);
    // Start with a 2 kHz tone frequency
    PWMSetup_SetFreqOnTimer(150, _Timer2_);
    // Keep the speaker silent initially
    PWMOperate_SetDutyOnChannel(0, 1);
    // Configure physical pin 11, RB4, as an output
    TRISBbits.TRISB4 = 0;
    // Connect OC1 to RB4; map the pin last
    PWMSetup_MapChannelToOutputPin(1, PWM_RPB4);

    // initialize deferral queue for testing Deferal function
    ES_InitDeferralQueueWith(DeferralQueue, ARRAY_SIZE(DeferralQueue));

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool PostAudioService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunAudioService(ES_Event_t ThisEvent)
{

    
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};
    #ifdef _INCLUDE_BYTE_DEBUG_
        _HW_ByteDebug_SetValueWithStrobe( ENTER_RUN );
    #endif  
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
            if (playing)
            {
                break;
            }

            unsigned int requestedTune = ThisEvent.EventParam;
            unsigned int numTunes = sizeof(tuneTable) / sizeof(tuneTable[0]);

            if (requestedTune >= numTunes)
            {
                DB_printf("Invalid tune number\r\n");
                break;
            }

            tuneIndex = requestedTune;
            toneIndex = 1; // Skip the length entry
            playing = true;
            PlayCurrentNote();
        
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

        case ES_TIMEOUT:
        {
            if (ThisEvent.EventParam == AUDIO_TIMER && playing)
            {
                toneIndex++;

                if (toneIndex <= tuneTable[tuneIndex][0])
                {
                    PlayCurrentNote();
                }
                else
                {
                    PWMOperate_SetDutyOnChannel(0, 1);
                    playing = false;
                    toneIndex = 0;
                }
            }
            break;
        }
    

        default:
            break;
    }

    return ReturnEvent;
}

static void PlayCurrentNote(void)
{
    int note = tuneTable[tuneIndex][toneIndex];

    PWMSetup_SetFreqOnTimer(toneTable[note][0], _Timer2_);
    PWMOperate_SetDutyOnChannel(toneTable[note][2], 1);
    ES_Timer_InitTimer(AUDIO_TIMER, toneTable[note][1]);
}