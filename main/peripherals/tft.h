#ifndef TFT_BACKLIGHT_H_INCLUDED
#define TFT_BACKLIGHT_H_INCLUDED


#include <stdint.h>
#include "lvgl.h"


void tft_init(void);
void tft_backlight_fade(uint8_t value);
void tft_backlight_set(uint8_t value);
int  tft_touch_detected(void);
void tft_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
void tft_reset_touch(void);
void tft_standby_touch(uint8_t standby);
void tft_lvgl_drivers_init(void);


#endif