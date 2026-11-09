#ifndef SETUPSERVICE_H
#define SETUPSERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

bool InitSetupService(uint8_t Priority);
ES_Event_t RunSetupService(ES_Event_t ThisEvent);
bool PostSetupService(ES_Event_t ThisEvent);
#endif /* ServTemplate_H */