#ifndef LEDSERVICE_H
#define LEDSERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

bool InitLEDService(uint8_t Priority);
ES_Event_t RunLEDService(ES_Event_t ThisEvent);
bool PostLEDService(ES_Event_t ThisEvent);
#endif /* ServTemplate_H */