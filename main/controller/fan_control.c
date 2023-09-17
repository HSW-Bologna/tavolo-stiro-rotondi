#include "model/model.h"
#include "esp_log.h"
#include "gel/timer/timecheck.h"
#include "utils/utils.h"


#define PHASE_CUT_SOFFIO      PHASE_CUT_FAN_1
#define PHASE_CUT_ASPIRAZIONE PHASE_CUT_FAN_2


typedef enum {
    STATE_NO_FOTOCELLULA = 0,
    STATE_WAITING_FOR_FOTOCELLULA,
    STATE_ASPIRAZIONE,
    STATE_SOFFIO,
} state_t;


static const char *TAG = "Fan control";

static unsigned long timestamp = 0;     // TODO: change this into a proper state machine
static state_t       state     = STATE_NO_FOTOCELLULA;


void fan_control(model_t *pmodel) {
    (void)TAG;

    uint8_t fotocellula = 0;
    switch (model_get_fotocellula(pmodel)) {
        case FOTOCELLULA_SX:
            fotocellula = model_digin_read(pmodel, DIGIN_FOTOCELLULA_SX);
            break;
        case FOTOCELLULA_DX:
            fotocellula = model_digin_read(pmodel, DIGIN_FOTOCELLULA_DX);
            break;
    };
    uint8_t pedale = model_digin_read(pmodel, DIGIN_PEDALE);

    switch (state) {
        case STATE_NO_FOTOCELLULA:
            if (fotocellula && !pedale) {
                state = STATE_WAITING_FOR_FOTOCELLULA;
            } else if (pedale) {
                state = STATE_SOFFIO;
            } else {
                timestamp = get_millis();
            }
            break;

        case STATE_WAITING_FOR_FOTOCELLULA:
            if (fotocellula && !pedale) {
                if (is_expired(timestamp, get_millis(), 100UL)) {
                    state     = STATE_ASPIRAZIONE;
                    timestamp = get_millis();
                }
            } else if (pedale) {
                state = STATE_SOFFIO;
            } else {
                state = STATE_NO_FOTOCELLULA;
            }
            break;

        case STATE_ASPIRAZIONE:
            if (fotocellula && !pedale) {
            } else if (pedale) {
                state = STATE_SOFFIO;
            } else {
                state = STATE_NO_FOTOCELLULA;
            }
            break;

        case STATE_SOFFIO:
            if (fotocellula && !pedale) {
                state     = STATE_NO_FOTOCELLULA;
                timestamp = get_millis();
            } else if (pedale) {
            } else {
                state = STATE_NO_FOTOCELLULA;
            }
            break;
    }

    switch (state) {
        case STATE_WAITING_FOR_FOTOCELLULA:
        case STATE_NO_FOTOCELLULA:
            model_set_aspirazione_on(pmodel, 0);
            model_set_soffio_on(pmodel, 0);
            break;

        case STATE_ASPIRAZIONE:
            model_set_aspirazione_on(pmodel, 1);
            model_set_soffio_on(pmodel, 0);
            break;

        case STATE_SOFFIO:
            model_set_aspirazione_on(pmodel, 0);
            model_set_soffio_on(pmodel, 1);
            break;
    }


    if (!model_get_test(pmodel)) {
        model_set_relay(pmodel, DIGOUT_ASPIRAZIONE, model_get_aspirazione_on(pmodel));
    }
}
