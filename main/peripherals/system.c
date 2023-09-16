#include "driver/i2c.h"
#include "esp_log.h"
#include "hardwareprofile.h"
#include "system.h"


static const char *TAG = "System";


int system_i2c_init(void) {
    const int port     = I2C_NUM_1;
    const int sda_pin  = HAP_SDA_CLK;
    const int scl_pin  = HAP_SCL_CLK;
    const int speed_hz = 400000;

    ESP_LOGI(TAG, "initializing i2c master port %d...", port);
    ESP_LOGI(TAG, "sda pin: %d, scl pin: %d, speed: %d (hz)", sda_pin, scl_pin, speed_hz);

    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = sda_pin,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_io_num       = scl_pin,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = speed_hz,
    };

    ESP_LOGI(TAG, "setting i2c master configuration...");
    ESP_ERROR_CHECK(i2c_param_config(port, &conf));

    ESP_LOGI(TAG, "installing i2c master driver...");
    ESP_ERROR_CHECK(i2c_driver_install(port, I2C_MODE_MASTER, 0,
                                       0 /*i2c_master_rx_buf_disable, i2c_master_tx_buf_disable */,
                                       0 /* intr_alloc_flags */));
    ESP_ERROR_CHECK(i2c_set_timeout(port, 20));

    return ESP_OK;
}
