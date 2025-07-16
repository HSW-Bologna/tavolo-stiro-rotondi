#ifndef MINION_H_INCLUDED
#define MINION_H_INCLUDED


#include <stdint.h>


typedef enum {
    MINION_RESPONSE_TAG_OK,
    MINION_RESPONSE_TAG_FIRMWARE_VERSION,
    MINION_RESPONSE_TAG_START_OTA,
    MINION_RESPONSE_TAG_READ_STATE,
} minion_response_tag_t;


typedef struct {
    minion_response_tag_t tag;
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
} minion_response_t;


void    minion_init(void);
uint8_t minion_get_response(minion_response_t *response);
void    minion_read_firmware_version(void);
void    minion_read_state(void);
void    minion_write_outputs(uint16_t relays, uint8_t percentage_suction, uint8_t percentage_blow);
void    minion_height_regulator_update(uint8_t enabled, uint16_t position);


#endif
