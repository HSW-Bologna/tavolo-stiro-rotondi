#ifndef BOILER_CONTROL_H_INCLUDED
#define BOILER_CONTROL_H_INCLUDED


#include <stdint.h>
#include "model/model.h"


uint8_t boiler_control_value_changed(model_t *pmodel);
void    boiler_control_refresh(model_t *pmodel);
void    boiler_control_toggle(model_t *pmodel);
void    boiler_control_manage_callbacks(model_t *pmodel);


#endif