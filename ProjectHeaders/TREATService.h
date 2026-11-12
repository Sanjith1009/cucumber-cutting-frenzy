
#ifndef TREATSERVICE_H
#define TREATSERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ES_Events.h"

// TODO THIS IS JUST A PLACEHOLDER TO TRY AND GET THE THING TO COMPULE NEEDS TO BE IMPLEMENTED LATER

// Initialize treat service
bool InitTREATService(uint8_t Priority);

// Post an event to the treat service
bool PostTREATService(ES_Event_t ThisEvent);

// Process an event
ES_Event_t RunTREATService(ES_Event_t ThisEvent);

#endif // TREATSERVICE_H
