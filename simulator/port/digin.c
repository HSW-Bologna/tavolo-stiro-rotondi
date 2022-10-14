#include "peripherals/digin.h"


uint8_t digin_have_changed(void) {
    return 0;
}


uint8_t digin_read(digin_t digin) {
    switch (digin) {
        case DIGIN_PEDALE:
            return 0;
        case DIGIN_FOTOCELLULA_DX:
            return 1;
        default:
            return 0;
    }
}


void digin_sync(void) {}