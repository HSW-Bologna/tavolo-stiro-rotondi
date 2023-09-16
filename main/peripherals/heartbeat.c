#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "hardwareprofile.h"
#include "heartbeat.h"


static void heartbeat_print_heap_status(void);


static const char   *TAG       = "Heartbeat";
static TimerHandle_t timer     = NULL;
static unsigned long hb_period = 1000UL;

/**
 * Timer di attivita'. Accende e spegne il led di attivita'
 */
static void heartbeat_timer(TimerHandle_t timer) {
    (void)timer;
    static int    blink   = 0;
    static size_t counter = 0;

    gpio_set_level(HAP_HEARTBIT, blink);
    blink = !blink;

    if (counter++ >= 5) {
        // heartbeat_print_heap_status();
        counter = 0;
    }

    xTimerChangePeriod(timer, pdMS_TO_TICKS(blink ? hb_period : 50UL), portMAX_DELAY);
}


void heartbeat_init(size_t period_ms) {
    gpio_config_t config = {
        .mode         = GPIO_MODE_OUTPUT,
        .intr_type    = GPIO_INTR_DISABLE,
        .pin_bit_mask = BIT64(HAP_HEARTBIT),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));

    hb_period = period_ms;
    timer     = xTimerCreate(TAG, pdMS_TO_TICKS(hb_period), pdTRUE, NULL, heartbeat_timer);
    xTimerStart(timer, portMAX_DELAY);
    heartbeat_print_heap_status();
}


void heartbeat_stop(void) {
    xTimerStop(timer, portMAX_DELAY);
}


void heartbeat_resume(void) {
    xTimerStart(timer, portMAX_DELAY);
}


static void heartbeat_print_heap_status(void) {
    printf("[%s] - Internal RAM: LWM = %u, free = %u, biggest = %u\n", TAG,
           heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
           heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    printf("[%s] - PSRAM       : LWM = %u, free = %u, biggest = %u\n", TAG,
           heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM), heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
           heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}
