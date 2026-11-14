
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "GameFSM.h"
#include "SerialUART.h"
#include "ShiftRegister.h"
#include "Potentiometer.h"
#include "ServoControl.h"

// ========================================
// CLOCK AND GAME CONFIGURATION
// ========================================

#define CP0_TICKS_PER_SECOND 20000000u

#define POT_UPDATE_TICKS     1000000u
#define SERVO_UPDATE_TICKS   1000000u
#define REPORT_TICKS        20000000u

#define GAME_DURATION_SECONDS 30u
#define GAME_DURATION_MS      30000u

#define GAME_DURATION_TICKS \
    (GAME_DURATION_SECONDS * CP0_TICKS_PER_SECOND)

// Previously tested servo command range
#define GAME_START_POSITION 90u
#define GAME_END_POSITION   10u

// ========================================
// MODULE-LEVEL VARIABLES
// ========================================

static GameState_t CurrentState = GAME_WELCOME;

static uint8_t Difficulty = 1u;
static uint8_t ServoPosition = GAME_END_POSITION;

static uint32_t GameStartTime = 0u;
static uint32_t LastPotUpdate = 0u;
static uint32_t LastServoUpdate = 0u;
static uint32_t LastReport = 0u;

// ========================================
// PRIVATE STATE FUNCTIONS
// ========================================

static void EnterWelcome(void)
{
    CurrentState = GAME_WELCOME;

    ServoPosition = GAME_END_POSITION;
    MoveTimerArmServo(ServoPosition);

    // All LEDs off
    WriteShiftRegister(0x00);

    SerialUART_WriteString("\r\n");
    SerialUART_WriteString(
        "============================\r\n");
    SerialUART_WriteString(
        "WELCOME TO CUCUMBER CUTTING FRENZY!\r\n");
    SerialUART_WriteString(
        "============================\r\n");
}

// Convert potentiometer reading to difficulty 1-5
static void UpdateDifficulty(void)
{
    uint16_t potValue = ReadPotentiometer();

    uint8_t newDifficulty =
        1u + (uint8_t)(
            ((uint32_t)potValue * 5u) / 1024u
        );

    if (newDifficulty != Difficulty)
    {
        Difficulty = newDifficulty;

        // LED 1 through LED Difficulty are ON
        uint8_t pattern =
            (uint8_t)((1u << Difficulty) - 1u);

        WriteShiftRegister(pattern);

        SerialUART_WriteString("Pot ADC: ");
        SerialUART_WriteUInt(potValue);

        SerialUART_WriteString(" | Difficulty: ");
        SerialUART_WriteUInt(Difficulty);
        SerialUART_WriteString(" / 5\r\n");
    }
}

static void EnterDifficultySelection(void)
{
    CurrentState = GAME_SELECT_DIFFICULTY;

    SerialUART_WriteString("\r\n");
    SerialUART_WriteString(
        "DIFFICULTY SELECTION\r\n");
    SerialUART_WriteString(
        "Rotate potentiometer to select 1-5.\r\n");
    SerialUART_WriteString(
        "LEDs indicate selected difficulty.\r\n");
    SerialUART_WriteString(
        "Press 'g' to begin the game.\r\n\r\n");

    // Force the initial LED and serial update
    Difficulty = 0u;
    UpdateDifficulty();

    LastPotUpdate = _CP0_GET_COUNT();
}

static void EnterPlaying(void)
{
    CurrentState = GAME_PLAY;

    // Start timer-arm servo at full time
    ServoPosition = GAME_START_POSITION;
    MoveTimerArmServo(ServoPosition);

    // Preserve difficulty LEDs
    uint8_t pattern =
        (uint8_t)((1u << Difficulty) - 1u);

    WriteShiftRegister(pattern);

    GameStartTime = _CP0_GET_COUNT();
    LastServoUpdate = GameStartTime;
    LastReport = GameStartTime;

    SerialUART_WriteString("\r\n");
    SerialUART_WriteString(
        "============================\r\n");
    SerialUART_WriteString("GAME STARTED!\r\n");
    SerialUART_WriteString(
        "============================\r\n");

    SerialUART_WriteString("Difficulty: ");
    SerialUART_WriteUInt(Difficulty);

    SerialUART_WriteString("\r\nGame time: ");
    SerialUART_WriteUInt(GAME_DURATION_SECONDS);
    SerialUART_WriteString(" seconds\r\n");

    SerialUART_WriteString("Servo start command: ");
    SerialUART_WriteUInt(ServoPosition);
    SerialUART_WriteString("\r\n");

    SerialUART_WriteString(
        "Countdown started!\r\n");
}

static void EnterGameOver(void)
{
    CurrentState = GAME_OVER;

    // Move servo to end position
    ServoPosition = GAME_END_POSITION;
    MoveTimerArmServo(ServoPosition);

    // Turn LEDs off
    WriteShiftRegister(0x00);

    SerialUART_WriteString("\r\n");
    SerialUART_WriteString(
        "============================\r\n");
    SerialUART_WriteString(
        "TIME'S UP! GAME OVER!\r\n");
    SerialUART_WriteString(
        "============================\r\n");

    SerialUART_WriteString(
        "Press 'r' to return to welcome.\r\n");
}

// ========================================
// PUBLIC STATE MACHINE INTERFACE
// ========================================

void GameFSM_Init(void)
{
    Difficulty = 1u;
    ServoPosition = GAME_END_POSITION;

    GameStartTime = 0u;
    LastPotUpdate = 0u;
    LastServoUpdate = 0u;
    LastReport = 0u;

    EnterWelcome();
}

GameState_t GameFSM_GetState(void)
{
    return CurrentState;
}

uint8_t GameFSM_GetDifficulty(void)
{
    return Difficulty;
}

void GameFSM_HandleEvent(GameEvent_t event)
{
    // Reset from any state
    if (event == GAME_RESET)
    {
        EnterWelcome();
        return;
    }

    switch (CurrentState)
    {
        case GAME_WELCOME:

            if (event == GAME_START)
            {
                EnterDifficultySelection();
            }
            break;

        case GAME_SELECT_DIFFICULTY:

            if (event == GAME_BEGIN_PLAY)
            {
                EnterPlaying();
            }
            break;

        case GAME_PLAY:

            if (event == GAME_PLAY_TIMEOUT)
            {
                EnterGameOver();
            }
            break;

        case GAME_OVER:
            // Wait for reset
            break;

        // Reserved for later game integration
        case GAME_IDLE:
        case GAME_COUNTDOWN:
        case GAME_DEADMAN:
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

        case 'g':
            GameFSM_HandleEvent(GAME_BEGIN_PLAY);
            break;

        case 'r':
        case 'x':
            GameFSM_HandleEvent(GAME_RESET);
            break;

        default:
            break;
    }
}

// ========================================
// PERIODIC STATE UPDATE
// ========================================

void GameFSM_Update(uint32_t now)
{
    switch (CurrentState)
    {
        case GAME_WELCOME:
            // Wait for start command
            break;

        case GAME_SELECT_DIFFICULTY:

            if ((uint32_t)(now - LastPotUpdate)
                >= POT_UPDATE_TICKS)
            {
                LastPotUpdate = now;
                UpdateDifficulty();
            }
            break;

        case GAME_PLAY:
        {
            uint32_t elapsedTicks =
                (uint32_t)(now - GameStartTime);

            // End after 30 seconds
            if (elapsedTicks >= GAME_DURATION_TICKS)
            {
                GameFSM_HandleEvent(GAME_PLAY_TIMEOUT);
                break;
            }

            // Update servo every 50 ms
            if ((uint32_t)(now - LastServoUpdate)
                >= SERVO_UPDATE_TICKS)
            {
                LastServoUpdate = now;

                uint32_t elapsedMs =
                    elapsedTicks / 20000u;

                uint32_t travel =
                    GAME_START_POSITION -
                    GAME_END_POSITION;

                uint8_t newPosition =
                    (uint8_t)(
                        GAME_START_POSITION -
                        ((elapsedMs * travel) /
                         GAME_DURATION_MS)
                    );

                if (newPosition != ServoPosition)
                {
                    ServoPosition = newPosition;
                    MoveTimerArmServo(ServoPosition);
                }
            }

            // Print remaining time every second
            if ((uint32_t)(now - LastReport)
                >= REPORT_TICKS)
            {
                LastReport = now;

                uint32_t remainingTicks =
                    GAME_DURATION_TICKS - elapsedTicks;

                uint32_t secondsRemaining =
                    (remainingTicks +
                     CP0_TICKS_PER_SECOND - 1u) /
                    CP0_TICKS_PER_SECOND;

                SerialUART_WriteString(
                    "Time remaining: ");
                SerialUART_WriteUInt(secondsRemaining);

                SerialUART_WriteString(
                    " s | Servo command: ");
                SerialUART_WriteUInt(ServoPosition);

                SerialUART_WriteString(
                    " | Difficulty: ");
                SerialUART_WriteUInt(Difficulty);

                SerialUART_WriteString("\r\n");
            }
            break;
        }

        case GAME_OVER:
            // Wait for reset
            break;

        // Reserved for future game states
        case GAME_IDLE:
        case GAME_COUNTDOWN:
        case GAME_DEADMAN:
        default:
            break;
    }
}
