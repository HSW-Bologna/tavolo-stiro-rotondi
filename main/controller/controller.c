#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "buzzer.h"
#include "peripherals/digin.h"
#include "peripherals/digout.h"
#include "peripherals/i2c_devices.h"
#include "I2C/i2c_devices/rtc/RX8010/rx8010.h"
#include "esp_log.h"
#include "utils/utils.h"
#include "peripherals/phase_cut.h"
#include "peripherals/ptc.h"
#include "peripherals/liquid_level.h"
#include "gel/data_structures/watcher.h"
#include "gel/timer/timecheck.h"
#include "configuration.h"
#include "hysteresis_control.h"
#include "fan_control.h"
#include "boiler_control.h"


#define NUM_WATCHED_VARIABLES 6


static void    refresh_light(void *mem, void *arg);
static void    refresh_test(void *mem, void *arg);
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
static uint8_t   variabili_velocita[4]                        = {0};
static uint16_t  variabili_temperatura[6]                     = {0};


void controller_init(model_t *pmodel) {
    tavolo_control.pmodel = pmodel;
    tavolo_control.sm     = hcontrol_state_machine_new();

    bracciolo_control.pmodel = pmodel;
    bracciolo_control.sm     = hcontrol_state_machine_new();

    buzzer_init();
    buzzer_beep(2, 50, 100);

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
    watched_variables[i++] = WATCHER(&pmodel->run.test, refresh_test, pmodel);
    watched_variables[i++] = WATCHER(&pmodel->run.luce, refresh_light, pmodel);
    watched_variables[i++] = WATCHER(&pmodel->run.ferro_1, refresh_ferro_1, pmodel);
    watched_variables[i++] = WATCHER(&pmodel->run.ferro_2, refresh_ferro_2, pmodel);
    watched_variables[i++] = WATCHER_ARRAY(variabili_velocita, 4, refresh_ventole, pmodel);
    watched_variables[i++] = WATCHER_ARRAY(variabili_temperatura, 6, refresh_temperature, pmodel);
    assert(i == NUM_WATCHED_VARIABLES);
    watched_variables[i] = WATCHER_NULL;
    watcher_list_init(watched_variables);
    watcher_trigger_all_cb(watched_variables);

    view_change_page(pmodel, &page_main);
}


void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_TEST_OUTPUT:
            digout_update(msg->digout.number, msg->digout.value);
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_TOGGLE_BOILER:
            boiler_control_toggle(pmodel);
            break;
    }
}


void controller_manage(model_t *pmodel) {
    static uint8_t old_vapore = 0;
    unsigned long  ts_1s      = 0;

    if (digin_have_changed()) {
        uint8_t vapore = digin_read(DIGIN_VAP);

        pmodel->test.inputs[0] = digin_read(DIGIN_IN1);
        pmodel->test.inputs[1] = digin_read(DIGIN_IN2);
        pmodel->test.inputs[2] = digin_read(DIGIN_IN3);
        pmodel->test.inputs[3] = digin_read(DIGIN_IN4);
        pmodel->test.inputs[4] = vapore;

        if (old_vapore != vapore) {
            if (vapore) {
                view_event((view_event_t){.code = VIEW_EVENT_CODE_VAPORE});
            }
            old_vapore = vapore;
        }

        view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});

        digin_sync();
    }

    if (is_expired(ts_1s, get_millis(), 1000UL)) {
        uint8_t update = 0;

        update |= model_set_temperatura_tavolo(pmodel, ptc_get_temperature(PTC_TEMP1));
        update |= model_set_temperatura_bracciolo(pmodel, ptc_get_temperature(PTC_TEMP2));
        update |= model_set_adc_ptc_1(pmodel, ptc_get_adc_value(PTC_TEMP1));
        update |= model_set_adc_ptc_2(pmodel, ptc_get_adc_value(PTC_TEMP2));
        update |= model_set_adc_level_1(pmodel, liquid_level_get_adc_value(LIQUID_LEVEL_PROBE_1));
        update |= model_set_adc_level_2(pmodel, liquid_level_get_adc_value(LIQUID_LEVEL_PROBE_2));

        if (update) {
            view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
        }

        ts_1s = get_millis();
    }

    update_watched_variables(pmodel);
    watcher_process_changes(watched_variables, get_millis());
    configuration_process_parameters();
}


static void refresh_light(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (!model_get_test(pmodel)) {
        digout_update(DIGOUT_LUCE, model_get_luce(pmodel));
    }
}


static void refresh_ferro_1(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (!model_get_test(pmodel)) {
        digout_update(DIGOUT_RISCALDAMENTO_FERRO_1, model_get_ferro_1(pmodel));
    }
}


static void refresh_ferro_2(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (!model_get_test(pmodel)) {
        digout_update(DIGOUT_RISCALDAMENTO_FERRO_2, model_get_ferro_2(pmodel));
    }
}


static void refresh_ventole(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (!model_get_test(pmodel)) {
        phase_cut_set_percentage(PHASE_CUT_FAN_1, model_get_percentuale_soffio(pmodel));
        phase_cut_set_percentage(PHASE_CUT_FAN_2, model_get_percentuale_aspirazione(pmodel));
    }
}


static void refresh_temperature(void *mem, void *arg) {
    (void)mem;
    (void)arg;
    hcontrol_value_changed(&tavolo_control);
    hcontrol_value_changed(&bracciolo_control);
}


static void refresh_test(void *mem, void *arg) {
    (void)mem;
    model_t *pmodel = arg;
    if (model_get_test(pmodel)) {
        DIGOUT_CLEAR(DIGOUT_LUCE);
        DIGOUT_CLEAR(DIGOUT_POMPA);
        DIGOUT_CLEAR(DIGOUT_RISCALDAMENTO_BRACCIOLO);
        DIGOUT_CLEAR(DIGOUT_RISCALDAMENTO_FERRO_1);
        DIGOUT_CLEAR(DIGOUT_RISCALDAMENTO_FERRO_2);
        DIGOUT_CLEAR(DIGOUT_RISCALDAMENTO_VAPORE);
        DIGOUT_CLEAR(DIGOUT_RISCALDAMENTO_PIANO);
        phase_cut_set_percentage(PHASE_CUT_FAN_1, 0);
        phase_cut_set_percentage(PHASE_CUT_FAN_2, 0);
    } else {
        refresh_light(NULL, pmodel);
        refresh_ferro_1(NULL, pmodel);
        refresh_ferro_2(NULL, pmodel);
        refresh_ventole(NULL, pmodel);
        hcontrol_refresh(&tavolo_control);
        hcontrol_refresh(&bracciolo_control);
    }
}


static void update_watched_variables(model_t *pmodel) {
    variabili_velocita[0] = model_get_soffio_on(pmodel);
    variabili_velocita[1] = model_get_aspirazione_on(pmodel);
    variabili_velocita[2] = model_get_velocita_soffio(pmodel);
    variabili_velocita[3] = model_get_velocita_aspirazione(pmodel);

    variabili_temperatura[0] = model_get_temperatura_tavolo(pmodel);
    variabili_temperatura[1] = model_get_temperatura_bracciolo(pmodel);
    variabili_temperatura[2] = model_get_setpoint_temperatura_tavolo(pmodel);
    variabili_temperatura[3] = model_get_setpoint_temperatura_bracciolo(pmodel);
    variabili_temperatura[4] = model_get_richiesta_temperatura_tavolo(pmodel);
    variabili_temperatura[5] = model_get_richiesta_temperatura_bracciolo(pmodel);
}


static void turn_on_table(model_t *pmodel) {
    DIGOUT_SET(DIGOUT_RISCALDAMENTO_PIANO);
    model_set_tavolo_on(pmodel, 1);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static void turn_off_table(model_t *pmodel) {
    DIGOUT_CLEAR(DIGOUT_RISCALDAMENTO_PIANO);
    model_set_tavolo_on(pmodel, 0);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static void turn_on_arm(model_t *pmodel) {
    DIGOUT_SET(DIGOUT_RISCALDAMENTO_BRACCIOLO);
    model_set_bracciolo_on(pmodel, 1);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static void turn_off_arm(model_t *pmodel) {
    DIGOUT_CLEAR(DIGOUT_RISCALDAMENTO_BRACCIOLO);
    model_set_bracciolo_on(pmodel, 0);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static uint8_t not_in_test(model_t *pmodel) {
    return !model_get_test(pmodel);
}