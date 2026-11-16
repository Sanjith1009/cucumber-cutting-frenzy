
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "PWM_PIC32.h"

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "AudioService.h"

// ==========================================
// BUZZER HARDWARE
// ==========================================

// OC4 -> RA2, physical pin 9
// Hardware Timer3 generates the buzzer PWM.
// Hardware Timer2 remains dedicated to servos.

#define BUZZER_CHANNEL  4u
#define BUZZER_TIMER    _Timer3_

// 50% duty cycle while sounding
#define BUZZER_VOLUME   50u

// ==========================================
// SOUND PRIORITIES
// ==========================================

// A higher-priority sound cannot be
// interrupted by a lower-priority sound.

#define AUDIO_PRI_EFFECT      1u
#define AUDIO_PRI_WARNING     2u
#define AUDIO_PRI_COMPLETE    3u
#define AUDIO_PRI_COUNTDOWN   4u
#define AUDIO_PRI_GAMEOVER    5u

// ==========================================
// TUNE STRUCTURES
// ==========================================

// One note consists of a frequency in Hz
// and a duration in milliseconds.
//
// A frequency of zero means silence.

typedef struct
{
    uint16_t freq;
    uint16_t ms;
} Note_t;

// A tune consists of an array of notes
// and the number of notes in that array.

typedef struct
{
    const Note_t *notes;
    uint8_t length;
} Tune_t;

#define TUNE(notes) \
    {notes, (uint8_t)(sizeof(notes) / sizeof(notes[0]))}

// ==========================================
// TUNE DEFINITIONS
// ==========================================

// ------------------------------------------
// DIFFICULTY CHANGE
// One short beep whenever difficulty changes.
// ------------------------------------------

static const Note_t TuneDifficulty[] =
{
    {1200, 80}
};

// ------------------------------------------
// 3-2-1-GO COUNTDOWN
//
// Beep at 0, 1, and 2 seconds.
// GO begins at 2.8 seconds.
//
// Total duration: 3000 ms
// ------------------------------------------

static const Note_t Tune321Go[] =
{
    {880, 200}, {0, 800},
    {880, 200}, {0, 800},
    {880, 200}, {0, 600},
    {1760, 200}
};

// ------------------------------------------
// SUCCESSFUL CHOP
// Short two-note chopping sound.
// ------------------------------------------

static const Note_t TuneChop[] =
{
    {300, 60},
    {200, 60}
};

// ------------------------------------------
// KNIFE NOT HIGH ENOUGH
// Descending error sound.
// ------------------------------------------

static const Note_t TuneNotHighEnough[] =
{
    {392, 250}, {0, 50},
    {370, 250}, {0, 50},
    {349, 250}, {0, 50},
    {330, 600}
};

// ------------------------------------------
// CUCUMBER COMPLETED
// Rising celebratory tune.
// ------------------------------------------

static const Note_t TuneFinished[] =
{
    {523, 120},
    {659, 120},
    {784, 120},
    {1047, 300}
};

// ------------------------------------------
// HANDLE RELEASE / DEADMAN GAME OVER
// Warning tone.
// ------------------------------------------

static const Note_t TuneHandleRelease[] =
{
    {1000, 150},
    {700, 150},
    {1000, 150},
    {700, 150},
    {1000, 150},
    {700, 150}
};

// ------------------------------------------
// 60-SECOND GAME OVER
// Final fanfare.
// ------------------------------------------

static const Note_t TuneEndGame[] =
{
    {523, 150},
    {659, 150},
    {784, 150},
    {1047, 400},
    {0, 100},
    {784, 150},
    {1047, 600}
};

// ==========================================
// TUNE OBJECTS
// ==========================================

static const Tune_t TUNE_DIFFICULTY =
    TUNE(TuneDifficulty);

static const Tune_t TUNE_321GO =
    TUNE(Tune321Go);

static const Tune_t TUNE_CHOP =
    TUNE(TuneChop);

static const Tune_t TUNE_NOT_HIGH =
    TUNE(TuneNotHighEnough);

static const Tune_t TUNE_FINISHED =
    TUNE(TuneFinished);

static const Tune_t TUNE_HANDLE_RELEASE =
    TUNE(TuneHandleRelease);

static const Tune_t TUNE_ENDGAME =
    TUNE(TuneEndGame);

// ==========================================
// MODULE VARIABLES
// ==========================================

static uint8_t MyPriority;

static const Tune_t *CurrentTune = NULL;

static uint8_t NoteIndex = 0u;

static uint8_t CurrentSoundPriority = 0u;

// ==========================================
// PRIVATE FUNCTION DECLARATIONS
// ==========================================

static void StartTune(
    const Tune_t *tune,
    uint8_t priority
);

static void PlayCurrentNote(void);

static void StopSound(void);

// ==========================================
// AUDIO SERVICE INITIALIZATION
// ==========================================

bool InitAudioService(uint8_t Priority)
{
    MyPriority = Priority;

    CurrentTune = NULL;
    NoteIndex = 0u;
    CurrentSoundPriority = 0u;

    // ServoControl must already have called:
    //
    // PWMSetup_BasicConfig(4u);
    //
    // Do NOT call BasicConfig here.
    // Doing so would reset the servo channels.

    // Assign OC4 to hardware Timer3
    if (!PWMSetup_AssignChannelToTimer(
            BUZZER_CHANNEL, BUZZER_TIMER))
    {
        return false;
    }

    // Configure initial buzzer frequency
    if (!PWMSetup_SetFreqOnTimer(
            1000u, BUZZER_TIMER))
    {
        return false;
    }

    // Start with the buzzer silent
    if (!PWMOperate_SetDutyOnChannel(
            0u, BUZZER_CHANNEL))
    {
        return false;
    }

    // Map OC4 to RA2, physical pin 9
    if (!PWMSetup_MapChannelToOutputPin(
            BUZZER_CHANNEL, PWM_RPA2))
    {
        return false;
    }

    ES_Event_t InitEvent = {
        ES_INIT, 0u
    };

    return ES_PostToService(
        MyPriority, InitEvent
    );
}

// ==========================================
// POST AUDIO EVENT
// ==========================================

bool PostAudioService(ES_Event_t ThisEvent)
{
    return ES_PostToService(
        MyPriority, ThisEvent
    );
}

// ==========================================
// AUDIO SERVICE EVENT HANDLER
// ==========================================

ES_Event_t RunAudioService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent = {
        ES_NO_EVENT, 0u
    };

    switch (ThisEvent.EventType)
    {
        // ----------------------------------
        // INITIALIZE
        // ----------------------------------

        case ES_INIT:
        {
            StopSound();
            break;
        }

        // ----------------------------------
        // DIFFICULTY CHANGE
        // ----------------------------------

        case ES_AUDIO_DIFFICULTY:
        {
            StartTune(
                &TUNE_DIFFICULTY,
                AUDIO_PRI_EFFECT
            );
            break;
        }

        // ----------------------------------
        // 3-2-1-GO COUNTDOWN
        // ----------------------------------

        case ES_AUDIO_321GO:
        {
            StartTune(
                &TUNE_321GO,
                AUDIO_PRI_COUNTDOWN
            );
            break;
        }

        // ----------------------------------
        // SUCCESSFUL CHOP
        // ----------------------------------

        case ES_AUDIO_CHOP:
        {
            StartTune(
                &TUNE_CHOP,
                AUDIO_PRI_EFFECT
            );
            break;
        }

        // ----------------------------------
        // KNIFE NOT HIGH ENOUGH
        // ----------------------------------

        case ES_AUDIO_NOT_HIGH_ENOUGH:
        {
            StartTune(
                &TUNE_NOT_HIGH,
                AUDIO_PRI_WARNING
            );
            break;
        }

        // ----------------------------------
        // CUCUMBER COMPLETED
        // ----------------------------------

        case ES_PLAY_FINISHED_AUDIO:
        {
            StartTune(
                &TUNE_FINISHED,
                AUDIO_PRI_COMPLETE
            );
            break;
        }

        // ----------------------------------
        // HANDLE RELEASE GAME OVER
        // ----------------------------------

        case ES_AUDIO_HANDLE_RELEASE_IDLE:
        {
            StartTune(
                &TUNE_HANDLE_RELEASE,
                AUDIO_PRI_GAMEOVER
            );
            break;
        }

        // ----------------------------------
        // 60-SECOND GAME OVER
        // ----------------------------------

        case ES_AUDIO_ENDGAME:
        {
            StartTune(
                &TUNE_ENDGAME,
                AUDIO_PRI_GAMEOVER
            );
            break;
        }

        // ----------------------------------
        // STOP AUDIO
        // ----------------------------------

        case ES_AUDIO_STOP:
        {
            StopSound();
            break;
        }

        // ----------------------------------
        // NOTE TIMER EXPIRED
        // ----------------------------------

        case ES_TIMEOUT:
        {
            if (ThisEvent.EventParam == AUDIO_TIMER &&
                CurrentTune != NULL)
            {
                NoteIndex++;

                if (NoteIndex < CurrentTune->length)
                {
                    PlayCurrentNote();
                }
                else
                {
                    // All notes completed
                    StopSound();
                }
            }

            break;
        }

        default:
        {
            break;
        }
    }

    return ReturnEvent;
}

// ==========================================
// BEGIN PLAYING A TUNE
// ==========================================

static void StartTune(
    const Tune_t *tune,
    uint8_t priority
)
{
    if (tune == NULL || tune->length == 0u)
    {
        return;
    }

    // Do not interrupt a higher-priority
    // sound with a lower-priority sound.
    if (CurrentTune != NULL &&
        priority < CurrentSoundPriority)
    {
        return;
    }

    // Stop any previous sound and its timer
    StopSound();

    // Store the new tune
    CurrentTune = tune;
    CurrentSoundPriority = priority;
    NoteIndex = 0u;

    // Begin first note
    PlayCurrentNote();
}

// ==========================================
// PLAY CURRENT NOTE
// ==========================================

static void PlayCurrentNote(void)
{
    if (CurrentTune == NULL ||
        NoteIndex >= CurrentTune->length)
    {
        StopSound();
        return;
    }

    const Note_t *note =
        &CurrentTune->notes[NoteIndex];

    if (note->freq == 0u)
    {
        // Rest: silence the buzzer
        PWMOperate_SetDutyOnChannel(
            0u, BUZZER_CHANNEL
        );
    }
    else
    {
        // Change frequency on Timer3 only
        if (!PWMSetup_SetFreqOnTimer(
                note->freq, BUZZER_TIMER))
        {
            StopSound();
            return;
        }

        // Turn the note on
        PWMOperate_SetDutyOnChannel(
            BUZZER_VOLUME,
            BUZZER_CHANNEL
        );
    }

    // Wait for this note to finish.
    // This does not block other services.
    ES_Timer_InitTimer(
        AUDIO_TIMER,
        note->ms
    );
}

// ==========================================
// STOP ALL SOUND
// ==========================================

static void StopSound(void)
{
    // Disable buzzer output
    PWMOperate_SetDutyOnChannel(
        0u, BUZZER_CHANNEL
    );

    // Stop pending note timer
    ES_Timer_StopTimer(AUDIO_TIMER);

    CurrentTune = NULL;
    NoteIndex = 0u;
    CurrentSoundPriority = 0u;
}
