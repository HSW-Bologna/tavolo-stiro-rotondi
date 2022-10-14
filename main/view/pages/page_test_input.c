#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "view/intl/intl.h"
#include "view/common.h"
#include "gel/pagemanager/page_manager.h"
#include "peripherals/digout.h"


enum {
    BACK_BTN_ID,
    NEXT_BTN_ID,
};


struct page_data {
    view_common_led_button_t led_buttons[NUM_INPUTS];
};


static void page_update(model_t *pmodel, struct page_data *pdata);


static const char *descriptions[NUM_INPUTS] = {
    "IN1", "IN2", "IN3", "IN4", "Vapore",
};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *cont;

    cont = view_common_create_title(lv_scr_act(), "Collaudo ingressi", BACK_BTN_ID, NEXT_BTN_ID);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    for (size_t i = 0; i < NUM_INPUTS; i++) {
        pdata->led_buttons[i] = view_common_create_led_button(cont, descriptions[i], 0, i);
        lv_obj_clear_flag(pdata->led_buttons[i].btn, LV_OBJ_FLAG_CLICKABLE);
    }

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

                        case NEXT_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_SWAP;
                            msg.vmsg.page = &page_test_analogs;
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
    for (size_t i = 0; i < NUM_INPUTS; i++) {
        if (pmodel->test.inputs[i]) {
            lv_led_on(pdata->led_buttons[i].led);
        } else {
            lv_led_off(pdata->led_buttons[i].led);
        }
    }
}


const pman_page_t page_test_input = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};