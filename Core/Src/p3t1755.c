#include "p3t1755.h"
int32_t P3T1755_Decode(uint8_t msb, uint8_t lsb)
{
    int32_t raw = ((uint32_t)msb << 4) | (lsb >> 4);
    if (raw & 0x800)
        raw -= 4096;
    return raw * 125 / 2;
}
