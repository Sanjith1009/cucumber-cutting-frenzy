
#ifndef SETUPSERVICE_H
#define SETUPSERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"

bool InitSetupService(uint8_t Priority);

ES_Event_t RunSetupService(ES_Event_t ThisEvent);

bool PostSetupService(ES_Event_t ThisEvent);

#endif
