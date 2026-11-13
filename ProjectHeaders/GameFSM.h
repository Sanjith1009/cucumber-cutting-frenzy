
#ifndef GAME_FSM_H
#define GAME_FSM_H

#include <stdint.h>

typedef enum
{
    GAME_WELCOME,
    GAME_IDLE,
    GAME_COUNTDOWN,
    GAME_PLAY,
    GAME_DEADMAN
} GameState_t;

typedef enum
{
    GAME_START,
    GAME_HANDLE_TOUCH,
    GAME_HANDLE_RELEASE,
    GAME_COUNTDOWN_DONE,
    GAME_DEADMAN_DONE,
    GAME_BEAM_CLEAR,
    GAME_CHOP,
    GAME_PLAY_TIMEOUT,
    GAME_RESET
} GameEvent_t;

void GameFSM_Init(void);
void GameFSM_HandleEvent(GameEvent_t event);
void GameFSM_HandleKey(char key);
GameState_t GameFSM_GetState(void);

#endif
