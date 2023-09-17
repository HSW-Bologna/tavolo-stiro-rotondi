#ifndef VIEW_COMMON_H_INCLUDED
#define VIEW_COMMON_H_INCLUDED

#include "lvgl.h"
#include "view_types.h"
#include "model/model.h"


typedef struct {
    lv_obj_t *btn;
    lv_obj_t *lbl;
    lv_obj_t *led;
} view_common_led_button_t;


typedef struct {
    const char         *password;
    view_page_message_t msg;
} password_page_options_t;


view_common_led_button_t view_common_create_led_button(lv_obj_t *root, const char *text, int id, int number);
password_page_options_t *view_common_default_password_page_options(view_page_message_t msg, const char *password);
void                     view_common_set_hidden(lv_obj_t *obj, int hidden);
void                     view_common_img_set_src(lv_obj_t *img, const lv_img_dsc_t *dsc);
void                     view_common_set_checked(lv_obj_t *obj, uint8_t checked);
lv_obj_t *view_common_create_title(lv_obj_t *root, const char *text, int back_id, int prev_id, int next_id);


#endif
