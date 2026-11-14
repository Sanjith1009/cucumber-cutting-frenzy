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
// Buzzer PWM: OC4 on Timer3, output on RA2 (physical pin 9)
#define BUZZER_CHANNEL 4
#define BUZZER_TIMER   _Timer3_

// Duty cycle (%) while a note is sounding. 50% is loudest for a passive
// buzzer; lower it to make everything quieter.
#define BUZZER_VOLUME  50

#define ENTER_POST     ((MyPriority<<3)|0)
#define ENTER_RUN      ((MyPriority<<3)|1)
#define ENTER_TIMEOUT  ((MyPriority<<3)|2)

/*------------------------------ Module Types -----------------------------*/
// One note: frequency in Hz (0 = rest/silence) and duration in ms
typedef struct
{
    uint16_t freq;
    uint16_t ms;
} Note_t;

// One tune: pointer to its notes and how many notes it has
typedef struct
{
    const Note_t *notes;
    uint8_t length;
} Tune_t;

/*---------------------------- Module Functions ---------------------------*/
static void StartTune(const Tune_t *tune);
static void PlayCurrentNote(void);
static void StopSound(void);

/*------------------------------ Tune Data --------------------------------*/
// 3-2-1-GO: three beeps one second apart, then a long high GO.
// GO starts at 3.0 s to line up with SetupService's 3000 ms countdown timer.
static const Note_t Tune321Go[] = {
    {880, 200}, {0, 800},
    {880, 200}, {0, 800},
    {880, 200}, {0, 800},
    {1760, 600}
};

// Chop: quick low "thunk" - kept short so rapid chops don't blur together
static const Note_t TuneChop[] = {
    {300, 60}, {200, 60}
};

// Not high enough: sad descending "womp womp womp womp"
static const Note_t TuneNotHighEnough[] = {
    {392, 250}, {0, 50},
    {370, 250}, {0, 50},
    {349, 250}, {0, 50},
    {330, 600}
};

// Finished one: rising C5-E5-G5-C6 "ding"
static const Note_t TuneFinished[] = {
    {523, 120}, {659, 120}, {784, 120}, {1047, 300}
};

// Handle released / idle: two-tone alarm warning
static const Note_t TuneHandleRelease[] = {
    {1000, 150}, {700, 150},
    {1000, 150}, {700, 150},
    {1000, 150}, {700, 150}
};

// End game: fanfare
static const Note_t TuneEndGame[] = {
    {523, 150}, {659, 150}, {784, 150}, {1047, 400},
    {0, 100},
    {784, 150}, {1047, 600}
};

#define TUNE(notes) { notes, (uint8_t)(sizeof(notes) / sizeof(notes[0])) }

static const Tune_t TUNE_321GO          = TUNE(Tune321Go);
static const Tune_t TUNE_CHOP           = TUNE(TuneChop);
static const Tune_t TUNE_NOT_HIGH       = TUNE(TuneNotHighEnough);
static const Tune_t TUNE_FINISHED       = TUNE(TuneFinished);
static const Tune_t TUNE_HANDLE_RELEASE = TUNE(TuneHandleRelease);
static const Tune_t TUNE_ENDGAME        = TUNE(TuneEndGame);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[5 + 1];

static const Tune_t *CurrentTune = NULL;  // tune being played (NULL = silent)
static uint8_t NoteIndex = 0;             // which note of CurrentTune is playing

bool InitAudioService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // PWM channels were already configured in InitServoControl
    // (BasicConfig(4)). Do NOT call PWMSetup_BasicConfig here - it would
    // reset the servo channels.
    // Buzzer uses OC4 on its own timer (Timer3) so tone changes can't
    // affect the servos on Timer2.
    PWMSetup_AssignChannelToTimer(BUZZER_CHANNEL, BUZZER_TIMER);
    // Start with a 150 Hz tone frequency
    PWMSetup_SetFreqOnTimer(150, BUZZER_TIMER);
    // Keep the speaker silent initially
    PWMOperate_SetDutyOnChannel(0, BUZZER_CHANNEL);
    // Connect OC4 to RA2 (physical pin 9); also sets RA2 as a digital output
    PWMSetup_MapChannelToOutputPin(BUZZER_CHANNEL, PWM_RPA2);

    // initialize deferral queue for testing Deferal function
    ES_InitDeferralQueueWith(DeferralQueue, ARRAY_SIZE(DeferralQueue));

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    ThisEvent.EventParam = 0;
    return ES_PostToService(MyPriority, ThisEvent);
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

    // The event type selects the tune; EventParam is ignored.
    // A new sound always interrupts whatever is currently playing.
    switch (ThisEvent.EventType)
    {
        case ES_INIT:
            StopSound();
            break;

        case ES_AUDIO_321GO:
            StartTune(&TUNE_321GO);
            break;

        case ES_AUDIO_CHOP:
            StartTune(&TUNE_CHOP);
            break;

        case ES_AUDIO_NOT_HIGH_ENOUGH:
            StartTune(&TUNE_NOT_HIGH);
            break;

        case ES_PLAY_FINISHED_AUDIO:
            StartTune(&TUNE_FINISHED);
            break;

        case ES_AUDIO_HANDLE_RELEASE_IDLE:
            StartTune(&TUNE_HANDLE_RELEASE);
            break;

        case ES_AUDIO_ENDGAME:
            StartTune(&TUNE_ENDGAME);
            break;

        case ES_TIMEOUT:
            // Current note finished: advance to the next one or stop
            if ((ThisEvent.EventParam == AUDIO_TIMER) && (CurrentTune != NULL))
            {
                NoteIndex++;
                if (NoteIndex < CurrentTune->length)
                {
                    PlayCurrentNote();
                }
                else
                {
                    StopSound();
                }
            }
            break;

        default:
            break;
    }

    return ReturnEvent;
}

/*--------------------------- Private Functions ---------------------------*/
// Begin playing a tune from its first note (interrupts any current tune)
static void StartTune(const Tune_t *tune)
{
    CurrentTune = tune;
    NoteIndex = 0;
    PlayCurrentNote();
}

// Output the note at NoteIndex and start the timer for its duration
static void PlayCurrentNote(void)
{
    const Note_t *note = &CurrentTune->notes[NoteIndex];

    if (note->freq == 0u)
    {
        // Rest: silence the buzzer (SetFreqOnTimer rejects 0 Hz)
        PWMOperate_SetDutyOnChannel(0, BUZZER_CHANNEL);
    }
    else
    {
        // Set frequency first so the duty is calculated from the new period
        PWMSetup_SetFreqOnTimer(note->freq, BUZZER_TIMER);
        PWMOperate_SetDutyOnChannel(BUZZER_VOLUME, BUZZER_CHANNEL);
    }

    ES_Timer_InitTimer(AUDIO_TIMER, note->ms);
}

// Silence the buzzer and mark nothing as playing
static void StopSound(void)
{
    PWMOperate_SetDutyOnChannel(0, BUZZER_CHANNEL);
    ES_Timer_StopTimer(AUDIO_TIMER);
    CurrentTune = NULL;
    NoteIndex = 0;
}
