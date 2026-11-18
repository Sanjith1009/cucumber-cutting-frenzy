#ifndef PLAYSERVICE_H
#define PLAYSERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT

#define PLAY_DURATION_MS 60000  // 60 seconds
// Public Function Prototypes

bool InitPlayService(uint8_t Priority);
ES_Event_t RunPlayService(ES_Event_t ThisEvent);
bool PostPlayService(ES_Event_t ThisEvent);
#endif /* ServTemplate_H */