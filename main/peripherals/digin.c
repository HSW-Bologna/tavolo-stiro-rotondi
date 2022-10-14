#include "digin.h"
#include "i2c_devices.h"
#include "I2C/i2c_devices/io/TCA9534/tca9534.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "generic_embedded_libs/gel/debounce/debounce.h"


static void digin_timer(TimerHandle_t timer);


static const char *TAG = "Digin";

static SemaphoreHandle_t sem    = NULL;
static debounce_filter_t filter = {0};
static uint8_t           update = 0;


void digin_init(void) {
    tca9534_configure_gpio_port(io_expander_1, 0xFF);

    debounce_filter_init(&filter);

    static StaticSemaphore_t semaphore_buffer;
    sem = xSemaphoreCreateMutexStatic(&semaphore_buffer);

    static StaticTimer_t timer_buffer;
    TimerHandle_t        timer = xTimerCreateStatic(TAG, 50, 1, NULL, digin_timer, &timer_buffer);
    xTimerStart(timer, portMAX_DELAY);

    ESP_LOGI(TAG, "Digin initialized");
}


uint8_t digin_have_changed(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    uint8_t res = update;
    xSemaphoreGive(sem);
    return res;
}


void digin_sync(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    update = 0;
    xSemaphoreGive(sem);
}


uint8_t digin_read(digin_t digin) {
    xSemaphoreTake(sem, portMAX_DELAY);
    uint8_t res = debounce_read(&filter, digin);
    xSemaphoreGive(sem);
    return res;
}


static void digin_timer(TimerHandle_t timer) {
    uint8_t port;
    if (tca9534_read_input_port(io_expander_1, &port)) {
        ESP_LOGW(TAG, "Error reading inputs!");
    } else {
        port = ~port;
        ESP_LOGD(TAG, "0x%02X", port);

        xSemaphoreTake(sem, portMAX_DELAY);
        if (debounce_filter(&filter, port, 4)) {
            update = 1;
        }
        xSemaphoreGive(sem);
    }
}
