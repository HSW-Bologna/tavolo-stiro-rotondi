#ifndef DIGOUT_H_INCLUDED
#define DIGOUT_H_INCLUDED

#include <stdint.h>


#define DIGOUT_SET(digout)   digout_update(digout, 1);
#define DIGOUT_CLEAR(digout) digout_update(digout, 0);


typedef enum {
    DIGOUT_BUZZER = 0,
    DIGOUT_POMPA,
    DIGOUT_RISCALDAMENTO_VAPORE,
    DIGOUT_RISCALDAMENTO_PIANO,
    DIGOUT_RISCALDAMENTO_BRACCIOLO,
    DIGOUT_RISCALDAMENTO_FERRO_1,
    DIGOUT_RISCALDAMENTO_FERRO_2,
    DIGOUT_LUCE,
} digout_t;


void digout_init(void);
void digout_update(digout_t digout, uint8_t value);


#endif