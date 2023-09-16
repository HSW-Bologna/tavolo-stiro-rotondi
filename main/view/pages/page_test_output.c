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
    OUTPUT_BTN_ID,
    BACK_BTN_ID,
    NEXT_BTN_ID,
};


struct page_data {
    view_common_led_button_t led_buttons[NUM_OUTPUTS];
    uint8_t                  output_map;
};


static void page_update(model_t *pmodel, struct page_data *pdata);


/*static const char *descriptions[NUM_OUTPUTS] = {
    "Pompa vapore", "Risc. vapore", "Piano", "Bracc.", "Ferro 1", "Ferro 2", "Luce",
};*/


static const digout_t digout_codes[NUM_OUTPUTS] = {
    DIGOUT_POMPA,
    DIGOUT_RISCALDAMENTO_VAPORE,
    DIGOUT_RISCALDAMENTO_PIANO,
    DIGOUT_RISCALDAMENTO_BRACCIOLO,
    DIGOUT_RISCALDAMENTO_FERRO_1,
    DIGOUT_RISCALDAMENTO_FERRO_2,
    DIGOUT_LUCE,
};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *cont;

    pdata->output_map = 0;

    cont = view_common_create_title(lv_scr_act(), "Collaudo uscite", BACK_BTN_ID, NEXT_BTN_ID);

    static lv_coord_t col_dsc[] = {150, 150, 150, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {60, 60, 60, LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 64);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_hor(cont, 8, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cont, 16, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cont, 5, LV_STATE_DEFAULT);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    pdata->led_buttons[0] = view_common_create_led_button(cont, "Pompa vapore", OUTPUT_BTN_ID, 0);
    lv_obj_set_width(pdata->led_buttons[0].btn, 300);
    lv_obj_set_grid_cell(pdata->led_buttons[0].btn, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);

    pdata->led_buttons[1] = view_common_create_led_button(cont, "Riscaldamento vapore", OUTPUT_BTN_ID, 1);
    lv_obj_set_width(pdata->led_buttons[1].btn, 300);
    lv_obj_set_grid_cell(pdata->led_buttons[1].btn, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);

    pdata->led_buttons[2] = view_common_create_led_button(cont, "Piano", OUTPUT_BTN_ID, 2);
    lv_obj_set_grid_cell(pdata->led_buttons[2].btn, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    pdata->led_buttons[3] = view_common_create_led_button(cont, "Bracc.", OUTPUT_BTN_ID, 3);
    lv_obj_set_grid_cell(pdata->led_buttons[3].btn, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    pdata->led_buttons[4] = view_common_create_led_button(cont, "Ferro 1", OUTPUT_BTN_ID, 4);
    lv_obj_set_grid_cell(pdata->led_buttons[4].btn, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    pdata->led_buttons[5] = view_common_create_led_button(cont, "Ferro 2", OUTPUT_BTN_ID, 5);
    lv_obj_set_grid_cell(pdata->led_buttons[5].btn, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    pdata->led_buttons[6] = view_common_create_led_button(cont, "Luce", OUTPUT_BTN_ID, 6);
    lv_obj_set_grid_cell(pdata->led_buttons[6].btn, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);

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
                        case OUTPUT_BTN_ID:
                            msg.cmsg.code          = VIEW_CONTROLLER_MESSAGE_CODE_TEST_OUTPUT;
                            msg.cmsg.digout.number = digout_codes[event.data.number];

                            if ((pdata->output_map & (1 << event.data.number)) > 0) {
                                pdata->output_map &= ~(1 << event.data.number);
                                msg.cmsg.digout.value = 0;
                            } else {
                                pdata->output_map |= 1 << event.data.number;
                                msg.cmsg.digout.value = 1;
                            }

                            page_update(pmodel, pdata);
                            break;

                        case NEXT_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_SWAP;
                            msg.vmsg.page = &page_test_input;
                            break;

                        case BACK_BTN_ID:
                            model_set_test(pmodel, 0);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
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
    for (size_t i = 0; i < NUM_OUTPUTS; i++) {
        if ((pdata->output_map & (1 << i)) > 0) {
            lv_led_on(pdata->led_buttons[i].led);
        } else {
            lv_led_off(pdata->led_buttons[i].led);
        }
    }
}


const pman_page_t page_test_output = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};
