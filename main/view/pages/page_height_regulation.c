#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "view/intl/intl.h"
#include "view/common.h"
#include "gel/pagemanager/page_manager.h"


LV_IMG_DECLARE(img_door);


enum {
    HEIGHT_REGULATION_ENABLE_BTN_ID,
    USER_HEIGHT_PRESETS_BTN_ID,
    SHAPE_HEIGHT_PRESETS_BTN_ID,
    BACK_BTN_ID,
};

struct page_data {
    lv_obj_t *button_user_height;
    lv_obj_t *button_shape_height;

    lv_obj_t *led;
};


static void update_page(model_t *model, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    lv_obj_t *cont, *btn;

    struct page_data *pdata = args;

    view_common_create_title(lv_scr_act(), "Regolazione altezza", BACK_BTN_ID, -1, -1);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_style_pad_column(cont, 16, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    {
        btn = lv_btn_create(cont);
        lv_obj_set_width(btn, LV_PCT(90));

        lv_obj_t *lbl_name = lv_label_create(btn);
        lv_obj_add_flag(lbl_name, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(lbl_name, LV_ALIGN_LEFT_MID, 0, 0);
        lv_label_set_text(lbl_name, "Abilita");
        lv_obj_set_style_text_font(lbl_name, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);

        lv_obj_t *led = lv_led_create(btn);
        lv_led_set_color(led, lv_color_make(0x00, 0xFF, 0x00));
        lv_obj_add_flag(led, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(led, LV_ALIGN_RIGHT_MID, 0, 0);
        pdata->led = led;

        view_register_object_default_callback(btn, HEIGHT_REGULATION_ENABLE_BTN_ID);
    }

    {
        btn = lv_btn_create(cont);
        lv_obj_set_width(btn, LV_PCT(90));

        lv_obj_t *lbl = lv_label_create(btn);
        lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(lbl, "Altezze utenti");
        lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);

        view_register_object_default_callback(btn, USER_HEIGHT_PRESETS_BTN_ID);

        pdata->button_user_height = btn;
    }

    {
        btn = lv_btn_create(cont);
        lv_obj_set_width(btn, LV_PCT(90));

        lv_obj_t *lbl = lv_label_create(btn);
        lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(lbl, "Altezze forme");
        lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);

        view_register_object_default_callback(btn, SHAPE_HEIGHT_PRESETS_BTN_ID);

        pdata->button_shape_height = btn;
    }

    update_page(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_UPDATE:
            break;

        case VIEW_EVENT_CODE_OPEN:
            model_set_machine_test(pmodel);
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            model_set_machine_on(pmodel);
                            break;

                        case HEIGHT_REGULATION_ENABLE_BTN_ID:
                            pmodel->configuration.height_regulation = !pmodel->configuration.height_regulation;
                            update_page(pmodel, pdata);
                            break;

                        case USER_HEIGHT_PRESETS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_user_height_presets;
                            break;

                        case SHAPE_HEIGHT_PRESETS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_shape_height_presets;
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


static void update_page(model_t *model, struct page_data *pdata) {
    if (model->configuration.height_regulation) {
        lv_obj_clear_state(pdata->button_user_height, LV_STATE_DISABLED);
        lv_obj_clear_state(pdata->button_shape_height, LV_STATE_DISABLED);
        lv_led_set_brightness(pdata->led, LV_LED_BRIGHT_MAX);
        lv_obj_set_style_bg_color(pdata->led, lv_color_make(0x00, 0xFF, 0x00), LV_STATE_DEFAULT);
    } else {
        lv_obj_add_state(pdata->button_user_height, LV_STATE_DISABLED);
        lv_obj_add_state(pdata->button_shape_height, LV_STATE_DISABLED);
        lv_led_set_brightness(pdata->led, LV_LED_BRIGHT_MIN);
        lv_obj_set_style_bg_color(pdata->led, lv_color_make(0x00, 0x80, 0x00), LV_STATE_DEFAULT);
    }
}


const pman_page_t page_height_regulation = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
