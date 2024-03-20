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
    PLUS_BTN_ID,
    MINUS_BTN_ID,
    ROLLER_ID,
    ONOFF_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_value;
    lv_obj_t *btn_minus;
    lv_obj_t *btn_plus;
    lv_obj_t *btn_onoff;
    lv_obj_t *roller;

    number_parameter_metadata_t *metadata;
    int                          value;
};


static void page_update(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);

    pdata->metadata = extra;
    pdata->value    = pdata->metadata->initial_value;

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *cont, *lbl, *btn;

    cont = view_common_create_title(lv_scr_act(), pdata->metadata->name, BACK_BTN_ID, -1, -1);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    if (pdata->metadata->step > 0) {
        lbl = lv_label_create(cont);
        lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
        lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
        pdata->lbl_value = lbl;

        btn = lv_btn_create(cont);
        lv_obj_set_size(btn, 64, 64);
        lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
        lv_obj_center(lbl);
        lv_label_set_text(lbl, LV_SYMBOL_MINUS);
        lv_obj_align_to(btn, pdata->lbl_value, LV_ALIGN_LEFT_MID, -120, 0);
        view_register_object_default_callback(btn, MINUS_BTN_ID);
        pdata->btn_minus = btn;

        btn = lv_btn_create(cont);
        lv_obj_set_size(btn, 64, 64);
        lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
        lv_obj_center(lbl);
        lv_label_set_text(lbl, LV_SYMBOL_PLUS);
        lv_obj_align_to(btn, pdata->lbl_value, LV_ALIGN_RIGHT_MID, 120, 0);
        view_register_object_default_callback(btn, PLUS_BTN_ID);
        pdata->btn_plus = btn;

        pdata->roller    = NULL;
        pdata->btn_onoff = NULL;
    } else if (pdata->metadata->min == 0 && pdata->metadata->max == 1) {
        lv_obj_t *btn = lv_btn_create(cont);
        lv_obj_set_style_bg_color(btn, lv_color_make(0xFF, 0, 0), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_make(0, 0xFF, 0), LV_STATE_CHECKED);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
        view_common_set_checked(btn, pdata->value);
        lv_obj_set_size(btn, 120, 80);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
        lv_obj_center(lbl);

        lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
        view_register_object_default_callback(btn, ONOFF_BTN_ID);

        pdata->btn_onoff = btn;

        pdata->lbl_value = lbl;

        pdata->btn_minus = NULL;
        pdata->btn_plus  = NULL;
        pdata->roller    = NULL;
    } else {
        lv_obj_t *roller = lv_roller_create(cont);

        char options[512] = {0};

        for (int value = pdata->metadata->min; value <= pdata->metadata->max; value++) {
            size_t len = strlen(options);
            pdata->metadata->to_string(&options[len], sizeof(options) - len, value);
            if (value != pdata->metadata->max) {
                strcat(options, "\n");
            }
        }

        lv_roller_set_visible_row_count(roller, 4);
        lv_roller_set_options(roller, options, LV_ROLLER_MODE_NORMAL);
        lv_obj_set_style_text_font(roller, STYLE_FONT_BIG, LV_STATE_DEFAULT);
        lv_obj_center(roller);
        view_register_object_default_callback(roller, ROLLER_ID);
        pdata->roller = roller;

        pdata->lbl_value = NULL;
        pdata->btn_minus = NULL;
        pdata->btn_plus  = NULL;
    }

    page_update(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_UPDATE:
            break;

        case VIEW_EVENT_CODE_OPEN:
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case ROLLER_ID:
                            pdata->value = pdata->metadata->min + lv_roller_get_selected(pdata->roller);
                            page_update(pmodel, pdata);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case LV_EVENT_LONG_PRESSED_REPEAT:
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            pdata->metadata->update(pmodel, pdata->value);
                            break;

                        case ONOFF_BTN_ID:
                            pdata->value = !pdata->value;
                            page_update(pmodel, pdata);
                            break;

                        case MINUS_BTN_ID:
                            if (pdata->value > pdata->metadata->min) {
                                pdata->value -= pdata->metadata->step;
                                pdata->value -= pdata->value % pdata->metadata->step;
                            } else {
                                pdata->value = pdata->metadata->min;
                            }
                            page_update(pmodel, pdata);
                            break;

                        case PLUS_BTN_ID:
                            if (pdata->value < pdata->metadata->max) {
                                pdata->value += pdata->metadata->step;
                                pdata->value -= pdata->value % pdata->metadata->step;
                            } else {
                                pdata->value = pdata->metadata->max;
                            }
                            page_update(pmodel, pdata);
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


static void page_update(model_t *pmodel, struct page_data *pdata) {
    if (pdata->lbl_value != NULL) {
        if (pdata->btn_onoff != NULL) {
            lv_label_set_text(pdata->lbl_value, pdata->value ? "ON" : "OFF");
            view_common_set_checked(pdata->btn_onoff, pdata->value);
        } else {
            char string[32] = {0};
            pdata->metadata->to_string(string, sizeof(string), pdata->value);
            lv_label_set_text(pdata->lbl_value, string);
        }
    }

    if (pdata->roller != NULL) {
        lv_roller_set_selected(pdata->roller, pdata->value - pdata->metadata->min, LV_ANIM_OFF);
    }
}


static void destroy_page(void *data, void *extra) {
    free(data);
    (void)extra;
}


const pman_page_t page_number_parameter = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
