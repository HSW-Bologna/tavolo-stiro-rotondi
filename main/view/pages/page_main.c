#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/view_types.h"
#include "view/common.h"
#include "view/theme/style.h"
#include "view/intl/intl.h"
#include "gel/pagemanager/page_manager.h"
#include "app_config.h"
#include "gel/parameter/parameter.h"
#include "esp_log.h"


LV_IMG_DECLARE(img_warning_lg);
LV_IMG_DECLARE(img_calore_1);
LV_IMG_DECLARE(img_calore_2);
LV_IMG_DECLARE(img_calore_3);
LV_IMG_DECLARE(img_vapore_1);
LV_IMG_DECLARE(img_vapore_2);
LV_IMG_DECLARE(img_vapore_3);
LV_IMG_DECLARE(img_ferro_1_off);
LV_IMG_DECLARE(img_ferro_2_off);
LV_IMG_DECLARE(img_ferro_1_on);
LV_IMG_DECLARE(img_ferro_2_on);
LV_IMG_DECLARE(img_aria);
LV_IMG_DECLARE(img_aria_on);
LV_IMG_DECLARE(img_boiler_off_0);
LV_IMG_DECLARE(img_boiler_off_1);
LV_IMG_DECLARE(img_boiler_off_2);
LV_IMG_DECLARE(img_boiler_on_2);
LV_IMG_DECLARE(img_boiler_off_3);
LV_IMG_DECLARE(img_boiler_on_3);
LV_IMG_DECLARE(img_fotocellula_sx);
LV_IMG_DECLARE(img_fotocellula_dx);
LV_IMG_DECLARE(img_luce_off);
LV_IMG_DECLARE(img_luce_on);
LV_IMG_DECLARE(img_menu);
LV_IMG_DECLARE(img_bracciolo_off);
LV_IMG_DECLARE(img_bracciolo_on);
LV_IMG_DECLARE(img_tavolo_off);
LV_IMG_DECLARE(img_tavolo_on);
LV_IMG_DECLARE(img_ventola);
LV_IMG_DECLARE(img_bolle_2);
LV_IMG_DECLARE(img_bolle_3);

#define ANIM_VAPORE_PERIOD 1200
#define POPUP_WIDTH        360
#define POPUP_HEIGHT       260
#define FAN_POPUP_WIDTH    370
#define FAN_POPUP_HEIGHT   (260 + 32)


enum {
    FERRO_1_BTN_ID,
    FERRO_2_BTN_ID,
    FOTOCELLULA_BTN_ID,
    TAVOLO_BTN_ID,
    BRACCIOLO_BTN_ID,
    SOFFIO_BTN_ID,
    SUCTION_BTN_ID,
    BOILER_BTN_ID,
    MENU_BTN_ID,
    LUCE_BTN_ID,
    VELOCITA_SOFFIO_SLIDER_ID,
    FAN_SUCTION_SLIDER_ID,
    SETPOINT_TAVOLO_ARC_ID,
    SETPOINT_TAVOLO_PLUS_BTN_ID,
    SETPOINT_TAVOLO_MINUS_BTN_ID,
    SETPOINT_BRACCIOLO_ARC_ID,
    SETPOINT_BRACCIOLO_PLUS_BTN_ID,
    SETPOINT_BRACCIOLO_MINUS_BTN_ID,
    FAN_SOFFIO_PLUS_BTN_ID,
    FAN_SOFFIO_MINUS_BTN_ID,
    FAN_SUCTION_PLUS_BTN_ID,
    FAN_SUCTION_MINUS_BTN_ID,
    SOFFIO_POPUP_BTN_ID,
    SUCTION_POPUP_BTN_ID,
    BTN_DISMISS_ALARM_ID,
    BLANKET_ID,
};


typedef enum {
    EDITING_TARGET_NONE = 0,
    EDITING_TARGET_TEMPERATURA_TAVOLO,
    EDITING_TARGET_TEMPERATURA_BRACCIOLO,
    EDITING_TARGET_SOFFIO,
    EDITING_TARGET_SUCTION,
} editing_target_t;


struct page_data {
    lv_obj_t *btn_tavolo;
    lv_obj_t *btn_bracciolo;
    lv_obj_t *btn_ferro_1;
    lv_obj_t *btn_ferro_2;
    lv_obj_t *btn_aspirazione;
    lv_obj_t *btn_boiler;
    lv_obj_t *btn_soffio;
    lv_obj_t *btn_fotocellula;
    lv_obj_t *btn_luce;
    lv_obj_t *btn_menu;

    lv_obj_t *img_ventola_aspirazione;
    lv_obj_t *img_ventola_soffio;
    lv_obj_t *img_ferro_1;
    lv_obj_t *img_ferro_2;
    lv_obj_t *img_vapore_ferro_1;
    lv_obj_t *img_vapore_ferro_2;
    lv_obj_t *img_tavolo;
    lv_obj_t *img_bracciolo;
    lv_obj_t *img_tavolo_popup;
    lv_obj_t *img_bracciolo_popup;
    lv_obj_t *img_calore_tavolo;
    lv_obj_t *img_calore_bracciolo;
    lv_obj_t *img_boiler;
    lv_obj_t *img_boiler_bubbles;

    lv_obj_t *blanket;

    lv_obj_t *popup_tavolo;
    lv_obj_t *arc_setpoint_tavolo;
    lv_obj_t *lbl_setpoint_tavolo;

    lv_obj_t *popup_bracciolo;
    lv_obj_t *arc_setpoint_bracciolo;
    lv_obj_t *lbl_setpoint_bracciolo;

    lv_obj_t *popup_soffio;
    lv_obj_t *slider_soffio;
    lv_obj_t *btn_soffio_popup;
    lv_obj_t *img_aria_soffio;
    lv_obj_t *img_aria_soffio_popup;

    lv_obj_t *popup_suction;
    lv_obj_t *slider_suction;
    lv_obj_t *btn_suction_popup;
    lv_obj_t *img_aria_suction;
    lv_obj_t *img_aria_suction_popup;

    lv_obj_t *img_aria_aspirazione;

    lv_obj_t *popup_alarm;
    lv_obj_t *lbl_alarm;

    lv_anim_t anim_ventola_aspirazione;
    lv_anim_t anim_ventola_soffio;
    lv_anim_t anim_ventola_soffio_popup;
    lv_anim_t anim_suction_fan;
    lv_anim_t anim_suction_fan_popup;
    lv_anim_t anim_popup_tavolo;
    lv_anim_t anim_popup_bracciolo;

    editing_target_t editing_target;

    parameter_handle_t setpoint_tavolo;
    parameter_handle_t setpoint_bracciolo;

    uint8_t ignore_click;
    uint8_t aspirazione_on;
    uint8_t soffio_on;
};


static lv_obj_t *base_button(lv_obj_t *root, size_t col, size_t row, int id);
static lv_obj_t *image_button(lv_obj_t *root, const lv_img_dsc_t *img, size_t col, size_t row, int id);
static lv_obj_t *heat_image_button(lv_obj_t *root, const lv_img_dsc_t *img_dsc, uint32_t h_shift, size_t col,
                                   size_t row, int id);
static lv_obj_t *iron_image_button(lv_obj_t *root, const lv_img_dsc_t *img_dsc, size_t col, size_t row, int id);
static lv_anim_t fan_animation(lv_obj_t *img, uint32_t period);
static lv_anim_t slide_in_animation(lv_obj_t *obj, int32_t start, int32_t end);
static lv_anim_t fade_animation(lv_obj_t *obj);
static void      update_page(model_t *pmodel, struct page_data *pdata, uint8_t restart_animations);
static lv_obj_t *popup_temperature_create(lv_obj_t *root, lv_obj_t **arc, lv_obj_t **lbl, lv_obj_t **img,
                                          const lv_img_dsc_t *img_dsc, int plus_id, int minus_id);
static lv_obj_t *popup_fan_create(lv_obj_t *root, lv_obj_t **slider, lv_obj_t **btn_fan, lv_obj_t **img_air,
                                  lv_anim_t *anim, const lv_img_dsc_t *air_img_dsc, uint8_t reverse, int plus_id,
                                  int minus_id);
static void      pause_background_animations(struct page_data *pdata);
static void      start_background_animations(struct page_data *pdata);
static lv_obj_t *popup_alarm_create(lv_obj_t *root, lv_obj_t **lbl, int dismiss_id);


static const char *TAG = "PageMain";

static const uint32_t speed_to_period_transform[NUM_SPEED_STEPS] = {3000, 2000, 1500, 1200, 900, 700, 500, 250};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));

    pdata->setpoint_tavolo    = PARAMETER(&pmodel->configuration.setpoint_temperatura_tavolo, 0,
                                          model_get_max_temperatura_tavolo(pmodel), 50, NULL, 0);
    pdata->setpoint_bracciolo = PARAMETER(&pmodel->configuration.setpoint_temperatura_bracciolo, 0,
                                          model_get_max_temperatura_tavolo(pmodel), 50, NULL, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *img;

    pdata->ignore_click   = 0;
    pdata->editing_target = EDITING_TARGET_NONE;
    pdata->soffio_on      = !model_get_soffio_on(pmodel);
    pdata->aspirazione_on = !model_get_aspirazione_on(pmodel);

#define COL_SIZE 95
#define ROW_SIZE 158

    static lv_coord_t col_dsc[] = {COL_SIZE, COL_SIZE, COL_SIZE, COL_SIZE, COL_SIZE, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {ROW_SIZE, ROW_SIZE, LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_add_style(cont, (lv_style_t *)&style_padless_cont, LV_STATE_DEFAULT);

    pdata->btn_tavolo        = heat_image_button(cont, &img_tavolo_off, 24, 0, 0, TAVOLO_BTN_ID);
    pdata->img_tavolo        = lv_obj_get_child(pdata->btn_tavolo, 0);
    pdata->img_calore_tavolo = lv_obj_get_child(pdata->btn_tavolo, 1);
    lv_obj_add_flag(pdata->btn_tavolo, LV_OBJ_FLAG_CHECKABLE);

    pdata->btn_bracciolo        = heat_image_button(cont, &img_bracciolo_off, 32, 0, 1, BRACCIOLO_BTN_ID);
    pdata->img_bracciolo        = lv_obj_get_child(pdata->btn_bracciolo, 0);
    pdata->img_calore_bracciolo = lv_obj_get_child(pdata->btn_bracciolo, 1);
    lv_obj_add_flag(pdata->btn_bracciolo, LV_OBJ_FLAG_CHECKABLE);

    pdata->btn_ferro_1        = iron_image_button(cont, &img_ferro_1_off, 1, 0, FERRO_1_BTN_ID);
    pdata->img_ferro_1        = lv_obj_get_child(pdata->btn_ferro_1, 0);
    pdata->img_vapore_ferro_1 = lv_obj_get_child(pdata->btn_ferro_1, 1);
    lv_obj_add_flag(pdata->btn_ferro_1, LV_OBJ_FLAG_CHECKABLE);

    pdata->btn_ferro_2        = iron_image_button(cont, &img_ferro_2_off, 1, 1, FERRO_2_BTN_ID);
    pdata->img_ferro_2        = lv_obj_get_child(pdata->btn_ferro_2, 0);
    pdata->img_vapore_ferro_2 = lv_obj_get_child(pdata->btn_ferro_2, 1);
    lv_obj_add_flag(pdata->btn_ferro_2, LV_OBJ_FLAG_CHECKABLE);

    btn = base_button(cont, 2, 0, SUCTION_BTN_ID);
    img = lv_img_create(btn);
    lv_img_set_src(img, &img_ventola);
    lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 0, 0);
    pdata->img_ventola_aspirazione = img;
    img                            = lv_img_create(btn);
    lv_img_set_src(img, &img_aria);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
    pdata->img_aria_aspirazione = img;
    pdata->btn_aspirazione      = btn;
    lv_obj_add_flag(pdata->btn_aspirazione, LV_OBJ_FLAG_CHECKABLE);

    pdata->btn_boiler = image_button(cont, &img_boiler_off_3, 2, 1, BOILER_BTN_ID);
    pdata->img_boiler = lv_obj_get_child(pdata->btn_boiler, 0);

    static const lv_img_dsc_t *anim_imgs[3] = {
        &img_bolle_2,
        &img_bolle_3,
    };

    img = lv_animimg_create(pdata->img_boiler);
    lv_animimg_set_src(img, (lv_img_dsc_t **)anim_imgs, 2);
    lv_animimg_set_duration(img, 1500);
    lv_animimg_set_repeat_count(img, LV_ANIM_REPEAT_INFINITE);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_animimg_start(img);
    pdata->img_boiler_bubbles = img;

    lv_obj_add_flag(pdata->btn_boiler, LV_OBJ_FLAG_CHECKABLE);

    btn = base_button(cont, 3, 0, SOFFIO_BTN_ID);
    img = lv_img_create(btn);
    lv_img_set_src(img, &img_ventola);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
    pdata->img_ventola_soffio = img;
    img                       = lv_img_create(btn);
    lv_img_set_src(img, &img_aria);
    lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 0, 0);
    pdata->img_aria_soffio = img;
    pdata->btn_soffio      = btn;
    lv_obj_add_flag(pdata->btn_soffio, LV_OBJ_FLAG_CHECKABLE);

    pdata->btn_fotocellula = image_button(cont, &img_fotocellula_sx, 4, 1, FOTOCELLULA_BTN_ID);

    pdata->btn_luce = image_button(cont, &img_luce_off, 3, 1, LUCE_BTN_ID);
    lv_obj_add_flag(pdata->btn_luce, LV_OBJ_FLAG_CHECKABLE);

    pdata->btn_menu = image_button(cont, &img_menu, 4, 0, MENU_BTN_ID);

    pdata->anim_ventola_aspirazione = fan_animation(pdata->img_ventola_aspirazione, 250);

    pdata->anim_ventola_soffio = fan_animation(pdata->img_ventola_soffio, 2000);
    pdata->anim_suction_fan    = fan_animation(pdata->img_ventola_aspirazione, 2000);

    lv_obj_t *blanket = lv_obj_create(lv_scr_act());
    lv_obj_set_size(blanket, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(blanket, (lv_style_t *)&style_blanket, LV_STATE_DEFAULT);
    lv_obj_add_flag(blanket, LV_OBJ_FLAG_CLICKABLE);
    view_register_object_default_callback(blanket, BLANKET_ID);
    pdata->blanket = blanket;

    lv_obj_t *popup = popup_temperature_create(pdata->blanket, &pdata->arc_setpoint_tavolo, &pdata->lbl_setpoint_tavolo,
                                               &pdata->img_tavolo_popup, &img_tavolo_off, SETPOINT_TAVOLO_PLUS_BTN_ID,
                                               SETPOINT_TAVOLO_MINUS_BTN_ID);

    lv_arc_set_range(pdata->arc_setpoint_tavolo, 0, model_get_max_temperatura_tavolo(pmodel));
    view_register_object_default_callback(pdata->arc_setpoint_tavolo, SETPOINT_TAVOLO_ARC_ID);

    lv_obj_align(popup, LV_ALIGN_TOP_MID, 0, -POPUP_HEIGHT);
    pdata->anim_popup_tavolo = slide_in_animation(popup, -POPUP_HEIGHT, (LV_VER_RES - POPUP_HEIGHT) / 2);
    pdata->popup_tavolo      = popup;


    popup = popup_temperature_create(pdata->blanket, &pdata->arc_setpoint_bracciolo, &pdata->lbl_setpoint_bracciolo,
                                     &pdata->img_bracciolo_popup, &img_bracciolo_off, SETPOINT_BRACCIOLO_PLUS_BTN_ID,
                                     SETPOINT_BRACCIOLO_MINUS_BTN_ID);

    lv_arc_set_range(pdata->arc_setpoint_bracciolo, 0, model_get_max_temperatura_bracciolo(pmodel));
    view_register_object_default_callback(pdata->arc_setpoint_bracciolo, SETPOINT_BRACCIOLO_ARC_ID);

    lv_obj_align(popup, LV_ALIGN_BOTTOM_MID, 0, POPUP_HEIGHT);
    pdata->anim_popup_bracciolo = slide_in_animation(popup, POPUP_HEIGHT, -(LV_VER_RES - POPUP_HEIGHT) / 2);
    pdata->popup_bracciolo      = popup;


    popup = popup_fan_create(pdata->blanket, &pdata->slider_soffio, &pdata->btn_soffio_popup,
                             &pdata->img_aria_soffio_popup, &pdata->anim_ventola_soffio_popup, &img_aria, 0,
                             FAN_SOFFIO_PLUS_BTN_ID, FAN_SOFFIO_MINUS_BTN_ID);
    view_register_object_default_callback(pdata->btn_soffio_popup, SOFFIO_POPUP_BTN_ID);
    view_register_object_default_callback(pdata->slider_soffio, VELOCITA_SOFFIO_SLIDER_ID);
    lv_obj_align(popup, LV_ALIGN_CENTER, 0, 0);
    pdata->popup_soffio = popup;

    popup = popup_fan_create(pdata->blanket, &pdata->slider_suction, &pdata->btn_suction_popup,
                             &pdata->img_aria_suction_popup, &pdata->anim_suction_fan_popup, &img_aria, 1,
                             FAN_SUCTION_PLUS_BTN_ID, FAN_SUCTION_MINUS_BTN_ID);
    view_register_object_default_callback(pdata->btn_suction_popup, SUCTION_POPUP_BTN_ID);
    view_register_object_default_callback(pdata->slider_suction, FAN_SUCTION_SLIDER_ID);
    lv_obj_align(popup, LV_ALIGN_CENTER, 0, 0);
    pdata->popup_suction = popup;

    pdata->popup_alarm = popup_alarm_create(lv_scr_act(), &pdata->lbl_alarm, BTN_DISMISS_ALARM_ID);

    update_page(pmodel, pdata, 1);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_UPDATE:
            update_page(pmodel, pdata, 0);
            break;

        case VIEW_EVENT_CODE_VAPORE:
            lv_obj_set_style_opa(pdata->img_vapore_ferro_1, LV_OPA_COVER, LV_STATE_DEFAULT);
            lv_obj_fade_out(pdata->img_vapore_ferro_1, ANIM_VAPORE_PERIOD, 0);
            lv_animimg_start(pdata->img_vapore_ferro_1);
            // lv_obj_set_style_opa(pdata->img_vapore_ferro_2, LV_OPA_COVER, LV_STATE_DEFAULT);
            // lv_obj_fade_out(pdata->img_vapore_ferro_2, ANIM_VAPORE_PERIOD, 0);
            // lv_animimg_start(pdata->img_vapore_ferro_2);
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BTN_DISMISS_ALARM_ID:
                            model_set_alarm_communication(pmodel, 0);
                            update_page(pmodel, pdata, 0);
                            break;

                        case MENU_BTN_ID: {
                            view_page_message_t pw_msg = {
                                .code = VIEW_PAGE_MESSAGE_CODE_SWAP,
                                .page = &page_test_output,
                            };
                            password_page_options_t *opts =
                                view_common_default_password_page_options(pw_msg, (const char *)APP_CONFIG_PASSWORD);
                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.extra = opts;
                            msg.vmsg.page  = &page_password;
                            break;
                        }

                        case FERRO_1_BTN_ID:
                            model_toggle_ferro_1(pmodel);
                            update_page(pmodel, pdata, 0);
                            break;

                        case FERRO_2_BTN_ID:
                            model_toggle_ferro_2(pmodel);
                            update_page(pmodel, pdata, 0);
                            break;

                        case LUCE_BTN_ID:
                            model_toggle_luce(pmodel);
                            update_page(pmodel, pdata, 0);
                            break;

                        case FOTOCELLULA_BTN_ID:
                            model_toggle_fotocellula(pmodel);
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_TAVOLO_PLUS_BTN_ID:
                            parameter_operator(&pdata->setpoint_tavolo, +1);
                            msg.beep = 1;
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_TAVOLO_MINUS_BTN_ID:
                            parameter_operator(&pdata->setpoint_tavolo, -1);
                            msg.beep = 1;
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_BRACCIOLO_PLUS_BTN_ID:
                            parameter_operator(&pdata->setpoint_bracciolo, +1);
                            msg.beep = 1;
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_BRACCIOLO_MINUS_BTN_ID:
                            parameter_operator(&pdata->setpoint_bracciolo, -1);
                            msg.beep = 1;
                            update_page(pmodel, pdata, 0);
                            break;

                        case FAN_SOFFIO_PLUS_BTN_ID:
                            if (model_get_velocita_soffio(pmodel) < NUM_SPEED_STEPS - 1) {
                                model_set_velocita_soffio(pmodel, model_get_velocita_soffio(pmodel) + 1);
                            }
                            msg.beep = 1;
                            update_page(pmodel, pdata, 1);
                            break;

                        case FAN_SOFFIO_MINUS_BTN_ID:
                            if (model_get_velocita_soffio(pmodel) > 0) {
                                model_set_velocita_soffio(pmodel, model_get_velocita_soffio(pmodel) - 1);
                            }
                            msg.beep = 1;
                            update_page(pmodel, pdata, 1);
                            break;

                        case FAN_SUCTION_PLUS_BTN_ID:
                            if (model_get_velocita_aspirazione(pmodel) < NUM_SPEED_STEPS - 1) {
                                model_set_velocita_aspirazione(pmodel, model_get_velocita_aspirazione(pmodel) + 1);
                            }
                            msg.beep = 1;
                            update_page(pmodel, pdata, 1);
                            break;

                        case FAN_SUCTION_MINUS_BTN_ID:
                            if (model_get_velocita_aspirazione(pmodel) > 0) {
                                model_set_velocita_aspirazione(pmodel, model_get_velocita_aspirazione(pmodel) - 1);
                            }
                            msg.beep = 1;
                            update_page(pmodel, pdata, 1);
                            break;

                        case BLANKET_ID:
                            pdata->editing_target = EDITING_TARGET_NONE;
                            update_page(pmodel, pdata, 1);
                            break;

                        case SOFFIO_BTN_ID:
                            if (pdata->editing_target == EDITING_TARGET_NONE) {
                                pdata->editing_target = EDITING_TARGET_SOFFIO;
                                update_page(pmodel, pdata, 1);
                                msg.beep = 1;
                            }
                            break;

                        case SUCTION_BTN_ID:
                            if (pdata->editing_target == EDITING_TARGET_NONE) {
                                pdata->editing_target = EDITING_TARGET_SUCTION;
                                update_page(pmodel, pdata, 1);
                                msg.beep = 1;
                            }
                            break;
                    }

                    break;
                }

                case LV_EVENT_LONG_PRESSED: {
                    switch (event.data.id) {
                        case TAVOLO_BTN_ID:
                            if (pdata->editing_target == EDITING_TARGET_NONE) {
                                pdata->editing_target = EDITING_TARGET_TEMPERATURA_TAVOLO;
                                lv_anim_start(&pdata->anim_popup_tavolo);
                                update_page(pmodel, pdata, 0);
                                msg.beep            = 1;
                                pdata->ignore_click = 1;
                            }
                            break;

                        case BRACCIOLO_BTN_ID:
                            if (pdata->editing_target == EDITING_TARGET_NONE) {
                                pdata->editing_target = EDITING_TARGET_TEMPERATURA_BRACCIOLO;
                                lv_anim_start(&pdata->anim_popup_bracciolo);
                                update_page(pmodel, pdata, 0);
                                msg.beep            = 1;
                                pdata->ignore_click = 1;
                            }
                            break;
                    }
                    break;
                }

                case LV_EVENT_LONG_PRESSED_REPEAT: {
                    switch (event.data.id) {
                        case SETPOINT_TAVOLO_PLUS_BTN_ID:
                            parameter_operator(&pdata->setpoint_tavolo, +1);
                            msg.beep = 1;
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_TAVOLO_MINUS_BTN_ID:
                            parameter_operator(&pdata->setpoint_tavolo, -1);
                            msg.beep = 1;
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_BRACCIOLO_PLUS_BTN_ID:
                            parameter_operator(&pdata->setpoint_bracciolo, +1);
                            msg.beep = 1;
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_BRACCIOLO_MINUS_BTN_ID:
                            parameter_operator(&pdata->setpoint_bracciolo, -1);
                            msg.beep = 1;
                            update_page(pmodel, pdata, 0);
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case BOILER_BTN_ID:
                            model_toggle_richiesta_boiler(pmodel);
                            msg.cmsg.code = VIEW_CONTROLLER_MESSAGE_CODE_TOGGLE_BOILER;
                            break;

                        case TAVOLO_BTN_ID:
                            if (pdata->ignore_click) {
                                pdata->ignore_click = 0;
                                break;
                            }
                            model_toggle_richiesta_temperatura_tavolo(pmodel);
                            lv_animimg_start(pdata->img_calore_tavolo);
                            update_page(pmodel, pdata, 0);
                            break;

                        case BRACCIOLO_BTN_ID:
                            if (pdata->ignore_click) {
                                pdata->ignore_click = 0;
                                break;
                            }
                            model_toggle_richiesta_temperatura_bracciolo(pmodel);
                            lv_animimg_start(pdata->img_calore_bracciolo);
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_TAVOLO_ARC_ID:
                            model_set_richiesta_temperatura_tavolo(pmodel, 1);
                            model_set_setpoint_temperatura_tavolo(pmodel, event.value);
                            update_page(pmodel, pdata, 0);
                            break;

                        case SETPOINT_BRACCIOLO_ARC_ID:
                            model_set_richiesta_temperatura_bracciolo(pmodel, 1);
                            model_set_setpoint_temperatura_bracciolo(pmodel, event.value);
                            update_page(pmodel, pdata, 0);
                            break;

                        case VELOCITA_SOFFIO_SLIDER_ID:
                            model_set_velocita_soffio(pmodel, event.value);
                            update_page(pmodel, pdata, 1);
                            break;

                        case FAN_SUCTION_SLIDER_ID:
                            model_set_velocita_aspirazione(pmodel, event.value);
                            update_page(pmodel, pdata, 1);
                            break;
                    }
                    break;
                }

                case LV_EVENT_RELEASED: {
                    switch (event.data.id) {
                        case TAVOLO_BTN_ID:
                        case BRACCIOLO_BTN_ID:
                        case SOFFIO_BTN_ID:
                        case SOFFIO_POPUP_BTN_ID:
                        case SUCTION_BTN_ID:
                            update_page(pmodel, pdata, 0);
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


static void update_page(model_t *pmodel, struct page_data *pdata, uint8_t restart_animations) {
    if (model_get_soffio_on(pmodel)) {
        lv_anim_set_time(&pdata->anim_ventola_soffio, speed_to_period_transform[model_get_velocita_soffio(pmodel)]);
        lv_anim_set_time(&pdata->anim_ventola_soffio_popup,
                         speed_to_period_transform[model_get_velocita_soffio(pmodel)]);
        if (!pdata->soffio_on || restart_animations) {
            lv_anim_start(&pdata->anim_ventola_soffio);
            lv_anim_start(&pdata->anim_ventola_soffio_popup);
        }
        view_common_set_checked(pdata->btn_soffio, 1);
        view_common_set_checked(pdata->btn_soffio_popup, 1);
        view_common_img_set_src(pdata->img_aria_soffio, &img_aria_on);
        view_common_img_set_src(pdata->img_aria_soffio_popup, &img_aria_on);
        pdata->soffio_on = 1;
    } else {
        lv_anim_custom_del(&pdata->anim_ventola_soffio, NULL);
        lv_anim_custom_del(&pdata->anim_ventola_soffio_popup, NULL);
        view_common_set_checked(pdata->btn_soffio, 0);
        view_common_set_checked(pdata->btn_soffio_popup, 0);
        view_common_img_set_src(pdata->img_aria_soffio, &img_aria);
        view_common_img_set_src(pdata->img_aria_soffio_popup, &img_aria);
        pdata->soffio_on = 0;
    }

    if (model_get_aspirazione_on(pmodel)) {
        if (!pdata->aspirazione_on || restart_animations) {
            lv_anim_start(&pdata->anim_ventola_aspirazione);
            lv_anim_start(&pdata->anim_suction_fan_popup);
        }
        view_common_set_checked(pdata->btn_aspirazione, 1);
        view_common_set_checked(pdata->btn_suction_popup, 1);
        view_common_img_set_src(pdata->img_aria_aspirazione, &img_aria_on);
        view_common_img_set_src(pdata->img_aria_suction_popup, &img_aria_on);
        pdata->aspirazione_on = 1;
    } else {
        lv_anim_custom_del(&pdata->anim_ventola_aspirazione, NULL);
        lv_anim_custom_del(&pdata->anim_suction_fan_popup, NULL);
        view_common_set_checked(pdata->btn_aspirazione, 0);
        view_common_set_checked(pdata->btn_suction_popup, 0);
        view_common_img_set_src(pdata->img_aria_aspirazione, &img_aria);
        view_common_img_set_src(pdata->img_aria_suction_popup, &img_aria);
        pdata->aspirazione_on = 0;
    }

    view_common_set_checked(pdata->btn_luce, model_get_luce(pmodel));
    view_common_img_set_src(lv_obj_get_child(pdata->btn_luce, 0),
                            model_get_luce(pmodel) ? &img_luce_on : &img_luce_off);

    view_common_set_checked(pdata->btn_tavolo, model_get_richiesta_temperatura_tavolo(pmodel));
    view_common_set_hidden(pdata->img_calore_tavolo, !model_get_tavolo_on(pmodel));
    view_common_img_set_src(pdata->img_tavolo,
                            model_get_richiesta_temperatura_tavolo(pmodel) ? &img_tavolo_on : &img_tavolo_off);
    view_common_img_set_src(pdata->img_tavolo_popup,
                            model_get_richiesta_temperatura_tavolo(pmodel) ? &img_tavolo_on : &img_tavolo_off);

    view_common_set_checked(pdata->btn_bracciolo, model_get_richiesta_temperatura_bracciolo(pmodel));
    view_common_set_hidden(pdata->img_calore_bracciolo, !model_get_bracciolo_on(pmodel));
    view_common_img_set_src(pdata->img_bracciolo,
                            model_get_richiesta_temperatura_bracciolo(pmodel) ? &img_bracciolo_on : &img_bracciolo_off);
    view_common_img_set_src(pdata->img_bracciolo_popup,
                            model_get_richiesta_temperatura_bracciolo(pmodel) ? &img_bracciolo_on : &img_bracciolo_off);

    view_common_img_set_src(pdata->img_ferro_1, model_get_ferro_1(pmodel) ? &img_ferro_1_on : &img_ferro_1_off);
    view_common_img_set_src(pdata->img_ferro_2, model_get_ferro_2(pmodel) ? &img_ferro_2_on : &img_ferro_2_off);

    view_common_set_checked(pdata->btn_boiler, model_get_richiesta_boiler(pmodel));
    if (model_boiler_pieno(pmodel)) {
        view_common_img_set_src(pdata->img_boiler, model_get_boiler_on(pmodel) ? &img_boiler_on_2 : &img_boiler_off_2);
    } else {
        view_common_img_set_src(pdata->img_boiler, model_get_pompa_on(pmodel) ? &img_boiler_off_1 : &img_boiler_off_0);
    }

    view_common_set_hidden(pdata->img_boiler_bubbles, !model_get_pompa_on(pmodel));

    switch (pdata->editing_target) {
        case EDITING_TARGET_NONE:
            view_common_set_hidden(pdata->blanket, 1);

            if (restart_animations) {
                start_background_animations(pdata);
            }

            switch (model_get_fotocellula(pmodel)) {
                case FOTOCELLULA_SX:
                    view_common_img_set_src(lv_obj_get_child(pdata->btn_fotocellula, 0), &img_fotocellula_sx);
                    break;
                case FOTOCELLULA_DX:
                    view_common_img_set_src(lv_obj_get_child(pdata->btn_fotocellula, 0), &img_fotocellula_dx);
                    break;
            }
            break;

        case EDITING_TARGET_TEMPERATURA_BRACCIOLO:
            view_common_set_hidden(pdata->blanket, 0);
            view_common_set_hidden(pdata->popup_bracciolo, 0);
            view_common_set_hidden(pdata->popup_tavolo, 1);
            view_common_set_hidden(pdata->popup_soffio, 1);
            view_common_set_hidden(pdata->popup_suction, 1);

            pause_background_animations(pdata);

            lv_obj_align_to(pdata->lbl_setpoint_bracciolo, pdata->arc_setpoint_bracciolo, LV_ALIGN_CENTER, -16, 0);
            lv_label_set_text_fmt(pdata->lbl_setpoint_bracciolo, "%i",
                                  model_get_setpoint_temperatura_bracciolo(pmodel));
            lv_arc_set_value(pdata->arc_setpoint_bracciolo, model_get_setpoint_temperatura_bracciolo(pmodel));
            break;

        case EDITING_TARGET_TEMPERATURA_TAVOLO:
            view_common_set_hidden(pdata->blanket, 0);
            view_common_set_hidden(pdata->popup_bracciolo, 1);
            view_common_set_hidden(pdata->popup_tavolo, 0);
            view_common_set_hidden(pdata->popup_soffio, 1);
            view_common_set_hidden(pdata->popup_suction, 1);

            pause_background_animations(pdata);

            lv_obj_align_to(pdata->lbl_setpoint_tavolo, pdata->arc_setpoint_tavolo, LV_ALIGN_CENTER, -16, 0);
            lv_label_set_text_fmt(pdata->lbl_setpoint_tavolo, "%i", model_get_setpoint_temperatura_tavolo(pmodel));
            lv_arc_set_value(pdata->arc_setpoint_tavolo, model_get_setpoint_temperatura_tavolo(pmodel));
            break;

        case EDITING_TARGET_SOFFIO:
            view_common_set_hidden(pdata->blanket, 0);
            view_common_set_hidden(pdata->popup_bracciolo, 1);
            view_common_set_hidden(pdata->popup_tavolo, 1);
            view_common_set_hidden(pdata->popup_soffio, 0);
            view_common_set_hidden(pdata->popup_suction, 1);

            pause_background_animations(pdata);

            lv_slider_set_value(pdata->slider_soffio, model_get_velocita_soffio(pmodel), LV_ANIM_OFF);
            break;

        case EDITING_TARGET_SUCTION:
            view_common_set_hidden(pdata->blanket, 0);
            view_common_set_hidden(pdata->popup_bracciolo, 1);
            view_common_set_hidden(pdata->popup_tavolo, 1);
            view_common_set_hidden(pdata->popup_soffio, 1);
            view_common_set_hidden(pdata->popup_suction, 0);

            pause_background_animations(pdata);

            lv_slider_set_value(pdata->slider_suction, model_get_velocita_aspirazione(pmodel), LV_ANIM_OFF);
            break;
    }


    view_common_set_hidden(pdata->popup_alarm, !model_get_alarm_communication(pmodel));
    lv_label_set_text(pdata->lbl_alarm, "Allarme comunicazione!");
}


static void pause_background_animations(struct page_data *pdata) {
    lv_anim_custom_del(&pdata->anim_ventola_aspirazione, NULL);
    lv_anim_custom_del(&pdata->anim_ventola_soffio, NULL);
}


static void start_background_animations(struct page_data *pdata) {}


static lv_obj_t *base_button(lv_obj_t *root, size_t col, size_t row, int id) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_size(btn, COL_SIZE - 4, ROW_SIZE - 4);
    lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, col, 1, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_add_style(btn, (lv_style_t *)&style_black_border, LV_STATE_DEFAULT);
    lv_obj_add_style(btn, (lv_style_t *)&style_btn_checked, LV_STATE_CHECKED);
    view_register_object_default_callback(btn, id);

    return btn;
}


static lv_obj_t *heat_image_button(lv_obj_t *root, const lv_img_dsc_t *img_dsc, uint32_t h_shift, size_t col,
                                   size_t row, int id) {
    lv_obj_t *btn = base_button(root, col, row, id);

    lv_obj_t *img = lv_img_create(btn);
    lv_img_set_src(img, img_dsc);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, h_shift);

    static const lv_img_dsc_t *anim_imgs[3] = {
        &img_calore_1,
        &img_calore_2,
        &img_calore_3,
    };

    img = lv_animimg_create(btn);
    lv_animimg_set_src(img, (lv_img_dsc_t **)anim_imgs, 3);
    lv_animimg_set_duration(img, 3000);
    lv_animimg_set_repeat_count(img, LV_ANIM_REPEAT_INFINITE);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_animimg_start(img);

    return btn;
}


static lv_obj_t *iron_image_button(lv_obj_t *root, const lv_img_dsc_t *img_dsc, size_t col, size_t row, int id) {
    lv_obj_t *btn = base_button(root, col, row, id);

    lv_obj_t *img = lv_img_create(btn);
    lv_img_set_src(img, img_dsc);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, -16);

    static const lv_img_dsc_t *anim_imgs[3] = {
        &img_vapore_1,
        &img_vapore_2,
        &img_vapore_3,
    };

    img = lv_animimg_create(btn);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(img, lv_color_make(0xff, 0xff, 0xff), LV_STATE_DEFAULT);
    lv_animimg_set_src(img, (lv_img_dsc_t **)anim_imgs, 3);
    lv_animimg_set_duration(img, ANIM_VAPORE_PERIOD);
    lv_animimg_set_repeat_count(img, 1);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 24);

    return btn;
}



static lv_obj_t *image_button(lv_obj_t *root, const lv_img_dsc_t *img_dsc, size_t col, size_t row, int id) {
    lv_obj_t *btn = base_button(root, col, row, id);

    lv_obj_t *img = lv_img_create(btn);
    lv_img_set_src(img, img_dsc);
    lv_obj_center(img);

    return btn;
}


static lv_obj_t *popup_temperature_create(lv_obj_t *root, lv_obj_t **arc, lv_obj_t **lbl, lv_obj_t **img,
                                          const lv_img_dsc_t *img_dsc, int plus_id, int minus_id) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(cont, (lv_style_t *)&style_popup, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, POPUP_WIDTH, POPUP_HEIGHT);
    lv_obj_center(cont);

    *arc = lv_arc_create(cont);
    lv_obj_set_size(*arc, 280, 240);
    lv_arc_set_rotation(*arc, 135);
    lv_arc_set_bg_angles(*arc, 0, 270);
    lv_obj_align(*arc, LV_ALIGN_CENTER, -8, 8);
    lv_obj_add_style(*arc, (lv_style_t *)&style_arc, LV_STATE_DEFAULT);
    lv_obj_add_style(*arc, (lv_style_t *)&style_arc_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(*arc, (lv_style_t *)&style_arc_knob, LV_PART_KNOB);
    view_register_object_default_callback(*arc, SETPOINT_TAVOLO_ARC_ID);

    *img = lv_img_create(cont);
    lv_img_set_src(*img, img_dsc);
    lv_img_set_zoom(*img, 320);
    lv_obj_align_to(*img, *arc, LV_ALIGN_BOTTOM_MID, -20, -16);

    *lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(*lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_obj_align_to(*lbl, *arc, LV_ALIGN_CENTER, -16, 0);

    lv_obj_t *degrees_lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(degrees_lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_align_to(degrees_lbl, *img, LV_ALIGN_CENTER, 12, 0);
    lv_label_set_text(degrees_lbl, "°C");

    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 48, 48);
    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(btn_lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(btn_lbl, LV_SYMBOL_CLOSE);
    lv_obj_center(btn_lbl);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    view_register_object_default_callback(btn, BLANKET_ID);

    btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 48, 48);
    btn_lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(btn_lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(btn_lbl, LV_SYMBOL_PLUS);
    lv_obj_center(btn_lbl);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, 0, 32);
    view_register_object_default_callback(btn, plus_id);

    btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 48, 48);
    btn_lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(btn_lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(btn_lbl, LV_SYMBOL_MINUS);
    lv_obj_center(btn_lbl);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, 0, 96);
    view_register_object_default_callback(btn, minus_id);

    return cont;
}


static void slider_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t       *obj  = lv_event_get_target(e);

    /*Provide some extra space for the value*/
    if (code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_event_set_ext_draw_size(e, 50);
    } else if (code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->part == LV_PART_KNOB) {
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%i", (int)lv_slider_get_value(obj) + 1);

            lv_point_t label_size;
            lv_txt_get_size(&label_size, buf, STYLE_FONT_BIG, 0, 0, LV_COORD_MAX, 0);
            lv_area_t label_area;
            label_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - label_size.x / 2;
            label_area.x2 = label_area.x1 + label_size.x;
            if (lv_slider_is_dragged(obj)) {
                label_area.y2 = dsc->draw_area->y1;
                label_area.y1 = label_area.y2 - label_size.y;
            } else {
                // label_area.y1 = dsc->draw_area->y1;
                label_area.y1 = label_area.y2 - label_size.y + 20;
            }

            lv_draw_label_dsc_t label_draw_dsc;
            lv_draw_label_dsc_init(&label_draw_dsc);
            label_draw_dsc.color = STYLE_LIGHT;
            label_draw_dsc.font  = STYLE_FONT_BIG;
            lv_draw_label(dsc->draw_ctx, &label_draw_dsc, &label_area, buf, NULL);
        }
    }
}


static lv_obj_t *popup_fan_create(lv_obj_t *root, lv_obj_t **slider, lv_obj_t **btn_fan, lv_obj_t **img_air,
                                  lv_anim_t *anim, const lv_img_dsc_t *air_img_dsc, uint8_t reverse, int plus_id,
                                  int minus_id) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(cont, (lv_style_t *)&style_popup, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, FAN_POPUP_WIDTH, FAN_POPUP_HEIGHT);
    lv_obj_center(cont);

    *btn_fan = lv_btn_create(cont);
    lv_obj_set_size(*btn_fan, 104, 104);
    lv_obj_add_style(*btn_fan, (lv_style_t *)&style_black_border, LV_STATE_DEFAULT);
    lv_obj_add_flag(*btn_fan, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_style(*btn_fan, (lv_style_t *)&style_btn_checked, LV_STATE_CHECKED);
    lv_obj_clear_flag(*btn_fan, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *img_fan = lv_img_create(*btn_fan);
    lv_img_set_src(img_fan, &img_ventola);
    lv_img_set_zoom(img_fan, 320);
    lv_obj_align(img_fan, LV_ALIGN_CENTER, 0, 0);

    *img_air = lv_img_create(cont);
    lv_img_set_src(*img_air, air_img_dsc);
    lv_img_set_zoom(*img_air, 400);

    if (reverse) {
        lv_obj_align(*btn_fan, LV_ALIGN_CENTER, 64 - 32, -32);
        lv_obj_align(*img_air, LV_ALIGN_CENTER, -72 - 32, -32);
        lv_img_set_angle(*img_air, -900);
    } else {
        lv_obj_align(*btn_fan, LV_ALIGN_CENTER, -64 - 32, -32);
        lv_obj_align(*img_air, LV_ALIGN_CENTER, 72 - 32, -32);
        lv_img_set_angle(*img_air, -900);
    }

    *slider = lv_slider_create(cont);
    lv_obj_add_style(*slider, (lv_style_t *)&style_speed_slider, LV_PART_INDICATOR);
    lv_obj_add_style(*slider, (lv_style_t *)&style_speed_slider_knob, LV_PART_KNOB);
    lv_obj_add_style(*slider, (lv_style_t *)&style_speed_slider_pressed, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(*slider, (lv_style_t *)&style_speed_slider_knob_pressed, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_slider_set_range(*slider, 0, NUM_SPEED_STEPS - 1);
    lv_obj_set_size(*slider, 240, 32);
    lv_obj_align(*slider, LV_ALIGN_BOTTOM_MID, -32, -24);
    lv_obj_add_event_cb(*slider, slider_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(*slider);

    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 48, 48);
    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(btn_lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(btn_lbl, LV_SYMBOL_CLOSE);
    lv_obj_center(btn_lbl);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    view_register_object_default_callback(btn, BLANKET_ID);

    btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 48, 48);
    btn_lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(btn_lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(btn_lbl, LV_SYMBOL_PLUS);
    lv_obj_center(btn_lbl);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, 0, 32);
    view_register_object_default_callback(btn, plus_id);

    btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 48, 48);
    btn_lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(btn_lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(btn_lbl, LV_SYMBOL_MINUS);
    lv_obj_center(btn_lbl);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, 0, 96);
    view_register_object_default_callback(btn, minus_id);

    *anim = fan_animation(img_fan, 250);

    return cont;
}


static lv_obj_t *popup_alarm_create(lv_obj_t *root, lv_obj_t **lbl, int dismiss_id) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(cont, (lv_style_t *)&style_popup, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, 240, 240);
    lv_obj_center(cont);

    *lbl = lv_label_create(cont);
    lv_obj_align(*lbl, LV_ALIGN_CENTER, 0, -16);
    lv_label_set_long_mode(*lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(*lbl, 200);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img, &img_warning_lg);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 48);

    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 48, 48);
    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(btn_lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(btn_lbl, LV_SYMBOL_CLOSE);
    lv_obj_center(btn_lbl);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    view_register_object_default_callback(btn, dismiss_id);

    return cont;
}


static lv_anim_t slide_in_animation(lv_obj_t *obj, int32_t start, int32_t end) {
    lv_anim_t a;
    lv_anim_init(&a);
    /*Set the "animator" function*/
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    /* Ease out animation */
    // lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    /*Set target of the animation*/
    lv_anim_set_var(&a, obj);
    /*Length of the animation [ms]*/
    lv_anim_set_time(&a, 200);
    /*Set start and end values. E.g. 0, 150*/
    lv_anim_set_values(&a, start, end);
    /* OPTIONAL SETTINGS
     *------------------*/
    /*Number of repetitions. Default is 1. LV_ANIM_REPEAT_INFINITE for infinite repetition*/
    lv_anim_set_repeat_count(&a, 1);
    return a;
}


static void fade_cb(lv_obj_t *obj, int32_t transp) {
    lv_obj_set_style_opa(obj, transp, LV_STATE_DEFAULT);
}


static lv_anim_t fade_animation(lv_obj_t *obj) {
    lv_anim_t a;
    lv_anim_init(&a);
    /*Set the "animator" function*/
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)fade_cb);
    /* Ease out animation */
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    /*Set target of the animation*/
    lv_anim_set_var(&a, obj);
    /*Length of the animation [ms]*/
    lv_anim_set_time(&a, 1200);
    /*Set start and end values. E.g. 0, 150*/
    lv_anim_set_values(&a, LV_OPA_MAX, LV_OPA_MIN);
    /* OPTIONAL SETTINGS
     *------------------*/
    /*Number of repetitions. Default is 1. LV_ANIM_REPEAT_INFINITE for infinite repetition*/
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    return a;
}


static lv_anim_t fan_animation(lv_obj_t *img, uint32_t period) {
    lv_anim_t a;
    lv_anim_init(&a);
    /*Set the "animator" function*/
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
    /*Set target of the animation*/
    lv_anim_set_var(&a, img);
    /*Length of the animation [ms]*/
    lv_anim_set_time(&a, period);
    /*Set start and end values. E.g. 0, 150*/
    lv_anim_set_values(&a, 0, 3600);
    /* OPTIONAL SETTINGS
     *------------------*/
    /*Time to wait before starting the animation [ms]*/
    lv_anim_set_delay(&a, 0);
    /*Number of repetitions. Default is 1. LV_ANIM_REPEAT_INFINITE for infinite repetition*/
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    /* START THE ANIMATION
     *------------------*/
    return a;
}


const pman_page_t page_main = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
