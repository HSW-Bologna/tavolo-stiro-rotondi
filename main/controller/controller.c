#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "peripherals/i2c_devices.h"
#include "I2C/i2c_devices/rtc/RX8010/rx8010.h"
#include "esp_log.h"
#include "utils/utils.h"
#include "gel/data_structures/watcher.h"
#include "gel/timer/timecheck.h"
#include "modbus.h"
#include "configuration.h"
#include "hysteresis_control.h"
#include "fan_control.h"
#include "boiler_control.h"


#define NUM_WATCHED_VARIABLES 9


static void    refresh_light(void *mem, void *arg);
static void    refresh_gun(void *mem, void *arg);
static void    refresh_state(void *mem, void *arg);
static void    refresh_ferro_1(void *mem, void *arg);
static void    refresh_ferro_2(void *mem, void *arg);
static void    refresh_ventole(void *mem, void *arg);
static void    refresh_temperature(void *mem, void *arg);
static void    update_watched_variables(model_t *pmodel);
static void    turn_off_table(model_t *pmodel);
static void    turn_on_table(model_t *pmodel);
static void    turn_off_arm(model_t *pmodel);
static void    turn_on_arm(model_t *pmodel);
static uint8_t not_in_test(model_t *pmodel);
static void    update_outputs(void *mem, void *arg);
static void    refresh_boiler(void *mem, void *arg);


static const char *TAG = "Controller";


static hcontrol_data_t tavolo_control = {
    .tag             = "Table control",
    .enabled         = not_in_test,
    .should_turn_on  = model_should_activate_table,
    .should_turn_off = model_should_deactivate_table,
    .turn_on         = turn_on_table,
    .turn_off        = turn_off_table,
};

static hcontrol_data_t bracciolo_control = {
    .tag             = "Arm control",
    .enabled         = not_in_test,
    .should_turn_on  = model_should_activate_arm,
    .should_turn_off = model_should_deactivate_arm,
    .turn_on         = turn_on_arm,
    .turn_off        = turn_off_arm,
};


static watcher_t watched_variables[NUM_WATCHED_VARIABLES + 1] = {};
static uint16_t  variables_temperature[6]                     = {0};
static uint16_t  variables_output[2]                          = {0};
static uint8_t   light_state                                  = 0;
static uint8_t   second_iron_state                            = 0;
static uint8_t   gun_state                                    = 0;


void controller_init(model_t *pmodel) {
    modbus_init();

    tavolo_control.pmodel = pmodel;
    tavolo_control.sm     = hcontrol_state_machine_new();

    bracciolo_control.pmodel = pmodel;
    bracciolo_control.sm     = hcontrol_state_machine_new();

    if (rx8010_is_stopped(rx8010_driver)) {
        ESP_LOGI(TAG, "RTC was stopped, initializing...");
        rtc_time_t rtc_time = {.day = 6, .wday = 1, .month = 3, .year = 22};
        rx8010_set_stop(rx8010_driver, 0);
        rx8010_set_time(rx8010_driver, rtc_time);
        ESP_LOGI(TAG, "RTC Clock started");
    } else {
        rtc_time_t rtc_time = {0};
        rx8010_get_time(rx8010_driver, &rtc_time);
        struct tm tm = rx8010_tm_from_rtc(rtc_time);
        tm.tm_isdst  = -1;
        utils_set_system_time(&tm);
    }

    configuration_init(pmodel);
    update_watched_variables(pmodel);

    size_t i               = 0;
    watched_variables[i++] = WATCHER(&variables_output, update_outputs, pmodel);
    watched_variables[i++] = WATCHER(&pmodel->run.machine_state, refresh_state, pmodel);
    watched_variables[i++] = WATCHER(&light_state, refresh_light, pmodel);
    watched_variables[i++] = WATCHER(&gun_state, refresh_gun, pmodel);
    watched_variables[i++] = WATCHER(&pmodel->run.ferro_1, refresh_ferro_1, pmodel);
    watched_variables[i++] = WATCHER(&second_iron_state, refresh_ferro_2, pmodel);
    watched_variables[i++] = WATCHER(&pmodel->minion.inputs, refresh_ventole, pmodel);
    watched_variables[i++] = WATCHER(&pmodel->configuration.boiler_enabled, refresh_boiler, pmodel);
    watched_variables[i++] = WATCHER_ARRAY(variables_temperature, 6, refresh_temperature, pmodel);
    assert(i == NUM_WATCHED_VARIABLES);
    watched_variables[i] = WATCHER_NULL;
    watcher_list_init(watched_variables);
    watcher_trigger_all(watched_variables);

    if (pmodel->configuration.machine_model == MACHINE_MODEL_NONE) {
        view_change_page(pmodel, &page_machine_model);
    } else {
        view_change_page(pmodel, &page_main);
    }
}


void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_TEST_OUTPUT:
            model_set_relay(pmodel, msg->digout.number, msg->digout.value);
            ESP_LOGI(TAG, "Relays %i", pmodel->minion.relays);
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_TOGGLE_BOILER:
            boiler_control_toggle(pmodel);
            break;
    }
}


void controller_manage(model_t *pmodel) {
    static unsigned long ts_minion_refresh = 0;
    static unsigned long ts_1s             = 0;
    static unsigned long ts_5s             = 0;
    static uint8_t       old_vapore        = 0;

    modbus_response_t response;
    if (modbus_get_response(&response)) {
        if (response.error) {
#ifndef SIMULATOR
            model_set_alarm_communication(pmodel, 1);
#endif
            view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
        } else {
            switch (response.tag) {
                case MODBUS_RESPONSE_TAG_READ_STATE: {
                    if (model_update_minion_state(pmodel, response.as.state.inputs_map, response.as.state.liquid_levels,
                                                  response.as.state.ptc_adcs, response.as.state.ptc_temperatures)) {
                        uint8_t vapore = model_digin_read(pmodel, DIGIN_VAP);
                        if (vapore != old_vapore) {
                            if (vapore) {
                                view_event((view_event_t){.code = VIEW_EVENT_CODE_VAPORE});
                            }
                            old_vapore = vapore;
                        }

                        if (!model_is_in_test(pmodel)) {
                            // model_set_relay(pmodel, DIGOUT_RECUPERATOR, model_digin_read(pmodel,
                            // DIGIN_AIR_FLOW_SWITCH));
                        }

                        boiler_control_value_changed(pmodel);
                        view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    if (is_expired(ts_minion_refresh, get_millis(), 200)) {
        modbus_read_state();
        ts_minion_refresh = get_millis();
    }

    if (is_expired(ts_1s, get_millis(), 2000UL)) {
        // FIXME: pezza per fiera, c'e' da capire come deve funzionare
        // modbus_write_outputs(model_get_minion_relays(pmodel), model_get_percentuale_aspirazione(pmodel),
        // model_get_percentuale_soffio(pmodel));
        update_outputs(NULL, pmodel);
        ts_1s = get_millis();
    }

    if (is_expired(ts_5s, get_millis(), 10000UL)) {
        ESP_LOGI(TAG, "State dump:\n\tFotocellula dx: %i\n\tPedale: %i\n\tADC1: %i\n\tADC2: %i\n",
                 pmodel->minion.inputs[0], pmodel->minion.inputs[3],
                 model_get_probe_level(pmodel, LIQUID_LEVEL_PROBE_1),
                 model_get_probe_level(pmodel, LIQUID_LEVEL_PROBE_2));
        ts_5s = get_millis();
    }

    boiler_control_manage_callbacks(pmodel);
    update_watched_variables(pmodel);
    watcher_process_changes(watched_variables, get_millis());
    configuration_process_parameters();
    fan_control(pmodel);
}


static void refresh_light(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (!model_is_in_test(pmodel)) {
        model_set_relay(pmodel, DIGOUT_LUCE, model_get_light(pmodel));
    }
}


static void refresh_gun(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (!model_is_in_test(pmodel)) {
        model_set_relay(pmodel, DIGOUT_GUN, model_get_gun_state(pmodel));
    }
}


static void refresh_ferro_1(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (!model_is_in_test(pmodel)) {
        model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_FERRO_1, model_get_ferro_1(pmodel));
    }
}


static void refresh_ferro_2(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (!model_is_in_test(pmodel)) {
        model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_FERRO_2, model_get_second_iron(pmodel));
    }
}


static void refresh_ventole(void *mem, void *arg) {
    (void)mem;
    ESP_LOGI(TAG, "Refresh ventole");
    model_t *pmodel = arg;
    fan_control(pmodel);
}


static void refresh_boiler(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    boiler_control_refresh(pmodel);
}


static void refresh_temperature(void *mem, void *arg) {
    (void)mem;
    (void)arg;
    hcontrol_value_changed(&tavolo_control);
    hcontrol_value_changed(&bracciolo_control);
}


static void refresh_state(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    switch (pmodel->run.machine_state) {
        case MACHINE_STATE_ON:
            refresh_light(NULL, pmodel);
            refresh_ferro_1(NULL, pmodel);
            refresh_ferro_2(NULL, pmodel);
            hcontrol_refresh(&tavolo_control);
            hcontrol_refresh(&bracciolo_control);
            fan_control(pmodel);
            break;

        case MACHINE_STATE_TEST:
            model_set_relay(pmodel, DIGOUT_LUCE, 0);
            model_set_relay(pmodel, DIGOUT_POMPA, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_BRACCIOLO, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_FERRO_1, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_FERRO_2, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_VAPORE, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_PIANO, 0);
            model_set_relay(pmodel, DIGOUT_RECUPERATOR, 0);
            model_set_test_percentage_suction(pmodel, 0);
            model_set_test_percentage_blow(pmodel, 0);
            break;

        case MACHINE_STATE_STANDBY:
            model_set_relay(pmodel, DIGOUT_LUCE, 0);
            model_set_relay(pmodel, DIGOUT_POMPA, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_BRACCIOLO, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_FERRO_1, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_FERRO_2, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_VAPORE, 0);
            model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_PIANO, 0);
            model_set_relay(pmodel, DIGOUT_RECUPERATOR, 0);
            model_set_test_percentage_suction(pmodel, 0);
            model_set_test_percentage_blow(pmodel, 0);
            break;
    }
}


static void update_watched_variables(model_t *pmodel) {
    variables_temperature[0] = model_get_temperatura_tavolo(pmodel);
    variables_temperature[1] = model_get_temperatura_bracciolo(pmodel);
    variables_temperature[2] = model_get_setpoint_temperatura_tavolo(pmodel);
    variables_temperature[3] = model_get_setpoint_temperatura_bracciolo(pmodel);
    variables_temperature[4] = model_get_richiesta_temperatura_tavolo(pmodel);
    variables_temperature[5] = model_get_richiesta_temperatura_bracciolo(pmodel);
    variables_output[0]      = model_get_minion_relays(pmodel);
    variables_output[1]      = model_get_percentuale_soffio(pmodel) | (model_get_percentuale_aspirazione(pmodel) << 8);
    light_state              = model_get_light(pmodel);
    second_iron_state        = model_get_second_iron(pmodel);
    gun_state                = model_get_steam_gun(pmodel);
}


static void turn_on_table(model_t *pmodel) {
    model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_PIANO, 1);
    model_set_tavolo_on(pmodel, 1);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static void turn_off_table(model_t *pmodel) {
    model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_PIANO, 0);
    model_set_tavolo_on(pmodel, 0);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static void turn_on_arm(model_t *pmodel) {
    model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_BRACCIOLO, 1);
    model_set_bracciolo_on(pmodel, 1);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static void turn_off_arm(model_t *pmodel) {
    model_set_relay(pmodel, DIGOUT_RISCALDAMENTO_BRACCIOLO, 0);
    model_set_bracciolo_on(pmodel, 0);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static uint8_t not_in_test(model_t *pmodel) {
    return !model_is_in_test(pmodel) && !model_is_in_standby(pmodel);
}


static void update_outputs(void *mem, void *arg) {
    model_t *pmodel = arg;
    if (model_get_percentuale_aspirazione(pmodel) > 0) {
        modbus_write_outputs(model_get_minion_relays(pmodel), model_get_percentuale_aspirazione(pmodel), 0);
    } else if (model_get_percentuale_soffio(pmodel) > 0) {
        switch (pmodel->configuration.fan_config) {
            case FAN_CONFIG_SWITCH:
                modbus_write_outputs(model_get_minion_relays(pmodel), model_get_percentuale_soffio(pmodel), 100);
                break;

            case FAN_CONFIG_SUCTION_ONLY:
                modbus_write_outputs(model_get_minion_relays(pmodel), model_get_percentuale_aspirazione(pmodel), 0);
                break;

            case FAN_CONFIG_BOTH:
                modbus_write_outputs(model_get_minion_relays(pmodel), model_get_percentuale_aspirazione(pmodel), model_get_percentuale_soffio(pmodel));
                break;
        }
    } else {
        modbus_write_outputs(model_get_minion_relays(pmodel), 0, 0);
    }
}
