#ifndef BUZZER_H_INCLUDED
#define BUZZER_H_INCLUDED


#include <stdint.h>


void buzzer_init(void);
void buzzer_beep(uint16_t r, unsigned long t_on, unsigned long t_off);


#endif
