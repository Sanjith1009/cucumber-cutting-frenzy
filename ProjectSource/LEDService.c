#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "LEDService.h"
#include "ShiftRegister.h"

#define NUM_GAME_LEDS       6u
#define ALL_LEDS_MASK      0x3Fu

// CP0 increments at SYSCLK / 2. These constants assume 40 MHz SYSCLK.
#define CP0_TICKS_PER_MS     20000u
#define FLASH_INTERVAL_TICKS (250u * CP0_TICKS_PER_MS)

static uint8_t MyPriority;
static uint8_t LitLEDCount = 0u;
static bool LEDsFlashing = false;
static bool FlashState = false;
static uint32_t LastFlashTime = 0u;
static uint32_t FlashStartTime = 0u;
static uint32_t FlashDurationTicks = 0u;  // 0 = keep flashing

static void UpdateLEDOutputs(void)
{
    // We want to generate our desired logical LED pattern
    uint8_t pattern = (uint8_t)((1u << LitLEDCount) - 1u);
    WriteShiftRegister((uint8_t)(ALL_LEDS_MASK ^ pattern));
}

void LEDService_SetLitCount(uint8_t count)
{
    if (count > NUM_GAME_LEDS)
    {
        count = NUM_GAME_LEDS;
    }

    LEDsFlashing = false;
    LitLEDCount = count;
    UpdateLEDOutputs();
}

void LEDService_ShowCutProgress(uint16_t completedCuts,
                                uint16_t requiredCuts)
{
    if (requiredCuts == 0u)
    {
        LEDService_SetLitCount(0u);
        return;
    }

    if (completedCuts > requiredCuts)
    {
        completedCuts = requiredCuts;
    }

    uint32_t remainingCuts = (uint32_t)(requiredCuts - completedCuts);

    // Scale remaining chops to six LEDs, rounding UP.
    // With 8 required cuts: 0=>6 LEDs, 2=>5, 4=>3, 6=>2, 8=>0.
    uint8_t litCount = (uint8_t)(
        (remainingCuts * NUM_GAME_LEDS + requiredCuts - 1u)
        / requiredCuts);

    LEDService_SetLitCount(litCount);
}

void TurnOnAllLEDs(void)
{
    LEDService_SetLitCount(NUM_GAME_LEDS);
}

void DecreaseLitLEDs(void)
{
    if (LitLEDCount > 0u)
    {
        LEDService_SetLitCount((uint8_t)(LitLEDCount - 1u));
    }
    else
    {
        LEDService_SetLitCount(0u);
    }
}

static void StartFlashing(uint32_t durationTicks)
{
    LEDsFlashing = true;
    FlashState = true;
    FlashStartTime = _CP0_GET_COUNT();
    LastFlashTime = FlashStartTime;
    FlashDurationTicks = durationTicks;
    WriteShiftRegister(0x00u);
}

void FlashAllLEDs(void)
{
    // Preserve the original behavior: indefinite until another LED command.
    if (!LEDsFlashing)
    {
        StartFlashing(0u);
    }
}

void LEDService_FlashForMs(uint16_t durationMs)
{
    if (durationMs == 0u)
    {
        return;
    }

    // Saved LitLEDCount will be restored when the flash expires.
    StartFlashing((uint32_t)durationMs * CP0_TICKS_PER_MS);
}

void LEDService_Update(void)
{
    if (!LEDsFlashing)
    {
        return;
    }

    uint32_t now = _CP0_GET_COUNT();

    if ((FlashDurationTicks != 0u) &&
        ((uint32_t)(now - FlashStartTime) >= FlashDurationTicks))
    {
        LEDsFlashing = false;
        UpdateLEDOutputs();   // restore saved LED bar
        return;
    }

    if ((uint32_t)(now - LastFlashTime) >= FLASH_INTERVAL_TICKS)
    {
        LastFlashTime = now;
        FlashState = !FlashState;
        WriteShiftRegister(FlashState ? 0x00u : ALL_LEDS_MASK);
    }
}

void SetLEDsPulsing(void)
{
    // Not implemented: OE is wired LOW, so no hardware brightness PWM yet.
}

void SetLEDBrightness(uint16_t potValue)
{
    (void)potValue;
    // Not implemented: requires an LED brightness control strategy.
}

bool InitLEDService(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t event = {ES_INIT, 0};
    return ES_PostToService(MyPriority, event);
}

ES_Event_t RunLEDService(ES_Event_t ThisEvent)
{
    (void)ThisEvent;
    ES_Event_t returnEvent = {ES_NO_EVENT, 0};
    return returnEvent;
}

bool PostLEDService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}
