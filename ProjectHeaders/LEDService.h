#ifndef LEDSERVICE_H
#define LEDSERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ES_Events.h"

// LED counts/patterns for the six M74HC595 outputs QA-QF.
void LEDService_SetLitCount(uint8_t count);
void LEDService_ShowCutProgress(uint16_t completedCuts,
                                uint16_t requiredCuts);

// Existing functions retained for compatibility.
void TurnOnAllLEDs(void);
void DecreaseLitLEDs(void);
void FlashAllLEDs(void);       // Flash until another LED command stops it.
void LEDService_FlashForMs(uint16_t durationMs); // Finite celebration.
void SetLEDsPulsing(void);    // Placeholder: requires PWM design.
void SetLEDBrightness(uint16_t potValue); // Placeholder.
void LEDService_Update(void); // Called by CheckLEDAnimation.

// Existing service API (not necessary to register it as an ES service yet).
bool InitLEDService(uint8_t Priority);
ES_Event_t RunLEDService(ES_Event_t ThisEvent);
bool PostLEDService(ES_Event_t ThisEvent);

#endif // LEDSERVICE_H
