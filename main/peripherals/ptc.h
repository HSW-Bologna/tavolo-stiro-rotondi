#ifndef PTC_H_INCLUDED
#define PTC_H_INCLUDED

#include <stdint.h>


typedef enum {
    PTC_TEMP1 = 0,
    PTC_TEMP2,
} ptc_t;


void     ptc_init(void);
uint16_t ptc_get_adc_value(ptc_t ptc);
int      ptc_get_temperature(ptc_t ptc);

#endif
