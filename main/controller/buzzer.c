#include "buzzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "peripherals/digout.h"


static void buzzer_periodic(TimerHandle_t timer);
static void buzzer_on(void);
static void buzzer_off(void);


static unsigned long time_on  = 0;
static unsigned long time_off = 0;
static size_t        repeat   = 0;
static int           is_set   = 0;
static int           is_on    = 0;
static TimerHandle_t timer;
static const char   *TAG = "Buzzer";


void buzzer_init(void) {
    static StaticTimer_t timer_buffer;
    timer = xTimerCreateStatic(TAG, pdMS_TO_TICKS(10), pdTRUE, NULL, buzzer_periodic, &timer_buffer);
}


void buzzer_beep(uint16_t r, unsigned long t_on, unsigned long t_off) {
    xTimerStop(timer, portMAX_DELAY);
    xTimerChangePeriod(timer, pdMS_TO_TICKS(t_on), portMAX_DELAY);

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
    DIGOUT_SET(DIGOUT_BUZZER);
}


static void buzzer_off(void) {
    DIGOUT_CLEAR(DIGOUT_BUZZER);
}
