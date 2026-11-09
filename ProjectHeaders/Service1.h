#ifndef Service1_H
#define Service1_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

bool InitService1(uint8_t Priority);
ES_Event_t RunService1(ES_Event_t ThisEvent);

#endif /* ServTemplate_H */