
#ifndef PHASE_CUT_H_INCLUDED
#define PHASE_CUT_H_INCLUDED

#include <stdint.h>


typedef enum {
    PHASE_CUT_FAN_1 = 0,
    PHASE_CUT_FAN_2,
} phase_cut_fan_t;


void phase_cut_init(void);
void phase_cut_timer_enable(int enable);
void phase_cut_set_percentage(phase_cut_fan_t fan, unsigned int perc);

#endif
