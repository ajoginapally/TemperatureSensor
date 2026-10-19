#ifndef TEMPERATURE_LOGIC_H
#define TEMPERATURE_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int32_t low_mc;
    int32_t high_mc;
} TemperatureThresholds;

typedef enum
{
    TEMPERATURE_INVALID = 0,
    TEMPERATURE_COLD,
    TEMPERATURE_NORMAL,
    TEMPERATURE_HOT
} TemperatureState;

TemperatureState Temperature_Classify(int32_t temperature_mc, bool reading_valid,
                                      const TemperatureThresholds *thresholds);

uint8_t Temperature_LedMask(TemperatureState state);

#endif
