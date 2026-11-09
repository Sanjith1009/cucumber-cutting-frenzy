#ifndef DEADMANHANDLING_H
#define DEADMANHANDLING_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

bool InitDeadmanHandling(uint8_t Priority);
ES_Event_t RunDeadmanHandling(ES_Event_t ThisEvent);
bool PostDeadmanHandling(ES_Event_t ThisEvent);
#endif /* ServTemplate_H */