#ifndef KNIFE_CAP_SENSOR_H
#define KNIFE_CAP_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

// One completed window of 30 CTMU/ADC measurements.
typedef struct
{
    uint16_t mean;
    uint16_t baseline;
    uint32_t variance;
    int32_t delta;        // baseline - mean (negative when held)
    bool calibrated;
    bool held;
    bool stateChanged;
} KnifeCapSensor_Reading_t;

// Reset calibration and detection state. Call once on boot with
// the knife UNTOUCHED, after CapSense_Init().
void KnifeCapSensor_Init(void);

// Nonblocking, cooperative poll called from an ES event checker.
// Returns true only when a complete 30-sample window has been processed.
// nowMs comes from ES_Timer_GetTime().
bool KnifeCapSensor_Poll(uint16_t nowMs,
                         KnifeCapSensor_Reading_t *result);

#endif
