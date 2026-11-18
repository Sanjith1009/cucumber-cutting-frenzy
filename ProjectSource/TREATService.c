#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TREATService.h"
#include "SetupService.h"
#include "ServoControl.h"
#include "SerialUART.h"

// These are ServoControl command units 0..100, NOT physical degrees.
// Start with the mechanism disconnected and calibrate both positions.
#define TREAT_SERVO_CLOSED_POS 50u
#define TREAT_SERVO_OPEN_POS   175u
#define TREAT_OPEN_MS         350u
#define TREAT_CLOSED_MS       400u
#define MAX_TREATS_PER_GAME   10u

typedef enum
{
    DISPENSER_IDLE,
    DISPENSER_OPEN,
    DISPENSER_CLOSED_WAIT
} DispenserState_t;

static uint8_t MyPriority;
static DispenserState_t State = DISPENSER_IDLE;
static uint16_t TreatsRemaining = 0u;

static void CloseDispenser(void)
{
    MoveTreatServo(TREAT_SERVO_CLOSED_POS);
}

static void FinishDispensing(void)
{
    ES_Timer_StopTimer(TREAT_TIMER);
    CloseDispenser();
    State = DISPENSER_IDLE;
    TreatsRemaining = 0u;

    SerialUART_WriteString("Dispensing complete - back to setup\r\n");
    ES_Event_t reset = {ES_RESET, 0u};
    PostSetupService(reset);
}

static void OpenNextTreat(void)
{
    if (TreatsRemaining == 0u)
    {
        FinishDispensing();
        return;
    }

    MoveTreatServo(TREAT_SERVO_OPEN_POS);
    State = DISPENSER_OPEN;
    ES_Timer_InitTimer(TREAT_TIMER, TREAT_OPEN_MS);
}

bool InitTREATService(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t initEvent = {ES_INIT, 0u};
    return ES_PostToService(MyPriority, initEvent);
}

ES_Event_t RunTREATService(ES_Event_t ThisEvent)
{
    ES_Event_t returnEvent = {ES_NO_EVENT, 0u};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
        case ES_RESET:
        {
            ES_Timer_StopTimer(TREAT_TIMER);
            TreatsRemaining = 0u;
            State = DISPENSER_IDLE;
            CloseDispenser();
            break;
        }

        case ES_DISPENSE_TREATS:
        {
            // Start a new end-of-game dispensing sequence.
            // Reset is posted to SetupService only after it finishes.
            ES_Timer_StopTimer(TREAT_TIMER);
            CloseDispenser();
            State = DISPENSER_IDLE;

            TreatsRemaining = ThisEvent.EventParam;
            if (TreatsRemaining > MAX_TREATS_PER_GAME)
            {
                TreatsRemaining = MAX_TREATS_PER_GAME;
            }

            SerialUART_WriteString("Treats to dispense: ");
            SerialUART_WriteUInt(TreatsRemaining);
            SerialUART_WriteString("\r\n");

            OpenNextTreat();
            break;
        }

        case ES_TIMEOUT:
        {
            if (ThisEvent.EventParam != TREAT_TIMER)
            {
                break;
            }

            if (State == DISPENSER_OPEN)
            {
                CloseDispenser();
                State = DISPENSER_CLOSED_WAIT;
                ES_Timer_InitTimer(TREAT_TIMER, TREAT_CLOSED_MS);
            }
            else if (State == DISPENSER_CLOSED_WAIT)
            {
                if (TreatsRemaining > 0u)
                {
                    --TreatsRemaining;
                }
                OpenNextTreat();
            }
            break;
        }

        default:
            break;
    }

    return returnEvent;
}

bool PostTREATService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}
