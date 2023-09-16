#ifndef MODBUS_H_INCLUDED
#define MODBUS_H_INCLUDED


#include <stdint.h>


typedef enum {
    MODBUS_RESPONSE_TAG_OK,
    MODBUS_RESPONSE_TAG_FIRMWARE_VERSION,
    MODBUS_RESPONSE_TAG_START_OTA,
    MODBUS_RESPONSE_TAG_READ_STATE,
} modbus_response_tag_t;


typedef struct {
    modbus_response_tag_t tag;
    uint8_t               error;
    union {
        struct {
            uint16_t inputs_map;
            uint16_t liquid_levels[2];
            uint16_t ptc_adcs[2];
            uint16_t ptc_temperatures[2];
        } state;
        struct {
            uint16_t version_major;
            uint16_t version_minor;
            uint16_t version_patch;
        } firmware_version;
    } as;
} modbus_response_t;


void    modbus_init(void);
uint8_t modbus_get_response(modbus_response_t *response);
void    modbus_read_firmware_version(void);
void    modbus_read_state(void);
void    modbus_write_outputs(uint8_t relays, uint8_t percentage_suction, uint8_t percentage_blow);


#endif
