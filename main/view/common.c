#include <string.h>
#include "lvgl.h"
#include "common.h"
#include "view.h"
#include "theme/style.h"


LV_IMG_DECLARE(img_door);


view_common_led_button_t view_common_create_led_button(lv_obj_t *root, const char *text, int id, int number) {
    lv_obj_t                *btn, *lbl, *led;
    view_common_led_button_t led_button = {0};

    btn = lv_btn_create(root);
    lv_obj_add_style(btn, (lv_style_t *)&style_padless_cont, LV_STATE_DEFAULT);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 145, 65);
    view_register_object_default_callback_with_number(btn, id, number);

    lbl = lv_label_create(btn);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, LV_PCT(80));
    lv_label_set_text(lbl, text);
    lv_obj_align(lbl, LV_ALIGN_RIGHT_MID, -4, 0);

    led = lv_led_create(btn);
    lv_led_set_color(led, STYLE_GREEN);
    lv_obj_align(led, LV_ALIGN_LEFT_MID, 4, 0);

    lv_obj_add_flag(led, LV_OBJ_FLAG_EVENT_BUBBLE);

    led_button.btn = btn;
    led_button.led = led;
    led_button.lbl = lbl;

    return led_button;
}


password_page_options_t *view_common_default_password_page_options(view_page_message_t msg, const char *password) {
    password_page_options_t *fence = (password_page_options_t *)lv_mem_alloc(sizeof(password_page_options_t));
    assert(fence != NULL);
    fence->password = password;
    fence->msg      = msg;
    return fence;
}


lv_obj_t *view_common_create_title(lv_obj_t *root, const char *text, int back_id, int next_id) {
    lv_obj_t *btn, *lbl, *cont, *img;

    cont = lv_obj_create(root);
    lv_obj_add_style(cont, (lv_style_t *)&style_padless_cont, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, LV_HOR_RES, 64);

    btn = lv_btn_create(cont);
    lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 52, 52);
    img = lv_img_create(btn);
    lv_img_set_src(img, &img_door);
    lv_obj_center(img);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 4, 0);
    view_register_object_default_callback(btn, back_id);

    if (next_id >= 0) {
        btn = lv_btn_create(cont);
        lv_obj_add_style(btn, (lv_style_t *)&style_config_btn, LV_STATE_DEFAULT);
        lv_obj_set_size(btn, 52, 52);
        lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
        lv_label_set_text(lbl, LV_SYMBOL_RIGHT);
        lv_obj_center(lbl);
        lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -4, 0);
        view_register_object_default_callback(btn, next_id);
    }

    lbl = lv_label_create(cont);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, LV_HOR_RES - 128);
    lv_label_set_text(lbl, text);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);

    return cont;
}



void view_common_set_hidden(lv_obj_t *obj, int hidden) {
    if (((obj->flags & LV_OBJ_FLAG_HIDDEN) == 0) && hidden) {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else if (((obj->flags & LV_OBJ_FLAG_HIDDEN) > 0) && !hidden) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}


void view_common_set_checked(lv_obj_t *obj, uint8_t checked) {
    if (((lv_obj_get_state(obj) & LV_STATE_CHECKED) == 0) && checked) {
        lv_obj_add_state(obj, LV_STATE_CHECKED);
    } else if (((lv_obj_get_state(obj) & LV_STATE_CHECKED) > 0) && !checked) {
        lv_obj_clear_state(obj, LV_STATE_CHECKED);
    }
}


void view_common_img_set_src(lv_obj_t *img, const lv_img_dsc_t *dsc) {
    if (lv_img_get_src(img) != dsc) {
        lv_img_set_src(img, dsc);
    }
}
