#ifndef STYLE_H_INCLUDED
#define STYLE_H_INCLUDED

#include "lvgl.h"


LV_FONT_DECLARE(font_montserrat_96)


#define STYLE_FONT_SMALL  (&lv_font_montserrat_16)
#define STYLE_FONT_MEDIUM (&lv_font_montserrat_24)
#define STYLE_FONT_BIG    (&lv_font_montserrat_32)
#define STYLE_FONT_HUGE   (&font_montserrat_96)

#define STYLE_PRIMARY        (lv_color_darken(((lv_color_t)lv_color_make(0x9e, 0xc1, 0xe7)), LV_OPA_30))
#define STYLE_SECONDARY      ((lv_color_t)LV_COLOR_MAKE(0x0, 0x0, 0x0))
//#define STYLE_PRIMARY_DARKER ((lv_color_t)LV_COLOR_MAKE(0x3E, 0x71, 0x87))
#define STYLE_PRIMARY_DARKER ((lv_color_t)LV_COLOR_MAKE(0x38, 0x6A, 0x80))
#define STYLE_LIGHT          ((lv_color_t)LV_COLOR_MAKE(0xff, 0xff, 0xff))
#define STYLE_PRESSED        STYLE_PRIMARY_DARKER

#define STYLE_GREEN ((lv_color_t)lv_color_make(0, 0xFF, 0))
#define STYLE_RED   ((lv_color_t)LV_COLOR_MAKE(0xA0, 0, 0))
#define STYLE_BLACK ((lv_color_t)LV_COLOR_MAKE(0x0, 0x0, 0x0))


extern const lv_style_t style_blanket;
extern const lv_style_t style_transparent_cont;
extern const lv_style_t style_padless_cont;
extern const lv_style_t style_black_border;
extern const lv_style_t style_popup;
extern const lv_style_t style_arc;
extern const lv_style_t style_arc_indicator;
extern const lv_style_t style_arc_knob;
extern const lv_style_t style_speed_slider;
extern const lv_style_t style_speed_slider_knob;
extern const lv_style_t style_speed_slider_pressed;
extern const lv_style_t style_speed_slider_knob_pressed;
extern const lv_style_t style_btn_checked;
extern const lv_style_t style_config_btn;


void style_init(void);

#endif