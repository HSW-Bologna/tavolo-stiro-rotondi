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
    PRESET_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_value[USER_HEIGHT_REGULATION_PRESETS];
    lv_obj_t *buttons[USER_HEIGHT_REGULATION_PRESETS];
};


static void page_update(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *cont;

    cont = view_common_create_title(lv_scr_act(), "Impostazioni altezze utenti", BACK_BTN_ID, -1, -1);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_VER_RES - 64);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for (size_t i = 0; i < USER_HEIGHT_REGULATION_PRESETS; i++) {
        lv_obj_t *obj = lv_obj_create(cont);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_style(obj, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_hor(obj, 8, LV_STATE_DEFAULT);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_set_width(obj, LV_PCT(100));

        {
            lv_obj_t *button = view_common_base_button_create(obj, PRESET_BTN_ID);
            lv_obj_set_size(button, 150, 48);
            view_register_object_default_callback_with_number(button, PRESET_BTN_ID, i);
            lv_obj_center(button);
            pdata->buttons[i] = button;

            lv_obj_t *label = lv_label_create(button);
            lv_obj_set_style_text_font(label, STYLE_FONT_BIG, LV_STATE_DEFAULT);
            lv_obj_center(label);
            pdata->lbl_value[i] = label;
        }

        {
            lv_obj_t *button = lv_btn_create(obj);
            lv_obj_set_size(button, 80, 48);
            lv_obj_t *label = lv_label_create(button);
            lv_label_set_text(label, LV_SYMBOL_MINUS);
            lv_obj_set_style_text_font(label, STYLE_FONT_BIG, LV_STATE_DEFAULT);
            lv_obj_center(label);
            view_register_object_default_callback_with_number(button, MINUS_BTN_ID, i);
            lv_obj_align(button, LV_ALIGN_LEFT_MID, 0, 0);
        }

        {
            lv_obj_t *button = lv_btn_create(obj);
            lv_obj_set_size(button, 80, 48);
            lv_obj_t *label = lv_label_create(button);
            lv_label_set_text(label, LV_SYMBOL_PLUS);
            lv_obj_set_style_text_font(label, STYLE_FONT_BIG, LV_STATE_DEFAULT);
            lv_obj_center(label);
            view_register_object_default_callback_with_number(button, PLUS_BTN_ID, i);
            lv_obj_align(button, LV_ALIGN_RIGHT_MID, 0, 0);
        }
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
                case LV_EVENT_LONG_PRESSED_REPEAT:
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case PRESET_BTN_ID: {
                            pmodel->configuration.selected_user_height_preset = event.data.number;
                            page_update(pmodel, pdata);
                            break;
                        }

                        case MINUS_BTN_ID: {
                            uint16_t min = 0;

                            if (pmodel->configuration.user_height_presets[event.data.number] > min) {
                                pmodel->configuration.user_height_presets[event.data.number]--;
                            }

                            pmodel->configuration.selected_user_height_preset = event.data.number;
                            page_update(pmodel, pdata);
                            break;
                        }

                        case PLUS_BTN_ID: {
                            uint16_t max = 100;

                            if (pmodel->configuration.user_height_presets[event.data.number] < max) {
                                pmodel->configuration.user_height_presets[event.data.number]++;
                            }

                            pmodel->configuration.selected_user_height_preset = event.data.number;
                            page_update(pmodel, pdata);
                            break;
                        }
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
    for (size_t i = 0; i < USER_HEIGHT_REGULATION_PRESETS; i++) {
        lv_label_set_text_fmt(pdata->lbl_value[i], "%zu: %3i", i + 1, pmodel->configuration.user_height_presets[i]);
        view_common_set_checked(pdata->buttons[i], i == pmodel->configuration.selected_user_height_preset);
    }
}


static void destroy_page(void *data, void *extra) {
    free(data);
    (void)extra;
}


const pman_page_t page_user_height_presets = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
