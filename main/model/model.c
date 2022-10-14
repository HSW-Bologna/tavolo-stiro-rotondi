#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "model.h"
#include "app_config.h"
#include "esp_log.h"


#define ADC_PROBE_1_THRESHOLD 2048
#define ADC_PROBE_2_THRESHOLD 600


static const char *TAG = "Model";


void model_init(model_t *pmodel) {
    assert(pmodel != NULL);
    (void)TAG;

    pmodel->configuration.language              = 0;
    pmodel->run.luce                            = 0;
    pmodel->run.test                            = 0;
    pmodel->run.ferro_1                         = 0;
    pmodel->run.ferro_2                         = 0;
    pmodel->run.soffio_on                       = 0;
    pmodel->run.aspirazione_on                  = 0;
    pmodel->run.temperatura_tavolo              = 0;
    pmodel->run.temperatura_bracciolo           = 0;
    pmodel->run.tavolo_on                       = 0;
    pmodel->run.bracciolo_on                    = 0;
    pmodel->run.boiler_on                       = 0;
    pmodel->run.richiesta_temperatura_tavolo    = 0;
    pmodel->run.richiesta_temperatura_bracciolo = 0;
    pmodel->run.richiesta_boiler                = 0;
    pmodel->run.liquid_level                    = LIQUID_LEVEL_1;

    pmodel->configuration.max_temperatura_tavolo         = APP_CONFIG_DEFAULT_LIMITE_TEMPERATURA;
    pmodel->configuration.max_temperatura_bracciolo      = APP_CONFIG_DEFAULT_LIMITE_TEMPERATURA;
    pmodel->configuration.setpoint_temperatura_tavolo    = 50;
    pmodel->configuration.setpoint_temperatura_bracciolo = 50;
    pmodel->configuration.velocita_soffio                = NUM_SPEED_STEPS - 1;
    pmodel->configuration.isteresi_tavolo                = 2;
    pmodel->configuration.isteresi_bracciolo             = 2;
    pmodel->configuration.numero_sonde                   = 1;

    pmodel->configuration.fotocellula = FOTOCELLULA_SX;

    pmodel->configuration.percentuali_soffio[0] = 20;
    pmodel->configuration.percentuali_soffio[1] = 30;
    pmodel->configuration.percentuali_soffio[2] = 40;
    pmodel->configuration.percentuali_soffio[3] = 45;
    pmodel->configuration.percentuali_soffio[4] = 50;
    pmodel->configuration.percentuali_soffio[5] = 60;
    pmodel->configuration.percentuali_soffio[6] = 80;
    pmodel->configuration.percentuali_soffio[7] = 100;

    memset(pmodel->test.inputs, 0, sizeof(pmodel->test.inputs));
    pmodel->test.adc_ptc_1  = 0;
    pmodel->test.adc_ptc_2  = 0;
    pmodel->run.adc_level_1 = 0;
    pmodel->run.adc_level_2 = 0;
}


void model_toggle_soffio(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->run.soffio_on      = !pmodel->run.soffio_on;
    pmodel->run.aspirazione_on = 0;
}


void model_toggle_aspirazione(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->run.aspirazione_on = !pmodel->run.aspirazione_on;
    pmodel->run.soffio_on      = 0;
}


uint8_t model_get_percentuale_soffio(model_t *pmodel) {
    assert(pmodel != NULL);
    if (model_get_soffio_on(pmodel)) {
        return pmodel->configuration.percentuali_soffio[model_get_velocita_soffio(pmodel)];
    } else {
        return 0;
    }
}


uint8_t model_get_percentuale_aspirazione(model_t *pmodel) {
    assert(pmodel != NULL);
    return model_get_aspirazione_on(pmodel) ? 100 : 0;
}


uint16_t model_get_language(model_t *pmodel) {
    assert(pmodel != NULL);
    return 0;
}


uint8_t model_should_activate_table(model_t *pmodel) {
    assert(pmodel != NULL);
    if (model_get_richiesta_temperatura_tavolo(pmodel)) {
        return (model_get_temperatura_tavolo(pmodel) + model_get_isteresi_tavolo(pmodel) <
                model_get_setpoint_temperatura_tavolo(pmodel));
    } else {
        return 0;
    }
}


uint8_t model_should_deactivate_table(model_t *pmodel) {
    assert(pmodel != NULL);
    if (model_get_richiesta_temperatura_tavolo(pmodel)) {
        return (model_get_temperatura_tavolo(pmodel) >
                model_get_setpoint_temperatura_tavolo(pmodel) + model_get_isteresi_tavolo(pmodel));
    } else {
        return 1;
    }
}


uint8_t model_should_activate_arm(model_t *pmodel) {
    assert(pmodel != NULL);
    if (model_get_richiesta_temperatura_bracciolo(pmodel)) {
        return (model_get_temperatura_bracciolo(pmodel) + model_get_isteresi_bracciolo(pmodel) <
                model_get_setpoint_temperatura_bracciolo(pmodel));
    } else {
        return 0;
    }
}


uint8_t model_should_deactivate_arm(model_t *pmodel) {
    assert(pmodel != NULL);
    if (model_get_richiesta_temperatura_bracciolo(pmodel)) {
        return (model_get_temperatura_bracciolo(pmodel) >
                model_get_setpoint_temperatura_bracciolo(pmodel) + model_get_isteresi_bracciolo(pmodel));
    } else {
        return 1;
    }
}


uint8_t model_liquid_threshold_1_reached(model_t *pmodel) {
    assert(pmodel != NULL);
    return model_get_adc_level_1(pmodel) < ADC_PROBE_1_THRESHOLD;
}


uint8_t model_liquid_threshold_2_reached(model_t *pmodel) {
    assert(pmodel != NULL);
    return model_get_adc_level_2(pmodel) < ADC_PROBE_2_THRESHOLD;
}