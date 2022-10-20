#include <stdint.h>
#include "hardwareprofile.h"
#include "liquid_level.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"


#define NUM_SAMPLES 20


static void level_timer(TimerHandle_t timer);


static const char *TAG = "Liquid level";

static SemaphoreHandle_t             sem                         = NULL;
static uint16_t                      level1_average[NUM_SAMPLES] = {0};
static uint16_t                      level2_average[NUM_SAMPLES] = {0};
static size_t                        level_index                 = 0;
static uint8_t                       first_loop                  = 1;
static esp_adc_cal_characteristics_t adc1_chars;
static esp_adc_cal_characteristics_t adc2_chars;


void liquid_level_init(void) {
    // ADC1 config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(HAP_TEMP2, ADC_ATTEN_DB_11));
    // ADC2 config
    ESP_ERROR_CHECK(adc2_config_channel_atten(HAP_TEMP1, ADC_ATTEN_DB_11));

    static StaticSemaphore_t semaphore_buffer;
    sem = xSemaphoreCreateMutexStatic(&semaphore_buffer);

    static StaticTimer_t timer_buffer;
    TimerHandle_t        timer = xTimerCreateStatic(TAG, pdMS_TO_TICKS(50), 1, NULL, level_timer, &timer_buffer);
    xTimerStart(timer, portMAX_DELAY);
}


static void level_timer(TimerHandle_t timer) {
    uint16_t level1 = adc1_get_raw(HAP_LIVELLO1_H20);
    // ESP_LOGI(TAG, "raw  data 2: %d", level1);
    uint16_t level2 = adc1_get_raw(HAP_LIVELLO2_H20);
    // ESP_LOGI(TAG, "raw  data 2: %d", level2);

    xSemaphoreTake(sem, portMAX_DELAY);
    level1_average[level_index] = level1;
    level2_average[level_index] = level2;

    if (level_index == NUM_SAMPLES - 1) {
        level_index = 0;
        first_loop  = 0;
    } else {
        level_index++;
    }
    xSemaphoreGive(sem);
}


uint16_t liquid_level_get_adc_value(liquid_level_probe_t level) {
    xSemaphoreTake(sem, portMAX_DELAY);
    uint16_t *level_averages[] = {level1_average, level2_average};

    uint16_t *levels = level_averages[level];

    unsigned long level_sum    = 0;
    unsigned long temp         = 0;
    size_t        num_readings = first_loop ? level_index : NUM_SAMPLES;
    for (size_t i = 0; i < num_readings; i++) {
        level_sum += levels[i];
    }
    xSemaphoreGive(sem);
    if (num_readings == 0) {
        return 0;
    } else {
        temp = level_sum / num_readings;
        return (uint16_t)temp;
    }
}