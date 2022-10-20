#include <driver/gpio.h>
#include "peripherals/i2c_devices.h"
#include "I2C/i2c_devices/io/TCA9534/tca9534.h"
#include "digout.h"
#include "esp_log.h"
#include "hardwareprofile.h"


static const char *TAG = "Digout";


static const tca9534_pin_t transform[] = {
    TCA9534_PIN_P6, TCA9534_PIN_P7, TCA9534_PIN_P5, TCA9534_PIN_P4,
    TCA9534_PIN_P1, TCA9534_PIN_P0, TCA9534_PIN_P3, TCA9534_PIN_P2,
};


void digout_init(void) {
    tca9534_configure_gpio_port(io_expander_2, 0);
    tca9534_set_output_port(io_expander_2, 0);

    gpio_config_t io_conf_output = {
        // interrupt of falling edge
        .intr_type = GPIO_INTR_DISABLE,
        // bit mask of the pins
        .pin_bit_mask = (1ULL << HAP_ASP),
        // set as input mode
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = 0,
        .pull_down_en = 0,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf_output));

    ESP_LOGI(TAG, "Digout initialized");
}


void digout_update(digout_t digout, uint8_t value) {

    switch (digout) {
        case DIGOUT_ASPIRAZIONE:
            gpio_set_level(HAP_ASP, value > 0);
            break;

        default:
            ESP_LOGI(TAG, "I2C Digout %i %i", digout, value);
            if (tca9534_set_output_pin(io_expander_2, transform[digout], value)) {
                ESP_LOGW(TAG, "I2C Error!");
            }
            break;
    }
}