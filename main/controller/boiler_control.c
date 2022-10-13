#include "model/model.h"
#include "utils/utils.h"
#include "gel/state_machine/state_machine.h"
#include "gel/timer/timecheck.h"
#include "gel/timer/timer.h"
#include "app_config.h"
#include "esp_log.h"
#include "view/view.h"
#include "boiler_control.h"
#include "peripherals/digout.h"


typedef enum {
    BOILER_SM_STATE_OFF = 0,
    BOILER_SM_STATE_ON,
} boiler_control_sm_state_t;


typedef enum {
    BOILER_CONTROL_EVENT_TOGGLE,
    BOILER_CONTROL_EVENT_LEVEL_CHANGE,
    BOILER_CONTROL_EVENT_REFRESH,
} boiler_control_event_t;


DEFINE_STATE_MACHINE(boiler_control, boiler_control_event_t, model_t);


static int  on_event_manager(model_t *pmodel, boiler_control_event_t event);
static int  off_event_manager(model_t *pmodel, boiler_control_event_t event);
static void boiler_update(model_t *pmodel, uint8_t on);
static void pump_update(model_t *pmodel, uint8_t on);


static boiler_control_event_manager_t managers[] = {
    [BOILER_SM_STATE_OFF] = off_event_manager,
    [BOILER_SM_STATE_ON]  = on_event_manager,
};

static boiler_control_state_machine_t sm = {
    .state    = BOILER_SM_STATE_OFF,
    .managers = managers,
};


uint8_t boiler_control_value_changed(model_t *pmodel) {
    return boiler_control_sm_send_event(&sm, pmodel, BOILER_CONTROL_EVENT_LEVEL_CHANGE) != 0;
}


void boiler_control_refresh(model_t *pmodel) {
    boiler_control_sm_send_event(&sm, pmodel, BOILER_CONTROL_EVENT_REFRESH);
}


void boiler_control_toggle(model_t *pmodel) {
    boiler_control_sm_send_event(&sm, pmodel, BOILER_CONTROL_EVENT_TOGGLE);
}


static int off_event_manager(model_t *pmodel, boiler_control_event_t event) {
    switch (event) {
        case BOILER_CONTROL_EVENT_LEVEL_CHANGE:
            break;

        case BOILER_CONTROL_EVENT_REFRESH:
            boiler_update(pmodel, 0);
            break;

        case BOILER_CONTROL_EVENT_TOGGLE:
            boiler_update(pmodel, 1);
            return BOILER_SM_STATE_ON;
    }

    return -1;
}


static int on_event_manager(model_t *pmodel, boiler_control_event_t event) {
    switch (event) {
        case BOILER_CONTROL_EVENT_LEVEL_CHANGE:
            break;

        case BOILER_CONTROL_EVENT_REFRESH:
            boiler_update(pmodel, 1);
            break;

        case BOILER_CONTROL_EVENT_TOGGLE:
            boiler_update(pmodel, 0);
            return BOILER_SM_STATE_OFF;
    }

    return -1;
}


static void boiler_update(model_t *pmodel, uint8_t on) {
    if (model_set_boiler_on(pmodel, on)) {
        view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
    }
    if (!model_get_test(pmodel)) {
        digout_update(DIGOUT_RISCALDAMENTO_VAPORE, on);
    }
}


static void pump_update(model_t *pmodel, uint8_t on) {
    if (model_set_pompa_on(pmodel, on)) {
        view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
    }
    if (!model_get_test(pmodel)) {
        digout_update(DIGOUT_POMPA, on);
    }
}