#ifndef TEMPERATURE_CONTROL_H_INCLUDED
#define TEMPERATURE_CONTROL_H_INCLUDED


#include "model/model.h"


typedef struct {
    model_t *pmodel;

    uint8_t (*enabled)(model_t *);
    uint8_t (*should_turn_on)(model_t *);
    uint8_t (*should_turn_off)(model_t *);
    void (*turn_on)(model_t *);
    void (*turn_off)(model_t *);

    const char *tag;
    void       *sm;
} hcontrol_data_t;


void *hcontrol_state_machine_new(void);
int   hcontrol_value_changed(hcontrol_data_t *pdata);
void  hcontrol_refresh(hcontrol_data_t *pdata);


#endif