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
    TEST_BTN_ID,
};


static void *create_page(model_t *pmodel, void *extra) {
    return NULL;
}


static void open_page(model_t *pmodel, void *args) {
    lv_obj_t         *cont, *lbl, *btn, *img;

    cont = lv_obj_create(lv_scr_act());
    lv_obj_add_style(cont, (lv_style_t *)&style_padless_cont, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);

    btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 52, 52);
    img = lv_img_create(btn);
    lv_img_set_src(img, &img_door);
    lv_obj_center(img);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 4, 4);
    view_register_object_default_callback(btn, BACK_BTN_ID);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 240, 100);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Test");
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    view_register_object_default_callback(btn, TEST_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, -64);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 240, 100);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Parametri");
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    view_register_object_default_callback(btn, PARAMETERS_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 64);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;

    switch (event.code) {
        case VIEW_EVENT_CODE_UPDATE:
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

                        case TEST_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_test_output;
                            break;

                        case PARAMETERS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_parameters;
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
