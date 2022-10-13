#include "model/model.h"
#include "utils/utils.h"
#include "gel/state_machine/state_machine.h"
#include "gel/timer/timecheck.h"
#include "gel/timer/timer.h"
#include "app_config.h"
#include "esp_log.h"
#include "view/view.h"
#include "fan_control.h"
#include "peripherals/digout.h"


typedef enum {
    FAN_SM_STATE_OFF = 0,
    FAN_SM_STATE_SOFFIO,
    FAN_SM_STATE_ASPIRAZIONE,
} fan_control_sm_state_t;


typedef enum {
    FAN_CONTROL_EVENT_FOTOCELLULA_ON,
    FAN_CONTROL_EVENT_FOTOCELLULA_OFF,
    FAN_CONTROL_EVENT_PEDALE_ON,
    FAN_CONTROL_EVENT_PEDALE_OFF,
} fan_control_event_t;


DEFINE_STATE_MACHINE(fan_control, fan_control_event_t, model_t);


static int off_event_manager(model_t *pmodel, fan_control_event_t event);
static int soffio_event_manager(model_t *pmodel, fan_control_event_t event);
static int aspirazione_event_manager(model_t *pmodel, fan_control_event_t event);


static fan_control_event_manager_t managers[] = {
    [FAN_SM_STATE_OFF]         = off_event_manager,
    [FAN_SM_STATE_SOFFIO]      = soffio_event_manager,
    [FAN_SM_STATE_ASPIRAZIONE] = aspirazione_event_manager,
};

static fan_control_state_machine_t sm = {
    .state    = FAN_SM_STATE_OFF,
    .managers = managers,
};


static int off_event_manager(model_t *pmodel, fan_control_event_t event) {
    switch (event) {
        default:
            return -1;
    }

    return -1;
}


static int soffio_event_manager(model_t *pmodel, fan_control_event_t event) {
    switch (event) {
        default:
            return -1;
    }

    return -1;
}



static int aspirazione_event_manager(model_t *pmodel, fan_control_event_t event) {
    switch (event) {
        default:
            return -1;
    }

    return -1;
}
