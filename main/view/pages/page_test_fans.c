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
    PREV_BTN_ID,
    NEXT_BTN_ID,
    SLIDER_SUCTION_ID,
    SLIDER_BLOW_ID,
};


struct page_data {
    lv_obj_t *lbl_suction;
    lv_obj_t *lbl_blow;

    lv_obj_t *slider_suction;
    lv_obj_t *slider_blow;
};


static void page_update(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *cont, *slider, *lbl;

    cont = view_common_create_title(lv_scr_act(), "Collaudo ventole", BACK_BTN_ID, PREV_BTN_ID, NEXT_BTN_ID);

    slider = lv_slider_create(lv_scr_act());
    lv_slider_set_range(slider, 0, 100);
    lv_obj_set_size(slider, 400, 32);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 148);
    view_register_object_default_callback(slider, SLIDER_SUCTION_ID);
    pdata->slider_suction = slider;

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align_to(lbl, slider, LV_ALIGN_OUT_TOP_MID, 0, -16);
    pdata->lbl_suction = lbl;

    slider = lv_slider_create(lv_scr_act());
    lv_slider_set_range(slider, 0, 100);
    lv_obj_set_size(slider, 400, 32);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 256);
    view_register_object_default_callback(slider, SLIDER_BLOW_ID);
    pdata->slider_blow = slider;

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align_to(lbl, slider, LV_ALIGN_OUT_TOP_MID, 0, -16);
    pdata->lbl_blow = lbl;

    page_update(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_OPEN:
            model_set_test(pmodel, 1);
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case PREV_BTN_ID:
                            model_set_test_percentage_suction(pmodel, 0);
                            model_set_test_percentage_blow(pmodel, 0);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_SWAP;
                            msg.vmsg.page = &page_temp_parameter;
                            break;

                        case NEXT_BTN_ID:
                            model_set_test_percentage_suction(pmodel, 0);
                            model_set_test_percentage_blow(pmodel, 0);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_SWAP;
                            msg.vmsg.page = &page_test_output;
                            break;

                        case BACK_BTN_ID:
                            model_set_test(pmodel, 0);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case SLIDER_SUCTION_ID:
                            model_set_test_percentage_suction(pmodel, lv_slider_get_value(pdata->slider_suction));
                            page_update(pmodel, pdata);
                            break;
                        case SLIDER_BLOW_ID:
                            model_set_test_percentage_blow(pmodel, lv_slider_get_value(pdata->slider_blow));
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
    lv_slider_set_value(pdata->slider_suction, model_get_test_percentage_suction(pmodel), LV_ANIM_OFF);
    lv_label_set_text_fmt(pdata->lbl_suction, "Aspirazione: %i%%", model_get_test_percentage_suction(pmodel));
    lv_obj_align_to(pdata->lbl_suction, pdata->slider_suction, LV_ALIGN_OUT_TOP_MID, 0, -16);

    lv_slider_set_value(pdata->slider_blow, model_get_test_percentage_blow(pmodel), LV_ANIM_OFF);
    lv_label_set_text_fmt(pdata->lbl_blow, "Soffio: %i%%", model_get_test_percentage_blow(pmodel));
    lv_obj_align_to(pdata->lbl_blow, pdata->slider_blow, LV_ALIGN_OUT_TOP_MID, 0, -16);
}


const pman_page_t page_test_fans = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
