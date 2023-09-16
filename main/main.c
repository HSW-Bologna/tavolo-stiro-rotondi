#include <driver/i2c.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "lvgl_helpers.h"
#include "lvgl_i2c/i2c_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "I2C/i2c_ports/esp-idf/esp_idf_i2c_port.h"
#include "peripherals/storage.h"
#include "peripherals/tft.h"
#include "peripherals/system.h"
#include "peripherals/heartbeat.h"
#include "peripherals/rs485.h"
#include "peripherals/buzzer.h"


static const char *TAG = "Main";


void app_main(void) {
    model_t model;

    vTaskDelay(pdMS_TO_TICKS(500));

    system_i2c_init();
    rs485_init();
    buzzer_init();
    tft_init();
    tft_lvgl_drivers_init();
    heartbeat_init(2000UL);

    storage_init();
    tft_backlight_set(60);

    model_init(&model);
    view_init(&model, disp_driver_flush, tft_touch_read);
    controller_init(&model);

    buzzer_beep(2, 250, 250);
    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_gui_manage(&model);
        controller_manage(&model);

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
