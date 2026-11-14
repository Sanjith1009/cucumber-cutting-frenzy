#ifndef PLAYSERVICE_H
#define PLAYSERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT

// Length of one game. SetupService starts PLAY_TIMER with this, and
// PlayService sweeps the timer-arm servo over the same time.
#define PLAY_DURATION_MS 60000u

// Public Function Prototypes

bool InitPlayService(uint8_t Priority);
ES_Event_t RunPlayService(ES_Event_t ThisEvent);
bool PostPlayService(ES_Event_t ThisEvent);
#endif /* ServTemplate_H */