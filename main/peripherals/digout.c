#include "peripherals/i2c_devices.h"
#include "I2C/i2c_devices/io/TCA9534/tca9534.h"
#include "digout.h"
#include "esp_log.h"


static const char *TAG = "Digout";


static const tca9534_pin_t transform[] = {
    TCA9534_PIN_P6, TCA9534_PIN_P7, TCA9534_PIN_P5, TCA9534_PIN_P4,
    TCA9534_PIN_P1, TCA9534_PIN_P0, TCA9534_PIN_P3, TCA9534_PIN_P2,
};


void digout_init(void) {
    tca9534_configure_gpio_port(io_expander_2, 0);
    tca9534_set_output_port(io_expander_2, 0);
    ESP_LOGI(TAG, "Digout initialized");
}


void digout_update(digout_t digout, uint8_t value) {
    tca9534_set_output_pin(io_expander_2, transform[digout], value);
}