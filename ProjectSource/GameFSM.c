
#include <stdint.h>
#include <stdbool.h>

#include "GameFSM.h"
#include "LEDService.h"
#include "ServoControl.h"

#define MAX_CUTS 6u

static GameState_t CurrentState;
static uint8_t CutCount = 0u;
static bool BeamReady = false;

static void EnterState(GameState_t nextState)
{
    CurrentState = nextState;

    switch (CurrentState)
    {
        case GAME_WELCOME:
            // Welcome indication
            TurnOnAllLEDs();
            MoveTimerArmServo(20);
            break;

        case GAME_IDLE:
            // Four LEDs indicate idle
            TurnOnAllLEDs();
            DecreaseLitLEDs();
            DecreaseLitLEDs();
            MoveTimerArmServo(20);
            break;

        case GAME_COUNTDOWN:
            // Start nonblocking flashing
            FlashAllLEDs();
            break;

        case GAME_PLAY:
            // Prepare for a new round
            CutCount = 0u;
            BeamReady = false;
            TurnOnAllLEDs();
            MoveTimerArmServo(50);
            break;

        case GAME_DEADMAN:
            // Indicate handle release
            FlashAllLEDs();
            MoveTimerArmServo(20);
            break;

        default:
            break;
    }
}

void GameFSM_Init(void)
{
    CutCount = 0u;
    BeamReady = false;
    EnterState(GAME_WELCOME);
}

GameState_t GameFSM_GetState(void)
{
    return CurrentState;
}

void GameFSM_HandleEvent(GameEvent_t event)
{
    if (event == GAME_RESET)
    {
        EnterState(GAME_WELCOME);
        return;
    }

    switch (CurrentState)
    {
        case GAME_WELCOME:
            if (event == GAME_START)
            {
                EnterState(GAME_IDLE);
            }
            break;

        case GAME_IDLE:
            if (event == GAME_HANDLE_TOUCH)
            {
                EnterState(GAME_COUNTDOWN);
            }
            break;

        case GAME_COUNTDOWN:
            if (event == GAME_COUNTDOWN_DONE)
            {
                EnterState(GAME_PLAY);
            }
            else if (event == GAME_HANDLE_RELEASE)
            {
                EnterState(GAME_DEADMAN);
            }
            break;

        case GAME_PLAY:
            if (event == GAME_HANDLE_RELEASE)
            {
                EnterState(GAME_DEADMAN);
            }
            else if (event == GAME_PLAY_TIMEOUT)
            {
                EnterState(GAME_WELCOME);
            }
            else if (event == GAME_BEAM_CLEAR)
            {
                BeamReady = true;
            }
            else if (event == GAME_CHOP)
            {
                // A chop only counts after a beam cycle
                if (BeamReady && CutCount < MAX_CUTS)
                {
                    CutCount++;
                    DecreaseLitLEDs();
                    BeamReady = false;
                }
            }
            break;

        case GAME_DEADMAN:
            if (event == GAME_DEADMAN_DONE)
            {
                EnterState(GAME_IDLE);
            }
            break;

        default:
            break;
    }
}

void GameFSM_HandleKey(char key)
{
    switch (key)
    {
        case 's':
            GameFSM_HandleEvent(GAME_START);
            break;

        case 'h':
            GameFSM_HandleEvent(GAME_HANDLE_TOUCH);
            break;

        case 'g':
            GameFSM_HandleEvent(GAME_COUNTDOWN_DONE);
            break;

        case 'r':
            GameFSM_HandleEvent(GAME_HANDLE_RELEASE);
            break;

        case 'd':
            GameFSM_HandleEvent(GAME_DEADMAN_DONE);
            break;

        case 'b':
            GameFSM_HandleEvent(GAME_BEAM_CLEAR);
            break;

        case 'c':
            GameFSM_HandleEvent(GAME_CHOP);
            break;

        case 't':
            GameFSM_HandleEvent(GAME_PLAY_TIMEOUT);
            break;

        case 'x':
            GameFSM_HandleEvent(GAME_RESET);
            break;

        default:
            break;
    }
}
