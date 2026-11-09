#ifndef MESSAGESERVICE_H
#define MESSAGESERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

bool InitMessageService(uint8_t Priority);
ES_Event_t RunMessageService(ES_Event_t ThisEvent);
bool PostMessageService(ES_Event_t ThisEvent);
#endif /* ServTemplate_H */