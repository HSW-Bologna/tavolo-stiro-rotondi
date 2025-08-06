#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "view/intl/intl.h"
#include "view/common.h"
#include "gel/pagemanager/page_manager.h"


enum {
    BACK_BTN_ID,
    ADC_PARAMETER_ID,
    BOARD_TEMPERATURE_CONTROL_PARAMETER_ID,
    HYSTERESIS_PARAMETER_ID,
    FAN_PARAMETER_ID,
    LIGHT_PARAMETER_ID,
    SECOND_IRON_PARAMETER_ID,
    STEAM_GUN_PARAMETER_ID,
    HEATED_ARM_PARAMETER_ID,
    BOILER_PARAMETER_ID,
};


struct page_data {
    lv_obj_t *page;
    int32_t   scroll_position;
};


static void      page_update(model_t *pmodel, struct page_data *pdata);
static lv_obj_t *btn_parameter_create(lv_obj_t *parent, const char *name, const char *value, int id);
static void      number_to_string(char *string, size_t len, int value);
static void      seconds_to_string(char *string, size_t len, int value);
static void      fan_to_string(char *string, size_t len, int value);
static void      update_adc_threshold(model_t *pmodel, int value);
static void      update_hysteresis(model_t *pmodel, int value);
static void      update_fan_config(model_t *pmodel, int value);
static void      update_light_enabled(model_t *pmodel, int value);
static void      update_boiler_enabled(model_t *pmodel, int value);
static void      update_second_iron_enabled(model_t *pmodel, int value);
static void      update_steam_gun_enabled(model_t *pmodel, int value);
static void      update_heated_arm_enabled(model_t *pmodel, int value);
static void      update_board_temperature_control(model_t *pmodel, int value);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);
    pdata->scroll_position = 0;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *cont;
    (void)pdata;

    view_common_create_title(lv_scr_act(), "Parametri", BACK_BTN_ID, -1, -1);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_style_pad_column(cont, 16, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    char string[32] = {0};
    btn_parameter_create(cont, "Luce", pmodel->configuration.light_enabled ? "Si" : "No", LIGHT_PARAMETER_ID);

    btn_parameter_create(cont, "Temp. tavolo", pmodel->configuration.board_temperature_control ? "Si" : "No",
                         BOARD_TEMPERATURE_CONTROL_PARAMETER_ID);

    btn_parameter_create(cont, "Secondo ferro", pmodel->configuration.second_iron_enabled ? "Si" : "No",
                         SECOND_IRON_PARAMETER_ID);

    btn_parameter_create(cont, "Pistola a vapore", pmodel->configuration.steam_gun_enabled ? "Si" : "No",
                         STEAM_GUN_PARAMETER_ID);

    btn_parameter_create(cont, "Risc. bracciolo", pmodel->configuration.heated_arm_enabled ? "Si" : "No",
                         HEATED_ARM_PARAMETER_ID);

    btn_parameter_create(cont, "Caldaia", pmodel->configuration.boiler_enabled ? "Si" : "No", BOILER_PARAMETER_ID);

    if (pmodel->configuration.boiler_enabled) {
        number_to_string(string, sizeof(string), model_get_boiler_adc_threshold(pmodel));
        btn_parameter_create(cont, "Livello boiler", string, ADC_PARAMETER_ID);
    }

    seconds_to_string(string, sizeof(string), model_get_isteresi_caldaia(pmodel));
    btn_parameter_create(cont, "Isteresi caldaia", string, HYSTERESIS_PARAMETER_ID);

    fan_to_string(string, sizeof(string), pmodel->configuration.fan_config);
    btn_parameter_create(cont, "Config. ventole", string, FAN_PARAMETER_ID);

    lv_obj_scroll_to_y(cont, pdata->scroll_position, LV_ANIM_OFF);
    pdata->page = cont;
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    static number_parameter_metadata_t metadata = {0};

    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_UPDATE:
            page_update(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_OPEN:
            model_set_machine_test(pmodel);
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_LONG_PRESSED_REPEAT:
                case LV_EVENT_CLICKED: {
                    pdata->scroll_position = lv_obj_get_scroll_y(pdata->page);

                    switch (event.data.id) {
                        case LIGHT_PARAMETER_ID: {
                            metadata.initial_value = pmodel->configuration.light_enabled;
                            metadata.min           = 0;
                            metadata.max           = 1;
                            metadata.step          = 0;
                            metadata.name          = "Abilitazione Luce";
                            metadata.to_string     = NULL;
                            metadata.update        = update_light_enabled;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }

                        case SECOND_IRON_PARAMETER_ID: {
                            pdata->scroll_position = lv_obj_get_scroll_y(pdata->page);

                            metadata.initial_value = pmodel->configuration.second_iron_enabled;
                            metadata.min           = 0;
                            metadata.max           = 1;
                            metadata.step          = 0;
                            metadata.name          = "Secondo Ferro";
                            metadata.to_string     = NULL;
                            metadata.update        = update_second_iron_enabled;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }

                        case STEAM_GUN_PARAMETER_ID: {
                            metadata.initial_value = pmodel->configuration.steam_gun_enabled;
                            metadata.min           = 0;
                            metadata.max           = 1;
                            metadata.step          = 0;
                            metadata.name          = "Pistola a Vapore";
                            metadata.to_string     = NULL;
                            metadata.update        = update_steam_gun_enabled;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }

                        case HEATED_ARM_PARAMETER_ID: {
                            metadata.initial_value = pmodel->configuration.heated_arm_enabled;
                            metadata.min           = 0;
                            metadata.max           = 1;
                            metadata.step          = 0;
                            metadata.name          = "Riscaldamento Bracciolo";
                            metadata.to_string     = NULL;
                            metadata.update        = update_heated_arm_enabled;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }

                        case BOILER_PARAMETER_ID: {
                            metadata.initial_value = pmodel->configuration.boiler_enabled;
                            metadata.min           = 0;
                            metadata.max           = 1;
                            metadata.step          = 0;
                            metadata.name          = "Abilitazione Caldaia";
                            metadata.to_string     = NULL;
                            metadata.update        = update_boiler_enabled;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }

                        case ADC_PARAMETER_ID: {
                            metadata.initial_value = model_get_boiler_adc_threshold(pmodel);
                            metadata.min           = 0;
                            metadata.max           = 4000;
                            metadata.step          = 10;
                            metadata.name          = "Livello boiler (ADC)";
                            metadata.to_string     = number_to_string;
                            metadata.update        = update_adc_threshold;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }

                        case BOARD_TEMPERATURE_CONTROL_PARAMETER_ID: {
                            metadata.initial_value = pmodel->configuration.board_temperature_control;
                            metadata.min           = 0;
                            metadata.max           = 1;
                            metadata.step          = 0;
                            metadata.name          = "Controllo temperatura tavolo";
                            metadata.to_string     = NULL;
                            metadata.update        = update_board_temperature_control;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }


                        case HYSTERESIS_PARAMETER_ID: {
                            metadata.initial_value = model_get_boiler_adc_threshold(pmodel);
                            metadata.min           = 0;
                            metadata.max           = 300;
                            metadata.step          = 1;
                            metadata.name          = "Isteresi caldaia";
                            metadata.to_string     = seconds_to_string;
                            metadata.update        = update_hysteresis;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }

                        case FAN_PARAMETER_ID: {
                            metadata.initial_value = pmodel->configuration.fan_config;
                            metadata.min           = FAN_CONFIG_SWITCH;
                            metadata.max           = FAN_CONFIG_SUCTION_ONLY;
                            metadata.step          = 0;
                            metadata.name          = "Configurazione ventole";
                            metadata.to_string     = fan_to_string;
                            metadata.update        = update_fan_config;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_number_parameter;
                            msg.vmsg.extra = &metadata;
                            break;
                        }

                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            model_set_machine_on(pmodel);
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    return msg;
}


static void page_update(model_t *pmodel, struct page_data *pdata) {}


static lv_obj_t *btn_parameter_create(lv_obj_t *parent, const char *name, const char *value, int id) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_width(btn, LV_PCT(90));

    lv_obj_t *lbl_name = lv_label_create(btn);
    lv_obj_add_flag(lbl_name, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_align(lbl_name, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(lbl_name, name);
    lv_obj_set_style_text_font(lbl_name, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);

    lv_obj_t *lbl_value = lv_label_create(btn);
    lv_obj_add_flag(lbl_value, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_align(lbl_value, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_label_set_text(lbl_value, value);
    lv_obj_set_style_text_font(lbl_value, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);

    view_register_object_default_callback(btn, id);

    return btn;
}


static void number_to_string(char *string, size_t len, int value) {
    snprintf(string, len, "%4i", value);
}


static void seconds_to_string(char *string, size_t len, int value) {
    snprintf(string, len, "%3i s", value);
}


static void fan_to_string(char *string, size_t len, int value) {
    switch (value) {
        case FAN_CONFIG_SWITCH:
            snprintf(string, len, "Scambio");
            break;

        case FAN_CONFIG_BOTH:
            snprintf(string, len, "Doppia");
            break;

        case FAN_CONFIG_SUCTION_ONLY:
            snprintf(string, len, "Solo asp.");
            break;
    }
}


static void update_light_enabled(model_t *pmodel, int value) {
    pmodel->configuration.light_enabled = value;
}


static void update_second_iron_enabled(model_t *pmodel, int value) {
    pmodel->configuration.second_iron_enabled = value;
}


static void update_steam_gun_enabled(model_t *pmodel, int value) {
    pmodel->configuration.steam_gun_enabled = value;
}


static void update_heated_arm_enabled(model_t *pmodel, int value) {
    pmodel->configuration.heated_arm_enabled = value;
}


static void update_boiler_enabled(model_t *pmodel, int value) {
    pmodel->configuration.boiler_enabled = value;
}


static void update_adc_threshold(model_t *pmodel, int value) {
    model_set_boiler_adc_threshold(pmodel, value);
}


static void update_board_temperature_control(model_t *pmodel, int value) {
    pmodel->configuration.board_temperature_control = value;
}


static void update_hysteresis(model_t *pmodel, int value) {
    pmodel->configuration.isteresi_caldaia = value;
}


static void update_fan_config(model_t *pmodel, int value) {
    pmodel->configuration.fan_config = value;
}


const pman_page_t page_parameters = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
