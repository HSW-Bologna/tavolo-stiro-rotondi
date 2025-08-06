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
    BACK_BTN_ID,
    PARAMETERS_BTN_ID,
    MACHINE_MODEL_BTN_ID,
    HEIGHT_REGULATION_BTN_ID,
    TEST_BTN_ID,
};


static void *create_page(model_t *pmodel, void *extra) {
    return NULL;
}


static void open_page(model_t *pmodel, void *args) {
    lv_obj_t *cont, *lbl, *btn;

    view_common_create_title(lv_scr_act(), "Impostazioni", BACK_BTN_ID, -1, -1);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_style_pad_column(cont, 16, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 180, 80);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Test");
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    view_register_object_default_callback(btn, TEST_BTN_ID);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 180, 80);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Parametri");
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    view_register_object_default_callback(btn, PARAMETERS_BTN_ID);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 180, 80);
    lbl = lv_label_create(btn);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, 170);
    lv_label_set_text(lbl, "Modello macchina");
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    view_register_object_default_callback(btn, MACHINE_MODEL_BTN_ID);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 180, 80);
    lbl = lv_label_create(btn);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, 170);
    lv_label_set_text(lbl, "Regolazione altezza");
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    view_register_object_default_callback(btn, HEIGHT_REGULATION_BTN_ID);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t msg = VIEW_NULL_MESSAGE;

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

                        case TEST_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_test_output;
                            break;

                        case PARAMETERS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_parameters;
                            break;

                        case HEIGHT_REGULATION_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_height_regulation;
                            break;

                        case MACHINE_MODEL_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_REBASE;
                            msg.vmsg.page = &page_machine_model;
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


const pman_page_t page_menu = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
