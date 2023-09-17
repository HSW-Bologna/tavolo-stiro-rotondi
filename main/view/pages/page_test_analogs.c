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
};


struct page_data {
    lv_obj_t *lbl;
};


static void page_update(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *cont, *lbl;

    cont = view_common_create_title(lv_scr_act(), "Collaudo ADC", BACK_BTN_ID, PREV_BTN_ID, NEXT_BTN_ID);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);

    lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl, LV_PCT(90));
    lv_obj_center(lbl);
    pdata->lbl = lbl;

    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    page_update(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_UPDATE:
            page_update(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_OPEN:
            model_set_test(pmodel, 1);
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            model_set_test(pmodel, 0);
                            break;

                        case PREV_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_SWAP;
                            msg.vmsg.page = &page_test_input;
                            break;

                        case NEXT_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_SWAP;
                            msg.vmsg.page = &page_temp_parameter;
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
    lv_label_set_text_fmt(
        pdata->lbl, "Temp. tavolo: %i C [%i]\nTemp. bracciolo: %i C [%i]\n\nLivello 1: %i\nLivello 2: %i",
        model_get_temperatura_tavolo(pmodel), model_get_adc_ptc(pmodel, PTC_TEMP1), model_get_temperatura_bracciolo(pmodel),
        model_get_adc_ptc(pmodel, PTC_TEMP2), model_get_probe_level(pmodel, LIQUID_LEVEL_PROBE_1),
        model_get_probe_level(pmodel, LIQUID_LEVEL_PROBE_2));
}


const pman_page_t page_test_analogs = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
