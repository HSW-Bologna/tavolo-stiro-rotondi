#include "I2C/i2c_devices/io/TCA9534/tca9534.h"
#include "I2C/i2c_devices/rtc/RX8010/rx8010.h"
#include "I2C/i2c_ports/esp-idf/esp_idf_i2c_port.h"
#include "i2c_devices.h"



i2c_driver_t io_expander_1 = {
    .device_address = TCA9534_DEFAULT_ADDR,
    .i2c_transfer   = esp_idf_i2c_port_transfer,
    .arg            = (void *)(uintptr_t)I2C_NUM_0,
};


i2c_driver_t io_expander_2 = {
    .device_address = TCA9534_DEFAULT_ADDR | 0x02,
    .i2c_transfer   = esp_idf_i2c_port_transfer,
    .arg            = (void *)(uintptr_t)I2C_NUM_0,
};


i2c_driver_t rx8010_driver = {
    .device_address = RX8010_DEFAULT_ADDRESS,
    .i2c_transfer   = esp_idf_i2c_port_transfer,
    .arg            = (void *)(uintptr_t)I2C_NUM_0,
};
