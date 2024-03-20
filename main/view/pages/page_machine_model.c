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
    MACHINE_MODEL_BTN_ID,
};


static lv_obj_t *btn_machine_model_create(lv_obj_t *parent, const char *name, int number);


static void *create_page(model_t *pmodel, void *extra) {
    return NULL;
}


static void open_page(model_t *pmodel, void *args) {
    lv_obj_t *cont, *lbl, *btn, *img;

    cont = lv_obj_create(lv_scr_act());
    lv_obj_add_style(cont, (lv_style_t *)&style_padless_cont, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);

    lbl = lv_label_create(cont);
    lv_label_set_text(lbl, "Selezionare un modello di macchina:");
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t *flex = lv_obj_create(cont);
    lv_obj_add_style(flex, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_set_size(flex, LV_PCT(100), LV_VER_RES - 64);
    lv_obj_set_layout(flex, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(flex, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(flex, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_style(flex, (lv_style_t *)&style_padless_cont, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(flex, 24, LV_STATE_DEFAULT);
    lv_obj_align(flex, LV_ALIGN_BOTTOM_MID, 0, 0);

    const char *names[] = {"388/398/399", "400", "2000", "2500", "3000", "3300"};
    for (size_t i = 0; i < NUM_MACHINE_MODELS; i++) {
        lv_obj_t *btn = btn_machine_model_create(flex, names[i], i);
        view_common_set_checked(btn, pmodel->configuration.machine_model == i);
    }
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

                        case MACHINE_MODEL_BTN_ID:
                            model_set_machine_model(pmodel, event.data.number);

                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_REBASE;
                            msg.vmsg.page = &page_main;
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


static lv_obj_t *btn_machine_model_create(lv_obj_t *parent, const char *name, int number) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 170, 60);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_style(btn, (lv_style_t *)&style_black_border, LV_STATE_DEFAULT);
    lv_obj_add_style(btn, (lv_style_t *)&style_btn_checked, LV_STATE_CHECKED);

    lv_obj_t *lbl_name = lv_label_create(btn);
    lv_obj_add_flag(lbl_name, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_align(lbl_name, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lbl_name, name);
    lv_obj_set_style_text_font(lbl_name, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);

    view_register_object_default_callback_with_number(btn, MACHINE_MODEL_BTN_ID, number);

    return btn;
}



const pman_page_t page_machine_model = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
