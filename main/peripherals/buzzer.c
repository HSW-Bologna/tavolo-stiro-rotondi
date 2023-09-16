#include "hardwareprofile.h"
#include "buzzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"


static void buzzer_periodic(TimerHandle_t timer);
static void buzzer_on(void);
static void buzzer_off(void);


static unsigned long  time_on   = 0;
static unsigned long  time_off  = 0;
static size_t         repeat    = 0;
static int            is_set    = 0;
static int            is_on     = 0;
static TimerHandle_t  timer;
static const char    *TAG = "Buzzer";



void buzzer_init(void) {
    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(HAP_BUZ),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
    gpio_set_level(HAP_BUZ, 0);

    static StaticTimer_t timer_buffer;
    timer = xTimerCreateStatic(TAG, pdMS_TO_TICKS(10), pdTRUE, NULL, buzzer_periodic, &timer_buffer);
}


void buzzer_beep(uint16_t r, unsigned long t_on, unsigned long t_off) {
    xTimerStop(timer, portMAX_DELAY);
    xTimerChangePeriod(timer, pdMS_TO_TICKS(t_on), portMAX_DELAY);

    // No tone, no buzzer
    repeat   = r;
    time_on  = t_on;
    time_off = t_off;
    is_set   = 1;

    buzzer_on();
    xTimerStart(timer, portMAX_DELAY);
}


static void buzzer_periodic(TimerHandle_t timer) {
    if (is_set && repeat > 0) {
        if (is_on) {
            buzzer_off();
            if (time_off > 0) {
                xTimerChangePeriod(timer, pdMS_TO_TICKS(time_off), portMAX_DELAY);
                is_on = 0;
            } else {
                xTimerChangePeriod(timer, pdMS_TO_TICKS(time_on), portMAX_DELAY);
                is_on = 1;
            }
            xTimerReset(timer, portMAX_DELAY);
            repeat--;
        } else {
            buzzer_on();
            is_on = 1;
            xTimerChangePeriod(timer, pdMS_TO_TICKS(time_on), portMAX_DELAY);
            xTimerReset(timer, portMAX_DELAY);
        }
    }

    if (is_set && repeat == 0) {
        is_set = 0;
        buzzer_off();
        xTimerStop(timer, portMAX_DELAY);
    }
}


static void buzzer_on(void) {
    gpio_set_level(HAP_BUZ, 1);
}


static void buzzer_off(void) {
    gpio_set_level(HAP_BUZ, 0);
}
