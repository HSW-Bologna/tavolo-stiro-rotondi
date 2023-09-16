#include "driver/ledc.h"
#include "hardwareprofile.h"
#include "tft.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "gt911.h"
#include "lvgl_helpers.h"
#include "esp_log.h"
#include "lvgl_tft/disp_spi.h"
#include "lvgl_touch/tp_spi.h"
#include "lvgl_spi_conf.h"
#include "lvgl_i2c/i2c_manager.h"
#include "gt911.h"


#define TFT_BACKLIGHT_SPEED_MODE LEDC_LOW_SPEED_MODE
#define TFT_BACKLIGHT_TIMER      LEDC_TIMER_2
#define TFT_BACKLIGHT_CHANNEL    LEDC_CHANNEL_2


static const char *TAG           = "TFT";
static uint8_t     standby_touch = 0;


void tft_init(void) {
    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,             // resolution of PWM duty
        .freq_hz         = 2000,                         // frequency of PWM signal
        .speed_mode      = TFT_BACKLIGHT_SPEED_MODE,     // timer mode
        .timer_num       = TFT_BACKLIGHT_TIMER,          // timer index
        .clk_cfg         = LEDC_AUTO_CLK,                // Auto select the source clock
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .channel    = TFT_BACKLIGHT_CHANNEL,
        .duty       = 0,
        .gpio_num   = HAP_RETRO,
        .speed_mode = TFT_BACKLIGHT_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_2,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ledc_fade_func_install(0);
    tft_backlight_set(0);

    gpio_wakeup_enable(HAP_IRQ, GPIO_INTR_LOW_LEVEL);

    gpio_config_t reset_config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT(CONFIG_LV_GT911_RESET_IO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&reset_config));
    gpio_set_level(CONFIG_LV_GT911_RESET_IO, 1);

    lvgl_i2c_init(I2C_NUM_0);
}


void tft_reset_touch(void) {
    gpio_set_level(CONFIG_LV_GT911_RESET_IO, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(CONFIG_LV_GT911_RESET_IO, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}


int tft_touch_detected(void) {
    // TODO: Use an interrupt
    return (gpio_get_level(HAP_IRQ) == 0);
}


void tft_backlight_fade(uint8_t value) {
    value        = value > 100 ? 100 : value;
    uint8_t duty = (uint8_t)((value * 0xFF) / 100);
    ledc_set_fade_time_and_start(TFT_BACKLIGHT_SPEED_MODE, TFT_BACKLIGHT_CHANNEL, duty, 1000, LEDC_FADE_NO_WAIT);
}


void tft_backlight_set(uint8_t value) {
    value         = value > 100 ? 100 : value;
    uint32_t duty = (uint32_t)((((uint32_t)value) * 0xFF) / 100);
    ledc_set_duty(TFT_BACKLIGHT_SPEED_MODE, TFT_BACKLIGHT_CHANNEL, duty);
    ledc_update_duty(TFT_BACKLIGHT_SPEED_MODE, TFT_BACKLIGHT_CHANNEL);
}


void tft_standby_touch(uint8_t standby) {
    standby_touch = standby;
}


void tft_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    if (!standby_touch) {
        gt911_read(drv, data);
    }
}


void tft_lvgl_drivers_init(void) {
    /* Display controller initialization */
    ESP_LOGI(TAG, "Initializing SPI master for display");
    lvgl_spi_driver_init(TFT_SPI_HOST, DISP_SPI_MISO, DISP_SPI_MOSI, DISP_SPI_CLK, SPI_BUS_MAX_TRANSFER_SZ, 1,
                         DISP_SPI_IO2, DISP_SPI_IO3);
    disp_spi_add_device(TFT_SPI_HOST);
    disp_driver_init();

    ESP_LOGI(TAG, "Inizializing touch IC");
    size_t counter = 0;
    do {
        tft_reset_touch();
        int res = gt911_init(GT911_I2C_SLAVE_ADDR);
        if (res) {
            ESP_LOGW(TAG, "Error in inizializing touch, retrying...");
            vTaskDelay(pdMS_TO_TICKS(200));
        } else {
            break;
        }
    } while (counter++ < 5);
    if (counter >= 5) {
        ESP_LOGE(TAG, "Unable to inizialize touch!");
    }
}
