#include "model/model.h"
#include "esp_log.h"
#include "gel/timer/timecheck.h"
#include "utils/utils.h"


#define PHASE_CUT_SOFFIO      PHASE_CUT_FAN_1
#define PHASE_CUT_ASPIRAZIONE PHASE_CUT_FAN_2

#define SWITCH_DELAY_MS 100UL


typedef enum {
    STATE_STOPPED = 0,
    STATE_SUCTION_DELAY,
    STATE_SUCTION,
    STATE_BLOWING_DELAY,
    STATE_BLOWING,
} state_t;


static const char *TAG = "Fan control";

static unsigned long timestamp = 0;
static state_t       state     = STATE_STOPPED;


void fan_control(model_t *pmodel) {
    (void)TAG;

    uint8_t suction_pedal = model_digin_read(pmodel, DIGIN_SUCTION_PEDAL);
    uint8_t blowing_pedal = model_digin_read(pmodel, DIGIN_BLOWING_PEDAL);

    switch (state) {
        case STATE_STOPPED:
            if (suction_pedal) {
                state = is_expired(timestamp, get_millis(), SWITCH_DELAY_MS) ? STATE_SUCTION : STATE_SUCTION_DELAY;
            } else if (blowing_pedal) {
                state = is_expired(timestamp, get_millis(), SWITCH_DELAY_MS) ? STATE_BLOWING : STATE_BLOWING_DELAY;
            }
            break;

        case STATE_BLOWING_DELAY:
            if (is_expired(timestamp, get_millis(), SWITCH_DELAY_MS)) {
                state = STATE_BLOWING;
            }
            break;

        case STATE_BLOWING:
            timestamp = get_millis();
            if (!blowing_pedal) {
                if (suction_pedal) {
                    state = STATE_SUCTION_DELAY;
                } else {
                    state = STATE_STOPPED;
                }
            }
            break;

        case STATE_SUCTION_DELAY:
            if (is_expired(timestamp, get_millis(), SWITCH_DELAY_MS)) {
                state = STATE_SUCTION;
            }
            break;

        case STATE_SUCTION:
            timestamp = get_millis();
            if (blowing_pedal) {     // Blowing fan takes precedence
                state = STATE_BLOWING_DELAY;
            } else if (!suction_pedal) {
                state = STATE_STOPPED;
            }
            break;
    }

    switch (state) {
        case STATE_SUCTION_DELAY:
        case STATE_BLOWING_DELAY:
        case STATE_STOPPED:
            model_set_aspirazione_on(pmodel, 0);
            model_set_soffio_on(pmodel, 0);
            break;

        case STATE_SUCTION:
            model_set_aspirazione_on(pmodel, 1);
            model_set_soffio_on(pmodel, 0);
            break;

        case STATE_BLOWING:
            model_set_aspirazione_on(pmodel, 0);
            model_set_soffio_on(pmodel, 1);
            break;
    }
}
