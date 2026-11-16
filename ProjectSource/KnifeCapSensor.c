#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "KnifeCapSensor.h"
#include "CapSense.h"

// Calibrated from measurements of the actual knife handle.
#define KNIFE_NUM_SAMPLES                 30u
#define KNIFE_SAMPLE_INTERVAL_MS           2u
#define KNIFE_CALIBRATION_WINDOWS          5u
#define KNIFE_CONFIRM_WINDOWS              2u
#define KNIFE_TOUCH_DELTA_THRESHOLD       (-6)
#define KNIFE_RELEASE_DELTA_THRESHOLD     (-3)
#define KNIFE_TOUCH_VARIANCE_THRESHOLD    80u
#define KNIFE_RELEASE_VARIANCE_THRESHOLD  60u

static uint16_t samples[KNIFE_NUM_SAMPLES];
static uint8_t sampleCount;
static uint16_t lastSampleMs;
static bool clockStarted;

static uint8_t calibrationCount;
static uint32_t calibrationSum;
static uint16_t baseline;
static bool calibrated;

static bool held;
static uint8_t confirmCount;

void KnifeCapSensor_Init(void)
{
    sampleCount = 0u;
    lastSampleMs = 0u;
    clockStarted = false;

    calibrationCount = 0u;
    calibrationSum = 0u;
    baseline = 0u;
    calibrated = false;

    held = false;
    confirmCount = 0u;
}

bool KnifeCapSensor_Poll(uint16_t nowMs,
                         KnifeCapSensor_Reading_t *result)
{
    if (result == NULL)
    {
        return false;
    }

    if (!clockStarted)
    {
        lastSampleMs = nowMs;
        clockStarted = true;
        return false;
    }

    // uint16_t subtraction remains correct over the timer's 16-bit wrap.
    if ((uint16_t)(nowMs - lastSampleMs) <
        KNIFE_SAMPLE_INTERVAL_MS)
    {
        return false;
    }
    lastSampleMs = nowMs;

    samples[sampleCount++] = CapSense_ReadRaw();
    if (sampleCount < KNIFE_NUM_SAMPLES)
    {
        return false;
    }
    sampleCount = 0u;

    uint32_t sum = 0u;
    for (uint8_t i = 0u; i < KNIFE_NUM_SAMPLES; ++i)
    {
        sum += samples[i];
    }
    uint16_t mean = (uint16_t)(sum / KNIFE_NUM_SAMPLES);

    uint32_t squaredDeviationSum = 0u;
    for (uint8_t i = 0u; i < KNIFE_NUM_SAMPLES; ++i)
    {
        int32_t diff = (int32_t)samples[i] - (int32_t)mean;
        squaredDeviationSum += (uint32_t)(diff * diff);
    }
    uint32_t variance = squaredDeviationSum / KNIFE_NUM_SAMPLES;

    bool changed = false;

    if (!calibrated)
    {
        // First five windows = 150 untouched samples.
        calibrationSum += mean;
        ++calibrationCount;
        if (calibrationCount >= KNIFE_CALIBRATION_WINDOWS)
        {
            baseline = (uint16_t)(calibrationSum /
                                  KNIFE_CALIBRATION_WINDOWS);
            calibrated = true;
        }
    }
    else
    {
        int32_t delta = (int32_t)baseline - (int32_t)mean;

        if (!held)
        {
            if ((delta <= KNIFE_TOUCH_DELTA_THRESHOLD) &&
                (variance >= KNIFE_TOUCH_VARIANCE_THRESHOLD))
            {
                if (++confirmCount >= KNIFE_CONFIRM_WINDOWS)
                {
                    held = true;
                    confirmCount = 0u;
                    changed = true;
                }
            }
            else
            {
                confirmCount = 0u;
            }
        }
        else
        {
            if ((delta >= KNIFE_RELEASE_DELTA_THRESHOLD) &&
                (variance <= KNIFE_RELEASE_VARIANCE_THRESHOLD))
            {
                if (++confirmCount >= KNIFE_CONFIRM_WINDOWS)
                {
                    held = false;
                    confirmCount = 0u;
                    changed = true;
                }
            }
            else
            {
                confirmCount = 0u;
            }
        }
    }

    result->mean = mean;
    result->baseline = baseline;
    result->variance = variance;
    result->delta = (int32_t)baseline - (int32_t)mean;
    result->calibrated = calibrated;
    result->held = held;
    result->stateChanged = changed;
    return true;
}
