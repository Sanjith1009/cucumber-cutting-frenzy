
#ifndef AUDIOSERVICE_H
#define AUDIOSERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ES_Events.h"

// TODO THIS IS CURRENLY JUST A PLACEHOLDER TO TRY AND GET THINGS TO COMPILE
// Initialize audio service
bool InitAudioService(uint8_t Priority);

// Post an event to the audio service
bool PostAudioService(ES_Event_t ThisEvent);

// Process an event
ES_Event_t RunAudioService(ES_Event_t ThisEvent);

#endif // AUDIOSERVICE_H
