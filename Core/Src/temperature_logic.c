#include "temperature_logic.h"
#include <stddef.h>

TemperatureState Temperature_Classify(int32_t temperature_mc, bool reading_valid,
                                     const TemperatureThresholds *thresholds)
{
    if (!reading_valid || thresholds == NULL ||
        thresholds->low_mc >= thresholds->high_mc) {
        return TEMPERATURE_INVALID;
    }
    if (temperature_mc < thresholds->low_mc) {
        return TEMPERATURE_COLD;
    }
    if (temperature_mc > thresholds->high_mc) {
        return TEMPERATURE_HOT;
    }
    /* Both threshold boundaries belong to the normal range. */
    return TEMPERATURE_NORMAL;
}

uint8_t Temperature_LedMask(TemperatureState state)
{
    switch (state) {
    case TEMPERATURE_COLD:   return 0x01U;
    case TEMPERATURE_NORMAL: return 0x03U;
    case TEMPERATURE_HOT:    return 0x07U;
    default:                return 0x00U;
    }
}
