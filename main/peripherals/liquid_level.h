#ifndef LIQUID_LEVEL_H_INCLUDED
#define LIQUID_LEVEL_H_INCLUDED


#include <stdint.h>


typedef enum {
    LIQUID_LEVEL_PROBE_1 = 0,
    LIQUID_LEVEL_PROBE_2,
} liquid_level_probe_t;


void     liquid_level_init(void);
uint16_t liquid_level_get_adc_value(liquid_level_probe_t level);


#endif