#ifndef LEDSERVICE_H
#define LEDSERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT

// Public Function Prototypes

void DecreaseLitLEDs(void);
void FlashAllLEDs(void);
void TurnOnAllLEDs(void);
void SetLEDsPulsing(void);
void SetLEDBrightness(uint16_t potValue);

bool InitLEDService(uint8_t Priority);
ES_Event_t RunLEDService(ES_Event_t ThisEvent);
bool PostLEDService(ES_Event_t ThisEvent);
#endif /* ServTemplate_H */