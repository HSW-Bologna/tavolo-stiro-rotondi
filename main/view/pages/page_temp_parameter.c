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
    ADC_PLUS_BTN_ID,
    ADC_MINUS_BTN_ID,
    HYSTERESIS_PLUS_BTN_ID,
    HYSTERESIS_MINUS_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_livello;
    lv_obj_t *lbl_isteresi;
};


static void page_update(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *cont, *lbl, *btn;

    cont = view_common_create_title(lv_scr_act(), "Calibrazione caldaia", BACK_BTN_ID, NEXT_BTN_ID);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);

    lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -64);
    pdata->lbl_livello = lbl;

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 64, 64);
    lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    lv_label_set_text(lbl, LV_SYMBOL_MINUS);
    lv_obj_align_to(btn, pdata->lbl_livello, LV_ALIGN_LEFT_MID, -120, 0);
    view_register_object_default_callback(btn, ADC_MINUS_BTN_ID);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 64, 64);
    lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    lv_label_set_text(lbl, LV_SYMBOL_PLUS);
    lv_obj_align_to(btn, pdata->lbl_livello, LV_ALIGN_RIGHT_MID, 120, 0);
    view_register_object_default_callback(btn, ADC_PLUS_BTN_ID);

    lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 64);
    pdata->lbl_isteresi = lbl;

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 64, 64);
    lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    lv_label_set_text(lbl, LV_SYMBOL_MINUS);
    lv_obj_align_to(btn, pdata->lbl_isteresi, LV_ALIGN_LEFT_MID, -120, 0);
    view_register_object_default_callback(btn, HYSTERESIS_MINUS_BTN_ID);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 64, 64);
    lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_obj_center(lbl);
    lv_label_set_text(lbl, LV_SYMBOL_PLUS);
    lv_obj_align_to(btn, pdata->lbl_isteresi, LV_ALIGN_RIGHT_MID, 120, 0);
    view_register_object_default_callback(btn, HYSTERESIS_PLUS_BTN_ID);

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
                case LV_EVENT_LONG_PRESSED_REPEAT:
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            model_set_test(pmodel, 0);
                            break;

                        case NEXT_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_SWAP;
                            msg.vmsg.page = &page_test_output;
                            break;

                        case ADC_MINUS_BTN_ID:
                            if (pmodel->configuration.boiler_adc_threshold > 0) {
                                pmodel->configuration.boiler_adc_threshold -= 10;
                                pmodel->configuration.boiler_adc_threshold -=
                                    pmodel->configuration.boiler_adc_threshold % 10;
                            } else {
                                pmodel->configuration.boiler_adc_threshold = 0;
                            }
                            page_update(pmodel, pdata);
                            break;

                        case ADC_PLUS_BTN_ID:
                            if (pmodel->configuration.boiler_adc_threshold < 4000) {
                                pmodel->configuration.boiler_adc_threshold += 10;
                                pmodel->configuration.boiler_adc_threshold -=
                                    pmodel->configuration.boiler_adc_threshold % 10;
                            } else {
                                pmodel->configuration.boiler_adc_threshold = 4000;
                            }
                            page_update(pmodel, pdata);
                            break;

                        case HYSTERESIS_MINUS_BTN_ID:
                            if (pmodel->configuration.isteresi_caldaia > 0) {
                                pmodel->configuration.isteresi_caldaia--;
                            } else {
                                pmodel->configuration.isteresi_caldaia = 0;
                            }
                            page_update(pmodel, pdata);
                            break;

                        case HYSTERESIS_PLUS_BTN_ID:
                            if (pmodel->configuration.isteresi_caldaia < 300) {
                                pmodel->configuration.isteresi_caldaia++;
                            } else {
                                pmodel->configuration.isteresi_caldaia = 300;
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
    lv_label_set_text_fmt(pdata->lbl_livello, "%4i", model_get_boiler_adc_threshold(pmodel));
    lv_label_set_text_fmt(pdata->lbl_isteresi, "%.1f s", ((float)model_get_isteresi_caldaia(pmodel)) / 10.);
}


const pman_page_t page_temp_parameter = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};