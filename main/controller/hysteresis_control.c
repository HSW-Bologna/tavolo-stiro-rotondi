#include "model/model.h"
#include "utils/utils.h"
#include "gel/state_machine/state_machine.h"
#include "gel/timer/timecheck.h"
#include "gel/timer/timer.h"
#include "app_config.h"
#include "esp_log.h"
#include "view/view.h"
#include "hysteresis_control.h"


typedef enum {
    TCONTROL_SM_STATE_OFF = 0,
    TCONTROL_SM_STATE_ON,
} hcontrol_sm_state_t;


typedef enum {
    TCONTROL_EVENT_TEMPERATURE_CHANGE,
    TCONTROL_EVENT_REFRESH,
} hcontrol_event_t;


DEFINE_STATE_MACHINE(hcontrol, hcontrol_event_t, hcontrol_data_t);


static int on_event_manager(hcontrol_data_t *pdata, hcontrol_event_t event);
static int off_event_manager(hcontrol_data_t *pdata, hcontrol_event_t event);


static hcontrol_event_manager_t managers[] = {
    [TCONTROL_SM_STATE_OFF] = off_event_manager,
    [TCONTROL_SM_STATE_ON]  = on_event_manager,
};


void *hcontrol_state_machine_new(void) {
    hcontrol_state_machine_t *sm = (hcontrol_state_machine_t *)malloc(sizeof(hcontrol_state_machine_t));
    assert(sm != NULL);
    sm->managers = managers;
    sm->state    = TCONTROL_SM_STATE_OFF;
    return sm;
}


int hcontrol_value_changed(hcontrol_data_t *pdata) {
    if (pdata->enabled != NULL && !pdata->enabled(pdata->pmodel)) {
        return 0;
    } else if (hcontrol_sm_send_event(pdata->sm, pdata, TCONTROL_EVENT_TEMPERATURE_CHANGE)) {
        return 1;
    } else {
        return 0;
    }
}


void hcontrol_refresh(hcontrol_data_t *pdata) {
    if (pdata->enabled == NULL || pdata->enabled(pdata->pmodel)) {
        hcontrol_sm_send_event(pdata->sm, pdata, TCONTROL_EVENT_REFRESH);
    }
}


static int off_event_manager(hcontrol_data_t *pdata, hcontrol_event_t event) {
    switch (event) {
        case TCONTROL_EVENT_TEMPERATURE_CHANGE:
            if (pdata->should_turn_on(pdata->pmodel)) {
                ESP_LOGI(pdata->tag, "Turning on");
                pdata->turn_on(pdata->pmodel);
                return TCONTROL_SM_STATE_ON;
            }
            break;

        case TCONTROL_EVENT_REFRESH:
            if (pdata->should_turn_off(pdata->pmodel)) {
                pdata->turn_off(pdata->pmodel);
            }
            break;
    }

    return -1;
}


static int on_event_manager(hcontrol_data_t *pdata, hcontrol_event_t event) {
    switch (event) {
        case TCONTROL_EVENT_TEMPERATURE_CHANGE:
            if (pdata->should_turn_off(pdata->pmodel)) {
                ESP_LOGI(pdata->tag, "Turning off");
                pdata->turn_off(pdata->pmodel);
                return TCONTROL_SM_STATE_OFF;
            }
            break;

        case TCONTROL_EVENT_REFRESH:
            if (pdata->should_turn_on(pdata->pmodel)) {
                pdata->turn_on(pdata->pmodel);
            }
            break;
    }

    return -1;
}