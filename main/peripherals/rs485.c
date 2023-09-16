#include <driver/gpio.h>
#include <driver/uart.h>
#include "hardwareprofile.h"
#include "rs485.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <string.h>
#include "config/app_config.h"


#define PORTNUM        UART_NUM_1
#define ECHO_READ_TOUT (3)     // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks


static const char   *TAG              = "RS485";
static QueueHandle_t uart_event_queue = NULL;


void rs485_init(void) {
    ESP_LOGI(TAG, "Initializing RS485...");

    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(HAP_DIR),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
    gpio_set_level(HAP_DIR, 0);

    uart_config_t uart_config = {
        .baud_rate           = 115200,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(PORTNUM, &uart_config));

    uart_set_pin(PORTNUM, HAP_TX_485, HAP_RX_485, -1, -1);
    ESP_ERROR_CHECK(uart_driver_install(PORTNUM, 512, 512, 16, &uart_event_queue, 0));
    ESP_ERROR_CHECK(uart_set_mode(PORTNUM, UART_MODE_RS485_HALF_DUPLEX));
    ESP_ERROR_CHECK(uart_set_rx_timeout(PORTNUM, ECHO_READ_TOUT));
}


void rs485_wait_tx_done(void) {
    uart_wait_tx_done(PORTNUM, portMAX_DELAY);
}


void rs485_flush(void) {
    uart_flush(PORTNUM);
}


void rs485_flush_input(void) {
    uart_flush_input(PORTNUM);
}


void rs485_write(uint8_t *buffer, size_t len) {
    gpio_set_level(HAP_DIR, 1);
    ets_delay_us(10);
    uart_write_bytes(PORTNUM, buffer, len);
    ESP_ERROR_CHECK(uart_wait_tx_done(PORTNUM, portMAX_DELAY));
    ets_delay_us(200);
    gpio_set_level(HAP_DIR, 0);
    // In this particular project asserting/deasserting the DIR line causes a couple of null bytes to be received, so we
    // must flush them
    rs485_flush();
}


int rs485_read(uint8_t *buffer, size_t len, unsigned long timeout_ms) {
    return uart_read_bytes(PORTNUM, buffer, len, pdMS_TO_TICKS(timeout_ms));
}
