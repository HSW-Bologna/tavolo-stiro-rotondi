#include <stdint.h>
#include "hardwareprofile.h"
#include "ptc.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "esp_err.h"


#define MINIMUM_AD_VALUE   1061L
#define MAXIMUM_AD_VALUE   2049L
#define MINIMUM_TEMP_VALUE -10L
#define MAXIMUM_TEMP_VALUE 140L

#define COEFF_M_TIMES(x) ((x * (MAXIMUM_TEMP_VALUE - MINIMUM_TEMP_VALUE)) / (MAXIMUM_AD_VALUE - MINIMUM_AD_VALUE))
#define COEFF_Q                                                                                                        \
    (-MINIMUM_AD_VALUE * (MAXIMUM_TEMP_VALUE - MINIMUM_TEMP_VALUE) / (MAXIMUM_AD_VALUE - MINIMUM_AD_VALUE) +           \
     MINIMUM_TEMP_VALUE)

#define NUM_SAMPLES 10


static void ptc_timer(TimerHandle_t timer);


static const char *TAG = "PTC";

static SemaphoreHandle_t             sem                               = NULL;
static uint16_t                      temperature1_average[NUM_SAMPLES] = {0};
static uint16_t                      temperature2_average[NUM_SAMPLES] = {0};
static size_t                        temperature_index                 = 0;
static uint8_t                       first_loop                        = 1;
static esp_adc_cal_characteristics_t adc1_chars;
static esp_adc_cal_characteristics_t adc2_chars;


void ptc_init(void) {
    // ADC1 config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(HAP_TEMP2, ADC_ATTEN_DB_11));
    // ADC2 config
    ESP_ERROR_CHECK(adc2_config_channel_atten(HAP_TEMP1, ADC_ATTEN_DB_11));

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_11_BIT,
        .freq_hz         = 1000,     // Set output frequency at 1 kHz
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = HAP_PWM_LIVELLI,
        .duty       = 1023,     // Set duty to 50%
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    static StaticSemaphore_t semaphore_buffer;
    sem = xSemaphoreCreateMutexStatic(&semaphore_buffer);

    static StaticTimer_t timer_buffer;
    TimerHandle_t        timer = xTimerCreateStatic(TAG, pdMS_TO_TICKS(100), 1, NULL, ptc_timer, &timer_buffer);
    xTimerStart(timer, portMAX_DELAY);
}


static void ptc_timer(TimerHandle_t timer) {
    xSemaphoreTake(sem, portMAX_DELAY);

    uint16_t temperature2                   = adc1_get_raw(HAP_TEMP2);
    temperature2_average[temperature_index] = temperature2;

    int temperature1 = 0;
    ESP_ERROR_CHECK(adc2_get_raw(HAP_TEMP1, ADC_WIDTH_BIT_DEFAULT, &temperature1));
    temperature1_average[temperature_index] = temperature1;

    if (temperature_index == NUM_SAMPLES - 1) {
        temperature_index = 0;
        first_loop        = 0;
    } else {
        temperature_index++;
    }

    xSemaphoreGive(sem);

    ESP_LOGD(TAG, "raw data: %5d\t%5d", temperature1, temperature2);
}


uint16_t ptc_get_adc_value(ptc_t ptc) {
    xSemaphoreTake(sem, portMAX_DELAY);
    uint16_t *temperature_averages[] = {temperature1_average, temperature2_average};

    uint16_t *temperatures = temperature_averages[ptc];

    unsigned long temperature_sum = 0;
    unsigned long temp            = 0;
    size_t        num_readings    = first_loop ? temperature_index : NUM_SAMPLES;
    for (size_t i = 0; i < num_readings; i++) {
        temperature_sum += temperatures[i];
    }
    xSemaphoreGive(sem);

    if (num_readings == 0) {
        return 0;
    } else {
        temp = temperature_sum / num_readings;
        return (uint16_t)temp;
    }
}


int ptc_get_temperature(ptc_t ptc) {
    uint16_t temp = ptc_get_adc_value(ptc);
    if (temp <= MINIMUM_AD_VALUE)
        return MINIMUM_TEMP_VALUE;
    else if (temp >= MAXIMUM_AD_VALUE)
        return MAXIMUM_TEMP_VALUE;
    else
        return (int)(COEFF_M_TIMES(temp) + COEFF_Q);
}
