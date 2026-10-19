#ifndef APP_CONFIG_H
#define APP_CONFIG_H
/* Provisional thresholds: confirm with the project lead. */
#define APP_LOW_MC 20000
#define APP_HIGH_MC 30000
#define APP_SAMPLE_MS 500U
/* A2=0, A1=0, A0=1 per schematic: NXP Table 4. Verify assembly. */
#define APP_SENSOR_ADDRESS 0x49U
/* Keep outputs low until R5-R7 current limiting is confirmed. Set to 1 then. */
#ifndef APP_LED_OUTPUTS_ENABLED
#define APP_LED_OUTPUTS_ENABLED 0
#endif
#endif
