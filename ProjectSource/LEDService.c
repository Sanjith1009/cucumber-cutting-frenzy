#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "../ProjectHeaders/LEDService.h"
#include <stdio.h>
#include <stdbool.h>
#include "ShiftRegister.h"
#include <xc.h>


// Module-level variable to store the service priority
static uint8_t MyPriority;

#define NUM_GAME_LEDS 6u
#define ALL_LEDS_MASK 0x3Fu

// At 40 MHz SYSCLK, CP0 counts at 20 MHz.
// 5,000,000 ticks = 250 ms.
#define FLASH_INTERVAL_TICKS 5000000u

static uint8_t LitLEDCount = 0u;
static bool LEDsFlashing = false;
static bool FlashState = false;
static uint32_t LastFlashTime = 0u;

// Update the six LEDs according to LitLEDCount
static void UpdateLEDOutputs(void)
{
    uint8_t pattern =
        (uint8_t)((1u << LitLEDCount) - 1u);

    WriteShiftRegister(pattern);
}

bool InitLEDService(uint8_t Priority)
{
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    ThisEvent.EventType = ES_INIT;
    ThisEvent.EventParam = 0;
    
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunLEDService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};

    switch (ThisEvent.EventType)
    {
        case ES_INIT:
            // LED hardware is initialized in main.c
            break;

        default:
            break;
    }

    return ReturnEvent;
}

bool PostLEDService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}


void TurnOnAllLEDs(void)
{
    // Stop any flashing animation
    LEDsFlashing = false;

    // Illuminate all six LEDs
    LitLEDCount = NUM_GAME_LEDS;

    UpdateLEDOutputs();
}


void DecreaseLitLEDs(void)
{
    // Stop flashing before changing LED count
    LEDsFlashing = false;

    // Decrease the number of illuminated LEDs
    if (LitLEDCount > 0u)
    {
        LitLEDCount--;
    }

    UpdateLEDOutputs();
}


void FlashAllLEDs(void)
{
    // Don't restart the timing if already flashing
    if (LEDsFlashing)
    {
        return;
    }

    LEDsFlashing = true;
    FlashState = true;

    // Record when flashing started
    LastFlashTime = _CP0_GET_COUNT();

    // Begin with all six LEDs illuminated
    WriteShiftRegister(ALL_LEDS_MASK);
}


void SetLEDsPulsing(void)
{
    // TODO: Start idle pulsing animation
}

void SetLEDBrightness(uint16_t potValue)
{
    // TODO: Adjust LED brightness
    (void)potValue;
}


void LEDService_Update(void)
{
    if (!LEDsFlashing)
    {
        return;
    }

    uint32_t currentTime = _CP0_GET_COUNT();

    // Check whether 250 ms have elapsed
    if ((uint32_t)(currentTime - LastFlashTime)
        >= FLASH_INTERVAL_TICKS)
    {
        LastFlashTime = currentTime;

        // Alternate between ON and OFF
        FlashState = !FlashState;

        if (FlashState)
        {
            WriteShiftRegister(ALL_LEDS_MASK);
        }
        else
        {
            WriteShiftRegister(0x00);
        }
    }
}

