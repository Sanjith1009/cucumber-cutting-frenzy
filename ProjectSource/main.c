
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

#include "../ProjectHeaders/ShiftRegister.h"
#include "../ProjectHeaders/ServoControl.h"
#include "../ProjectHeaders/Potentiometer.h"

// Assumes SYSCLK = 40 MHz, PBCLK = 20 MHz
#define CP0_TICKS_PER_SECOND 20000000u

#define POT_UPDATE_TICKS     1000000u  // 50 ms
#define SERVO_UPDATE_TICKS   1000000u  // 50 ms
#define REPORT_TICKS        20000000u  // 1 second

#define GAME_DURATION_SECONDS 30u
#define GAME_DURATION_MS (GAME_DURATION_SECONDS * 1000u)
#define GAME_DURATION_TICKS \
    (GAME_DURATION_SECONDS * CP0_TICKS_PER_SECOND)

#define GAME_START_POSITION 90u
#define GAME_END_POSITION   10u

// ====================================================
// GAME STATES
// ====================================================

typedef enum
{
    STATE_WELCOME,
    STATE_SELECT_DIFFICULTY,
    STATE_PLAYING,
    STATE_GAME_OVER
} GameState_t;

static GameState_t CurrentState = STATE_WELCOME;

static uint8_t Difficulty = 1u;
static uint8_t ServoPosition = GAME_START_POSITION;

static uint32_t GameStartTime = 0u;
static uint32_t LastPotUpdate = 0u;
static uint32_t LastServoUpdate = 0u;
static uint32_t LastReport = 0u;

// ====================================================
// PIN CONFIGURATION
// ====================================================

static void ConfigurePinMappings(void)
{
    uint32_t previousStatus =
        __builtin_disable_interrupts();

    SYSKEY = 0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    CFGCONbits.IOLOCK = 0;

    // SPI2 data: RB6, physical pin 15
    RPB6R = 0b0100;   // SDO2

    // UART2
    U2RXR = 0b0100;   // RB8, pin 17
    RPB9R = 0b0010;   // RB9, pin 18

    // Servo PWM
    RPB3R = 0b0101;   // OC1, pin 7
    RPB5R = 0b0101;   // OC2, pin 14

    CFGCONbits.IOLOCK = 1;
    SYSKEY = 0;

    if (previousStatus & 1u)
    {
        __builtin_enable_interrupts();
    }
}

// ====================================================
// UART2
// ====================================================

static void UART2_Init(void)
{
    U2MODE = 0;
    U2STA = 0;

    ANSELBCLR = (1u << 8) | (1u << 9);
    TRISBSET = (1u << 8);
    TRISBCLR = (1u << 9);

    // 9600 baud, 8N1
    U2MODEbits.BRGH = 0;
    U2MODEbits.PDSEL = 0;
    U2MODEbits.STSEL = 0;
    U2BRG = 129;

    U2MODEbits.ON = 1;
    U2STAbits.UTXEN = 1;
    U2STAbits.URXEN = 1;
}

static void SerialWriteChar(char c)
{
    while (U2STAbits.UTXBF)
    {
    }

    U2TXREG = c;
}

static void SerialWriteString(const char *str)
{
    while (*str != '\0')
    {
        SerialWriteChar(*str++);
    }
}

static void SerialWriteUInt(uint32_t value)
{
    char digits[11];
    uint8_t count = 0u;

    do
    {
        digits[count++] =
            (char)('0' + (value % 10u));

        value /= 10u;
    }
    while (value > 0u);

    while (count > 0u)
    {
        SerialWriteChar(digits[--count]);
    }
}

// ====================================================
// GAME STATE FUNCTIONS
// ====================================================

static void EnterWelcome(void)
{
    CurrentState = STATE_WELCOME;

    // Turn off the LEDs
    WriteShiftRegister(0x00);

    SerialWriteString("\r\n");
    SerialWriteString("============================\r\n");
    SerialWriteString("WELCOME TO CUCUMBER CUTTING FRENZY!\r\n");
    SerialWriteString("============================\r\n");
    SerialWriteString("Press 's' to start setup.\r\n");
}

static void UpdateDifficulty(void)
{
    uint16_t potValue = ReadPotentiometer();

    // Convert ADC range 0-1023 into levels 1-5
    uint8_t newDifficulty =
        1u + (uint8_t)(((uint32_t)potValue * 5u)
                       / 1024u);

    if (newDifficulty != Difficulty)
    {
        Difficulty = newDifficulty;

        // Light LEDs 1 through Difficulty
        uint8_t ledPattern =
            (uint8_t)((1u << Difficulty) - 1u);

        WriteShiftRegister(ledPattern);

        SerialWriteString("Pot ADC: ");
        SerialWriteUInt(potValue);

        SerialWriteString(" | Difficulty: ");
        SerialWriteUInt(Difficulty);
        SerialWriteString(" / 5\r\n");
    }
}

static void EnterDifficultySelection(void)
{
    CurrentState = STATE_SELECT_DIFFICULTY;

    SerialWriteString("\r\n");
    SerialWriteString("DIFFICULTY SELECTION\r\n");
    SerialWriteString("Rotate the potentiometer to choose 1-5.\r\n");
    SerialWriteString("LEDs indicate selected difficulty.\r\n");
    SerialWriteString("Press 'g' to begin the game.\r\n\r\n");

    // Force an initial display and serial report
    Difficulty = 0u;
    UpdateDifficulty();

    LastPotUpdate = _CP0_GET_COUNT();
}

static void EnterPlaying(void)
{
    CurrentState = STATE_PLAYING;

    // Set timer arm to full game time
    ServoPosition = GAME_START_POSITION;
    MoveTimerArmServo(ServoPosition);

    // Keep the selected difficulty LEDs lit
    WriteShiftRegister(
        (uint8_t)((1u << Difficulty) - 1u)
    );

    GameStartTime = _CP0_GET_COUNT();
    LastServoUpdate = GameStartTime;
    LastReport = GameStartTime;

    SerialWriteString("\r\n");
    SerialWriteString("============================\r\n");
    SerialWriteString("GAME STARTED!\r\n");
    SerialWriteString("Difficulty: ");
    SerialWriteUInt(Difficulty);

    SerialWriteString("\r\nGame time: ");
    SerialWriteUInt(GAME_DURATION_SECONDS);
    SerialWriteString(" seconds\r\n");

    SerialWriteString("Servo start command: ");
    SerialWriteUInt(ServoPosition);
    SerialWriteString("\r\n");

    SerialWriteString("Countdown started!\r\n");
}

static void EnterGameOver(void)
{
    CurrentState = STATE_GAME_OVER;

    ServoPosition = GAME_END_POSITION;
    MoveTimerArmServo(ServoPosition);

    WriteShiftRegister(0x00);

    SerialWriteString("\r\n");
    SerialWriteString("============================\r\n");
    SerialWriteString("TIME'S UP! GAME OVER!\r\n");
    SerialWriteString("============================\r\n");
    SerialWriteString("Press 'r' to return to welcome.\r\n");
}

// ====================================================
// SERIAL KEYBOARD INPUT
// ====================================================

static void HandleKey(char key)
{
    // Reset works from any state
    if (key == 'r')
    {
        EnterWelcome();
        return;
    }

    switch (CurrentState)
    {
        case STATE_WELCOME:
            if (key == 's')
            {
                EnterDifficultySelection();
            }
            break;

        case STATE_SELECT_DIFFICULTY:
            if (key == 'g')
            {
                EnterPlaying();
            }
            break;

        case STATE_PLAYING:
            // Gameplay inputs will be added later
            break;

        case STATE_GAME_OVER:
            // Wait for reset
            break;

        default:
            break;
    }
}

// ====================================================
// NONBLOCKING GAME UPDATE
// ====================================================

static void UpdateGame(uint32_t now)
{
    switch (CurrentState)
    {
        case STATE_WELCOME:
            // Wait for serial start command
            break;

        case STATE_SELECT_DIFFICULTY:

            if ((uint32_t)(now - LastPotUpdate)
                >= POT_UPDATE_TICKS)
            {
                LastPotUpdate = now;
                UpdateDifficulty();
            }
            break;

        case STATE_PLAYING:
        {
            uint32_t elapsedTicks =
                (uint32_t)(now - GameStartTime);

            // Has the game finished?
            if (elapsedTicks >= GAME_DURATION_TICKS)
            {
                EnterGameOver();
                break;
            }

            // Move timer-arm servo as time passes
            if ((uint32_t)(now - LastServoUpdate)
                >= SERVO_UPDATE_TICKS)
            {
                LastServoUpdate = now;

                // Convert elapsed time into milliseconds
                uint32_t elapsedMs =
                    elapsedTicks / 20000u;

                // Move linearly from start to end
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

                SerialWriteString("Time remaining: ");
                SerialWriteUInt(secondsRemaining);

                SerialWriteString(" s | Servo command: ");
                SerialWriteUInt(ServoPosition);

                SerialWriteString(" | Difficulty: ");
                SerialWriteUInt(Difficulty);
                SerialWriteString("\r\n");
            }
            break;
        }

        case STATE_GAME_OVER:
            // Wait for reset command
            break;

        default:
            break;
    }
}

// ====================================================
// MAIN TEST HARNESS
// ====================================================

int main(void)
{
    _HW_PIC32Init();

    ConfigurePinMappings();

    InitShiftRegister();
    UART2_Init();
    InitServoControl();
    InitPotentiometer();

    // Keep treat servo stationary
    MoveServo(2, 50);

    // Start with timer arm at its end position
    MoveTimerArmServo(GAME_END_POSITION);

    EnterWelcome();

    while (1)
    {
        // Handle UART receive overflow
        if (U2STAbits.OERR)
        {
            U2STAbits.OERR = 0;
        }

        // Process incoming keyboard commands
        while (U2STAbits.URXDA)
        {
            char received = (char)U2RXREG;
            HandleKey(received);
        }

        // Run state-dependent behavior
        UpdateGame(_CP0_GET_COUNT());
    }
}
