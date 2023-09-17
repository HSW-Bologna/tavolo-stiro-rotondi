#include "model/model.h"
#include "utils/utils.h"
#include "gel/state_machine/state_machine.h"
#include "gel/timer/timecheck.h"
#include "gel/timer/timer.h"
#include "app_config.h"
#include "esp_log.h"
#include "view/view.h"
#include "boiler_control.h"


typedef enum {
    BOILER_SM_STATE_OFF = 0,
    BOILER_SM_STATE_LEVEL_HYSTERESIS,
    BOILER_SM_STATE_FILLING,
    BOILER_SM_STATE_FILLING_HEATING,
    BOILER_SM_STATE_HEATING,
} boiler_control_sm_state_t;


typedef enum {
    BOILER_CONTROL_EVENT_TOGGLE,
    BOILER_CONTROL_EVENT_LEVEL_CHANGE,
    BOILER_CONTROL_EVENT_TIMER_EXPIRED,
    BOILER_CONTROL_EVENT_REFRESH,
} boiler_control_event_t;


DEFINE_STATE_MACHINE(boiler_control, boiler_control_event_t, model_t);


static int off_event_manager(model_t *pmodel, boiler_control_event_t event);
static int hysteresis_event_manager(model_t *pmodel, boiler_control_event_t event);
static int filling_event_manager(model_t *pmodel, boiler_control_event_t event);
static int filling_heating_event_manager(model_t *pmodel, boiler_control_event_t event);
static int heating_event_manager(model_t *pmodel, boiler_control_event_t event);

static void gel_timer_callback(gel_timer_t *timer, void *pmodel, void *arg);
static void boiler_update(model_t *pmodel, uint8_t on);
static void pump_update(model_t *pmodel, uint8_t on);


static const char *TAG = "Boiler control";


static boiler_control_event_manager_t managers[] = {
    [BOILER_SM_STATE_OFF]              = off_event_manager,
    [BOILER_SM_STATE_LEVEL_HYSTERESIS] = hysteresis_event_manager,
    [BOILER_SM_STATE_FILLING]          = filling_event_manager,
    [BOILER_SM_STATE_FILLING_HEATING]  = filling_heating_event_manager,
    [BOILER_SM_STATE_HEATING]          = heating_event_manager,
};

static boiler_control_state_machine_t sm = {
    .state    = BOILER_SM_STATE_OFF,
    .managers = managers,
};

static gel_timer_t hysteresis_timer = GEL_TIMER_NULL;


void boiler_control_manage_callbacks(model_t *pmodel) {
    gel_timer_manage_callbacks(&hysteresis_timer, 1, get_millis(), pmodel);
}


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

        case BOILER_CONTROL_EVENT_TIMER_EXPIRED:
            break;

        case BOILER_CONTROL_EVENT_REFRESH:
            boiler_update(pmodel, 0);
            pump_update(pmodel, 0);
            break;

        case BOILER_CONTROL_EVENT_TOGGLE:
            if (model_boiler_pieno(pmodel)) {
                ESP_LOGI(TAG, "Accensione caldaia diretta");
                pump_update(pmodel, 0);
                boiler_update(pmodel, 1);
                return BOILER_SM_STATE_HEATING;
            } else {
                ESP_LOGI(TAG, "Caldaia vuota, riempo...");
                pump_update(pmodel, 1);
                boiler_update(pmodel, 0);
                return BOILER_SM_STATE_FILLING;
            }
    }

    return -1;
}


static int filling_event_manager(model_t *pmodel, boiler_control_event_t event) {
    switch (event) {
        case BOILER_CONTROL_EVENT_LEVEL_CHANGE:
            if (model_boiler_pieno(pmodel)) {
                ESP_LOGI(TAG, "Livello raggiunto (%i), riscaldo", model_get_probe_level(pmodel, LIQUID_LEVEL_PROBE_2));
                boiler_update(pmodel, 1);
                pump_update(pmodel, 0);
                return BOILER_SM_STATE_HEATING;
            }
            break;

        case BOILER_CONTROL_EVENT_REFRESH:
            boiler_update(pmodel, 0);
            pump_update(pmodel, 1);
            break;

        case BOILER_CONTROL_EVENT_TOGGLE:
            ESP_LOGI(TAG, "Spegnimento");
            pump_update(pmodel, 0);
            boiler_update(pmodel, 0);
            return BOILER_SM_STATE_OFF;

        case BOILER_CONTROL_EVENT_TIMER_EXPIRED:
            break;
    }

    return -1;
}


static int filling_heating_event_manager(model_t *pmodel, boiler_control_event_t event) {
    switch (event) {
        case BOILER_CONTROL_EVENT_LEVEL_CHANGE:
            if (model_boiler_pieno(pmodel)) {
                ESP_LOGI(TAG, "Livello raggiunto (%i), riscaldo", model_get_probe_level(pmodel, LIQUID_LEVEL_PROBE_2));
                boiler_update(pmodel, 1);
                pump_update(pmodel, 0);
                return BOILER_SM_STATE_HEATING;
            }
            break;

        case BOILER_CONTROL_EVENT_REFRESH:
            boiler_update(pmodel, 1);
            pump_update(pmodel, 1);
            break;

        case BOILER_CONTROL_EVENT_TOGGLE:
            ESP_LOGI(TAG, "Spegnimento");
            pump_update(pmodel, 0);
            boiler_update(pmodel, 0);
            return BOILER_SM_STATE_OFF;

        case BOILER_CONTROL_EVENT_TIMER_EXPIRED:
            break;
    }

    return -1;
}


static int heating_event_manager(model_t *pmodel, boiler_control_event_t event) {
    switch (event) {
        case BOILER_CONTROL_EVENT_LEVEL_CHANGE:
            if (!model_boiler_pieno(pmodel)) {
                ESP_LOGI(TAG, "Liquido finito (%i), aspetto per il tempo di isteresi...",
                         model_get_probe_level(pmodel, LIQUID_LEVEL_PROBE_2));
                gel_timer_activate(&hysteresis_timer, model_get_isteresi_caldaia(pmodel) * 100, get_millis(),
                                   gel_timer_callback, NULL);
                return BOILER_SM_STATE_LEVEL_HYSTERESIS;
            }
            break;

        case BOILER_CONTROL_EVENT_REFRESH:
            boiler_update(pmodel, 1);
            pump_update(pmodel, 0);
            break;

        case BOILER_CONTROL_EVENT_TOGGLE:
            ESP_LOGI(TAG, "Spegnimento");
            pump_update(pmodel, 0);
            boiler_update(pmodel, 0);
            return BOILER_SM_STATE_OFF;

        case BOILER_CONTROL_EVENT_TIMER_EXPIRED:
            break;
    }

    return -1;
}


static int hysteresis_event_manager(model_t *pmodel, boiler_control_event_t event) {
    switch (event) {
        case BOILER_CONTROL_EVENT_LEVEL_CHANGE:
            if (model_boiler_pieno(pmodel)) {
                ESP_LOGI(TAG, "Di nuovo in livello (%i)", model_get_probe_level(pmodel, LIQUID_LEVEL_PROBE_2));
                gel_timer_deactivate(&hysteresis_timer);
                pump_update(pmodel, 0);
                boiler_update(pmodel, 1);
                return BOILER_SM_STATE_HEATING;
            }
            break;

        case BOILER_CONTROL_EVENT_REFRESH:
            boiler_update(pmodel, 0);
            pump_update(pmodel, 0);
            break;

        case BOILER_CONTROL_EVENT_TOGGLE:
            ESP_LOGI(TAG, "Spegnimento");
            boiler_update(pmodel, 0);
            pump_update(pmodel, 0);
            return BOILER_SM_STATE_OFF;

        case BOILER_CONTROL_EVENT_TIMER_EXPIRED:
            ESP_LOGI(TAG, "Isteresi terminata, richiedo acqua");
            boiler_update(pmodel, 1);
            pump_update(pmodel, 1);
            return BOILER_SM_STATE_FILLING_HEATING;
    }

    return -1;
}


static void boiler_update(model_t *pmodel, uint8_t on) {
    if (model_set_boiler_on(pmodel, on)) {
        view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
    }
    if (!model_get_test(pmodel)) {
        model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_VAPORE, on);
    }
}


static void pump_update(model_t *pmodel, uint8_t on) {
    if (model_set_pompa_on(pmodel, on)) {
        view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
    }
    if (!model_get_test(pmodel)) {
        model_set_relay(pmodel, DIGOUT_POMPA, on);
    }
}


static void gel_timer_callback(gel_timer_t *timer, void *pmodel, void *arg) {
    (void)arg;
    boiler_control_sm_send_event(&sm, pmodel, BOILER_CONTROL_EVENT_TIMER_EXPIRED);
}
