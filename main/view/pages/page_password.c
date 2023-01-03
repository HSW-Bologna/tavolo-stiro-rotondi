#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "view/common.h"
#include "view/intl/intl.h"
#include "gel/pagemanager/page_manager.h"
#include "config/app_config.h"
#include "utils/utils.h"
#include "gel/timer/timecheck.h"


LV_IMG_DECLARE(img_lock);


enum {
    BACK_BTN_ID,
    PASSWORD_TA_ID,
    PASSWORD_KB_ID,
};


struct page_data {
    lv_timer_t              *timer;
    password_page_options_t *fence;

    int           valid;
    unsigned long lockts;

    lv_obj_t *textarea;
};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);
    pdata->timer = view_register_periodic_timer(10000UL, 0);
    pdata->fence = extra;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_timer_resume(pdata->timer);
    pdata->valid  = 0;
    pdata->lockts = 0;

    view_common_create_title(lv_scr_act(), "Password", BACK_BTN_ID, -1);

    static const char *kbmap[] = {
        "1", "2", "3", "4", "5", "\n", "6", "7", "8", "9", "0", "\n", LV_SYMBOL_BACKSPACE, LV_SYMBOL_OK, "",
    };
    static const lv_btnmatrix_ctrl_t ctrl_map[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);
    lv_obj_add_style(cont, (lv_style_t *)&style_padless_cont, LV_STATE_DEFAULT);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t *ta = lv_textarea_create(cont);
    lv_textarea_set_one_line(ta, 1);
    lv_textarea_set_password_mode(ta, 1);
    lv_textarea_set_text(ta, "");
    lv_textarea_set_max_length(ta, strlen(pdata->fence->password));
    lv_obj_set_width(ta, LV_PCT(50));
    lv_obj_clear_flag(ta, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_font(ta, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ta, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_add_state(ta, LV_STATE_FOCUSED);
    pdata->textarea = ta;

    view_register_object_default_callback(ta, PASSWORD_TA_ID);

    lv_obj_t *kb = lv_keyboard_create(cont);
    lv_keyboard_set_textarea(kb, ta);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_height(kb, 180);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(kb, (lv_style_t *)&style_config_btn, LV_PART_ITEMS);
    lv_obj_set_style_text_font(kb, STYLE_FONT_BIG, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(kb, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_NUMBER, kbmap, ctrl_map);
    view_register_object_default_callback(kb, PASSWORD_KB_ID);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
            break;

        case VIEW_EVENT_CODE_LVGL:
            if (event.event == LV_EVENT_CLICKED) {
                lv_timer_reset(pdata->timer);

                switch (event.data.id) {
                    case BACK_BTN_ID:
                        msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                        break;
                }
            } else if (event.event == LV_EVENT_PRESSED) {
                lv_timer_reset(pdata->timer);
            } else if (event.event == LV_EVENT_LONG_PRESSED_REPEAT) {
                lv_timer_reset(pdata->timer);
            } else if (event.event == LV_EVENT_READY) {
                lv_timer_reset(pdata->timer);

                if (strcmp(event.string_value, pdata->fence->password) == 0) {
                    pdata->valid = 1;
                } else {
                    lv_obj_set_style_border_color(pdata->textarea, STYLE_RED, LV_STATE_DEFAULT);
                    lv_textarea_set_text(pdata->textarea, "");
                }
            } else if (event.event == LV_EVENT_RELEASED) {
                lv_timer_reset(pdata->timer);

                switch (event.data.id) {
                    case PASSWORD_KB_ID:
                        if (pdata->valid) {
                            msg.vmsg = pdata->fence->msg;
                        }
                        break;
                }
            }
            break;

        default:
            break;
    }

    return msg;
}


static void close_page(void *args) {
    struct page_data *pdata = args;
    lv_timer_pause(pdata->timer);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *args, void *extra) {
    struct page_data *pdata = args;
    lv_timer_del(pdata->timer);
    lv_mem_free(pdata);
    lv_mem_free(extra);
}


const pman_page_t page_password = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};