#ifndef DIGIN_H_INCLUDED
#define DIGIN_H_INCLUDED


#include <stdint.h>


typedef enum {
    DIGIN_IN4 = 0,
    DIGIN_IN3,
    DIGIN_IN2,
    DIGIN_IN1,
    DIGIN_VAP,
} digin_t;


void    digin_sync(void);
uint8_t digin_read(digin_t digin);
uint8_t digin_have_changed(void);
void    digin_init(void);


#endif