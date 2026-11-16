
#ifndef AUDIOSERVICE_H
#define AUDIOSERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"

// Initialize AudioService
bool InitAudioService(uint8_t Priority);

// Post an audio event
bool PostAudioService(ES_Event_t ThisEvent);

// Process events
ES_Event_t RunAudioService(ES_Event_t ThisEvent);

#endif // AUDIOSERVICE_H
