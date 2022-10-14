#include "model/model.h"
#include "peripherals/digin.h"
#include "peripherals/phase_cut.h"
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

    uint8_t fotocellula = digin_read(DIGIN_FOTOCELLULA_DX);
    uint8_t pedale      = digin_read(DIGIN_PEDALE);

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
                if (is_expired(timestamp, get_millis(), 300UL)) {
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
        phase_cut_set_percentage(PHASE_CUT_ASPIRAZIONE, model_get_percentuale_aspirazione(pmodel));
        phase_cut_set_percentage(PHASE_CUT_SOFFIO, model_get_percentuale_soffio(pmodel));
    }
}
