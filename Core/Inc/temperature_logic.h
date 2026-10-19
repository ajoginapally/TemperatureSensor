#ifndef TEMPERATURE_LOGIC_H
#define TEMPERATURE_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

/* All temperatures are in milli-degrees Celsius: 23500 means 23.5 C. */
typedef struct {
    int32_t low_mc;
    int32_t high_mc;
} TemperatureThresholds;

typedef enum {
    TEMPERATURE_INVALID = 0,
    TEMPERATURE_COLD,
    TEMPERATURE_NORMAL,
    TEMPERATURE_HOT
} TemperatureState;

/* Invalid readings/configuration never masquerade as a valid temperature. */
TemperatureState Temperature_Classify(int32_t temperature_mc, bool reading_valid,
                                     const TemperatureThresholds *thresholds);
/* Bits 0, 1, 2 mean LED 1, 2, 3 requested on; independent of GPIO polarity.
 * Invalid input maps to all off as a provisional fault indication. */
uint8_t Temperature_LedMask(TemperatureState state);

#endif
