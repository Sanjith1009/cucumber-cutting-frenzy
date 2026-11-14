
#ifndef GAME_FSM_H
#define GAME_FSM_H

#include <stdint.h>

typedef enum
{
    GAME_WELCOME,
    GAME_IDLE,               // save for full game
    GAME_COUNTDOWN,          // save for full game
    GAME_PLAY,
    GAME_DEADMAN,            // save for full game
    GAME_SELECT_DIFFICULTY,
    GAME_OVER
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
    GAME_RESET,

    // New checkpoint event
    GAME_BEGIN_PLAY
} GameEvent_t;

// Initialize game state machine
void GameFSM_Init(void);

// Process a game event
void GameFSM_HandleEvent(GameEvent_t event);

// Process keyboard input from Serial Monitor
void GameFSM_HandleKey(char key);

// Update active state using current CP0 count
void GameFSM_Update(uint32_t currentTime);

// Get current state and difficulty
GameState_t GameFSM_GetState(void);
uint8_t GameFSM_GetDifficulty(void);

#endif
