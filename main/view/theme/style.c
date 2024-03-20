#include "lvgl.h"
#include "style.h"


#define LV_STYLE_CONST_NULL                                                                                            \
    {                                                                                                                  \
        LV_STYLE_PROP_INV, {                                                                                           \
            0                                                                                                          \
        }                                                                                                              \
    }


static lv_style_transition_dsc_t transition_dsc;


static const lv_style_const_prop_t style_padless_cont_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(0), LV_STYLE_CONST_PAD_TOP(0),    LV_STYLE_CONST_PAD_LEFT(0), LV_STYLE_CONST_PAD_RIGHT(0),
    LV_STYLE_CONST_PAD_ROW(0),    LV_STYLE_CONST_PAD_COLUMN(0), LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_padless_cont, style_padless_cont_props);


static const lv_style_const_prop_t style_transparent_cont_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(0),
    LV_STYLE_CONST_PAD_TOP(0),
    LV_STYLE_CONST_PAD_LEFT(0),
    LV_STYLE_CONST_PAD_RIGHT(0),
    LV_STYLE_CONST_PAD_ROW(0),
    LV_STYLE_CONST_PAD_COLUMN(0),
    LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_BG_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_transparent_cont, style_transparent_cont_props);


static const lv_style_const_prop_t style_blanket_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(0),
    LV_STYLE_CONST_PAD_TOP(0),
    LV_STYLE_CONST_PAD_LEFT(0),
    LV_STYLE_CONST_PAD_RIGHT(0),
    LV_STYLE_CONST_PAD_ROW(0),
    LV_STYLE_CONST_PAD_COLUMN(0),
    LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_BG_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_BG_COLOR(STYLE_BLACK),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_blanket, style_blanket_props);


static const lv_style_const_prop_t style_black_border_props[] = {
    LV_STYLE_CONST_BORDER_WIDTH(3),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_BLACK),
    LV_STYLE_CONST_RADIUS(16),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_black_border, style_black_border_props);


static const lv_style_const_prop_t style_popup_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(4),   LV_STYLE_CONST_PAD_TOP(4),
    LV_STYLE_CONST_PAD_LEFT(4),     LV_STYLE_CONST_PAD_RIGHT(4),
    LV_STYLE_CONST_BORDER_WIDTH(4), LV_STYLE_CONST_BORDER_COLOR(STYLE_BLACK),
    LV_STYLE_CONST_RADIUS(32),      LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_popup, style_popup_props);


static const lv_style_const_prop_t style_arc_props[] = {
    LV_STYLE_CONST_ARC_WIDTH(24),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_arc, style_arc_props);

static const lv_style_const_prop_t style_arc_indicator_props[] = {
    LV_STYLE_CONST_ARC_WIDTH(24),
    LV_STYLE_CONST_ARC_COLOR(STYLE_RED),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_arc_indicator, style_arc_indicator_props);

static const lv_style_const_prop_t style_arc_knob_props[] = {
    LV_STYLE_CONST_BG_COLOR(STYLE_RED), LV_STYLE_CONST_PAD_BOTTOM(8), LV_STYLE_CONST_PAD_TOP(8),
    LV_STYLE_CONST_PAD_LEFT(16),        LV_STYLE_CONST_PAD_RIGHT(16), LV_STYLE_CONST_BORDER_COLOR(STYLE_BLACK),
    LV_STYLE_CONST_BORDER_WIDTH(4),     LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_arc_knob, style_arc_knob_props);

static const lv_style_const_prop_t style_speed_slider_props[] = {
    LV_STYLE_CONST_TRANSITION(&transition_dsc),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_speed_slider, style_speed_slider_props);

static const lv_style_const_prop_t style_speed_slider_knob_props[] = {
    LV_STYLE_CONST_TRANSITION(&transition_dsc),
    LV_STYLE_CONST_PAD_TOP(16),
    LV_STYLE_CONST_PAD_BOTTOM(16),
    LV_STYLE_CONST_PAD_LEFT(8),
    LV_STYLE_CONST_PAD_RIGHT(8),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_BLACK),
    LV_STYLE_CONST_BORDER_WIDTH(4),
    LV_STYLE_CONST_BG_COLOR(STYLE_PRIMARY_DARKER),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_speed_slider_knob, style_speed_slider_knob_props);

static const lv_style_const_prop_t style_speed_slider_pressed_props[] = {
    LV_STYLE_CONST_BG_COLOR(STYLE_PRESSED),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_speed_slider_pressed, style_speed_slider_pressed_props);

static const lv_style_const_prop_t style_speed_slider_knob_pressed_props[] = {
    LV_STYLE_CONST_PAD_TOP(20),
    LV_STYLE_CONST_PAD_BOTTOM(20),
    LV_STYLE_CONST_PAD_LEFT(12),
    LV_STYLE_CONST_PAD_RIGHT(12),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_BLACK),
    LV_STYLE_CONST_BORDER_WIDTH(4),
    LV_STYLE_CONST_BG_COLOR(STYLE_PRESSED),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_speed_slider_knob_pressed, style_speed_slider_knob_pressed_props);

static const lv_style_const_prop_t style_btn_checked_props[] = {
    LV_STYLE_CONST_BG_COLOR(STYLE_PRESSED),
    LV_STYLE_CONST_BORDER_WIDTH(6),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_btn_checked, style_btn_checked_props);


static const lv_style_const_prop_t style_config_btn_props[] = {
    LV_STYLE_CONST_BG_COLOR(STYLE_PRIMARY_DARKER),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_config_btn, style_config_btn_props);


void style_init(void) {
    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, 0};
    lv_style_transition_dsc_init(&transition_dsc, props, lv_anim_path_linear, 300, 0, NULL);
}
