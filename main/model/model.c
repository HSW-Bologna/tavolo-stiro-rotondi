#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "model.h"
#include "app_config.h"
#include "esp_log.h"


#define ADC_PROBE_1_THRESHOLD 2048


static const char *TAG = "Model";


void model_init(model_t *pmodel) {
    assert(pmodel != NULL);
    (void)TAG;

    memset(pmodel, 0, sizeof(*pmodel));

    pmodel->run.machine_state                   = MACHINE_STATE_ON;
    pmodel->configuration.language              = 0;
    pmodel->run.luce                            = 0;
    pmodel->run.gun_state                       = 0;
    pmodel->run.ferro_1                         = 0;
    pmodel->run.ferro_2                         = 0;
    pmodel->run.soffio_on                       = 0;
    pmodel->run.aspirazione_on                  = 0;
    pmodel->run.tavolo_on                       = 0;
    pmodel->run.bracciolo_on                    = 0;
    pmodel->run.boiler_on                       = 0;
    pmodel->run.pompa_on                        = 0;
    pmodel->run.richiesta_temperatura_tavolo    = 0;
    pmodel->run.richiesta_temperatura_bracciolo = 0;
    pmodel->run.richiesta_boiler                = 0;
    pmodel->run.test_percentage_suction         = 0;
    pmodel->run.test_percentage_blow            = 0;
    pmodel->run.alarm_communication             = 0;

    pmodel->configuration.machine_model                  = MACHINE_MODEL_NONE;
    pmodel->configuration.second_iron_enabled            = 0;
    pmodel->configuration.steam_gun_enabled              = 0;
    pmodel->configuration.heated_arm_enabled             = 0;
    pmodel->configuration.light_enabled                  = 0;
    pmodel->configuration.boiler_enabled                 = 0;
    pmodel->configuration.max_temperatura_tavolo         = APP_CONFIG_DEFAULT_LIMITE_TEMPERATURA;
    pmodel->configuration.max_temperatura_bracciolo      = APP_CONFIG_DEFAULT_LIMITE_TEMPERATURA;
    pmodel->configuration.setpoint_temperatura_tavolo    = 50;
    pmodel->configuration.setpoint_temperatura_bracciolo = 50;
    pmodel->configuration.velocita_soffio                = NUM_SPEED_STEPS - 1;
    pmodel->configuration.velocita_aspirazione           = NUM_SPEED_STEPS - 1;
    pmodel->configuration.isteresi_tavolo                = 2;
    pmodel->configuration.isteresi_bracciolo             = 2;
    pmodel->configuration.numero_sonde                   = 1;
    pmodel->configuration.boiler_adc_threshold           = 2000;
    pmodel->configuration.isteresi_caldaia               = 15;
    pmodel->configuration.fan_config                     = FAN_CONFIG_SWITCH;
    pmodel->configuration.height_regulation              = 0;
    pmodel->configuration.height_regulation_presets[0]              = 0;
    pmodel->configuration.height_regulation_presets[1]              = 50;
    pmodel->configuration.height_regulation_presets[2]              = 100;

    pmodel->configuration.percentuali_soffio[0] = 40;
    pmodel->configuration.percentuali_soffio[1] = 45;
    pmodel->configuration.percentuali_soffio[2] = 50;
    pmodel->configuration.percentuali_soffio[3] = 55;
    pmodel->configuration.percentuali_soffio[4] = 60;
    pmodel->configuration.percentuali_soffio[5] = 65;
    pmodel->configuration.percentuali_soffio[6] = 70;
    pmodel->configuration.percentuali_soffio[7] = 100;

    pmodel->configuration.percentuali_aspirazione[0] = 40;
    pmodel->configuration.percentuali_aspirazione[1] = 45;
    pmodel->configuration.percentuali_aspirazione[2] = 50;
    pmodel->configuration.percentuali_aspirazione[3] = 55;
    pmodel->configuration.percentuali_aspirazione[4] = 60;
    pmodel->configuration.percentuali_aspirazione[5] = 65;
    pmodel->configuration.percentuali_aspirazione[6] = 70;
    pmodel->configuration.percentuali_aspirazione[7] = 100;

    memset(pmodel->minion.inputs, 0, sizeof(pmodel->minion.inputs));
    pmodel->minion.relays              = 0;
    pmodel->minion.liquid_levels[0]    = 0;
    pmodel->minion.liquid_levels[1]    = 0;
    pmodel->minion.ptc_adcs[0]         = 0;
    pmodel->minion.ptc_adcs[1]         = 0;
    pmodel->minion.ptc_temperatures[0] = 0;
    pmodel->minion.ptc_temperatures[1] = 0;
}


void model_set_machine_model(model_t *pmodel, machine_model_t machine_model) {
    assert(pmodel != NULL);

    pmodel->configuration.machine_model = machine_model;

    switch (machine_model) {
        case MACHINE_MODEL_NONE:
            pmodel->configuration.fan_config          = FAN_CONFIG_SUCTION_ONLY;
            pmodel->configuration.second_iron_enabled = 0;
            pmodel->configuration.heated_arm_enabled  = 0;
            pmodel->configuration.light_enabled       = 0;
            pmodel->configuration.boiler_enabled      = 0;
            pmodel->configuration.steam_gun_enabled   = 0;
            break;

        case MACHINE_MODEL_388_398_399:
            pmodel->configuration.fan_config          = FAN_CONFIG_SUCTION_ONLY;
            pmodel->configuration.second_iron_enabled = 0;
            pmodel->configuration.heated_arm_enabled  = 0;
            pmodel->configuration.light_enabled       = 0;
            pmodel->configuration.boiler_enabled      = 0;
            pmodel->configuration.steam_gun_enabled   = 0;
            break;

        case MACHINE_MODEL_400:
            pmodel->configuration.fan_config          = FAN_CONFIG_BOTH;
            pmodel->configuration.second_iron_enabled = 0;
            pmodel->configuration.heated_arm_enabled  = 0;
            pmodel->configuration.light_enabled       = 0;
            pmodel->configuration.boiler_enabled      = 0;
            pmodel->configuration.steam_gun_enabled   = 0;
            break;

        case MACHINE_MODEL_2000:
            pmodel->configuration.fan_config          = FAN_CONFIG_SWITCH;
            pmodel->configuration.second_iron_enabled = 0;
            pmodel->configuration.heated_arm_enabled  = 1;
            pmodel->configuration.light_enabled       = 0;
            pmodel->configuration.boiler_enabled      = 0;
            pmodel->configuration.steam_gun_enabled   = 0;
            break;

        case MACHINE_MODEL_2500:
            pmodel->configuration.fan_config          = FAN_CONFIG_SUCTION_ONLY;
            pmodel->configuration.second_iron_enabled = 0;
            pmodel->configuration.heated_arm_enabled  = 0;
            pmodel->configuration.light_enabled       = 0;
            pmodel->configuration.boiler_enabled      = 0;
            pmodel->configuration.steam_gun_enabled   = 0;
            break;

        case MACHINE_MODEL_3000:
            pmodel->configuration.fan_config          = FAN_CONFIG_SUCTION_ONLY;
            pmodel->configuration.second_iron_enabled = 0;
            pmodel->configuration.heated_arm_enabled  = 0;
            pmodel->configuration.light_enabled       = 0;
            pmodel->configuration.boiler_enabled      = 1;
            pmodel->configuration.steam_gun_enabled   = 0;
            break;

        case MACHINE_MODEL_3300:
            pmodel->configuration.fan_config          = FAN_CONFIG_SWITCH;
            pmodel->configuration.second_iron_enabled = 0;
            pmodel->configuration.heated_arm_enabled  = 0;
            pmodel->configuration.light_enabled       = 0;
            pmodel->configuration.boiler_enabled      = 1;
            pmodel->configuration.steam_gun_enabled   = 0;
            break;
    }
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


uint8_t model_get_light(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.light_enabled && pmodel->run.luce;
}


uint8_t model_get_second_iron(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.second_iron_enabled && pmodel->run.ferro_2;
}


uint8_t model_get_steam_gun(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.steam_gun_enabled && pmodel->run.gun_state;
}


uint8_t model_is_blow_fan_configured(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.fan_config != FAN_CONFIG_SUCTION_ONLY;
}


uint8_t model_is_in_test(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.machine_state == MACHINE_STATE_TEST;
}


uint8_t model_is_in_standby(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.machine_state == MACHINE_STATE_STANDBY;
}


void model_set_machine_test(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->run.machine_state = MACHINE_STATE_TEST;
}


void model_set_machine_on(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->run.machine_state = MACHINE_STATE_ON;
}


void model_set_machine_standby(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->run.machine_state = MACHINE_STATE_STANDBY;
}


uint8_t model_get_percentuale_soffio(model_t *pmodel) {
    assert(pmodel != NULL);
    if (model_is_in_test(pmodel)) {
        return model_get_test_percentage_blow(pmodel);
    } else if (model_get_soffio_on(pmodel)) {
        return pmodel->configuration.percentuali_soffio[model_get_velocita_soffio(pmodel)];
    } else {
        return 0;
    }
}


uint8_t model_get_percentuale_aspirazione(model_t *pmodel) {
    assert(pmodel != NULL);
    if (model_is_in_test(pmodel)) {
        return model_get_test_percentage_suction(pmodel);
    } else if (model_get_aspirazione_on(pmodel)) {
        return pmodel->configuration.percentuali_aspirazione[model_get_velocita_aspirazione(pmodel)];
    } else {
        return 0;
    }
}


uint16_t model_get_language(model_t *pmodel) {
    assert(pmodel != NULL);
    return 0;
}


void model_set_suction_trap(model_t *model, uint8_t value) {
    assert(model != NULL);
    if (value) {
        model->minion.relays |= 1 << DIGOUT_TAGLIOLA_1;
    } else {
        model->minion.relays &= ~(1 << DIGOUT_TAGLIOLA_1);
    }
}


void model_set_blow_trap(model_t *model, uint8_t value) {
    assert(model != NULL);
    if (value) {
        model->minion.relays |= 1 << DIGOUT_TAGLIOLA_2;
    } else {
        model->minion.relays &= ~(1 << DIGOUT_TAGLIOLA_2);
    }
}


uint8_t model_should_activate_table(model_t *pmodel) {
    assert(pmodel != NULL);
    // FIXME: pezza fiera
    return model_get_richiesta_temperatura_tavolo(pmodel);
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
    return pmodel->configuration.heated_arm_enabled && model_get_richiesta_temperatura_bracciolo(pmodel);
    // FIXME: pezze fiera
    if (model_get_richiesta_temperatura_bracciolo(pmodel)) {
        return (model_get_temperatura_bracciolo(pmodel) + model_get_isteresi_bracciolo(pmodel) <
                model_get_setpoint_temperatura_bracciolo(pmodel));
    } else {
        return 0;
    }
}


uint8_t model_should_deactivate_arm(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.heated_arm_enabled || !model_get_richiesta_temperatura_bracciolo(pmodel);
    // FIXME: pezze fiera
    if (model_get_richiesta_temperatura_bracciolo(pmodel)) {
        return (model_get_temperatura_bracciolo(pmodel) >
                model_get_setpoint_temperatura_bracciolo(pmodel) + model_get_isteresi_bracciolo(pmodel));
    } else {
        return 1;
    }
}


uint8_t model_liquid_threshold_1_reached(model_t *pmodel) {
    assert(pmodel != NULL);
    return 1;
    // Fixme: pezze fiera
    // return pmodel->minion.liquid_levels[0] < ADC_PROBE_1_THRESHOLD;
}


uint8_t model_boiler_pieno(model_t *pmodel) {
    assert(pmodel != NULL);
    return 1;
    // Fixme: pezze fiera
    // return pmodel->minion.liquid_levels[1] < model_get_boiler_adc_threshold(pmodel);
}


void model_set_relay(model_t *pmodel, size_t coil, uint8_t value) {
    assert(pmodel != NULL);
    pmodel->minion.relays = (pmodel->minion.relays & ~(1 << coil)) | ((value > 0) << coil);
}


uint8_t model_digin_read(model_t *pmodel, digin_t digin) {
    assert(pmodel != NULL);
    return pmodel->minion.inputs[digin];
}


uint8_t model_update_minion_state(model_t *pmodel, uint16_t input_map, uint16_t *liquid_levels, uint16_t *ptc_adcs,
                                  uint16_t *ptc_temperatures) {
    assert(pmodel != NULL);
    uint8_t change = 0;

    for (size_t i = 0; i < NUM_INPUTS; i++) {
        uint8_t new_value = (input_map & (1 << i)) > 0;
        if (pmodel->minion.inputs[i] != new_value) {
            pmodel->minion.inputs[i] = new_value;
            change                   = 1;
        }
    }

    for (size_t i = 0; i < LIQUID_LEVEL_PROBE_NUM; i++) {
        if (pmodel->minion.liquid_levels[i] != liquid_levels[i]) {
            pmodel->minion.liquid_levels[i] = liquid_levels[i];
            change                          = 1;
        }
    }

    for (size_t i = 0; i < PTC_NUM; i++) {
        if (pmodel->minion.ptc_adcs[i] != ptc_adcs[i]) {
            pmodel->minion.ptc_adcs[i] = ptc_adcs[i];
            change                     = 1;
        }
        if (pmodel->minion.ptc_temperatures[i] != ptc_temperatures[i]) {
            pmodel->minion.ptc_temperatures[i] = ptc_temperatures[i];
            change                             = 1;
        }
    }

    return change;
}


uint16_t model_get_probe_level(model_t *pmodel, liquid_level_probe_t probe) {
    assert(pmodel != NULL);
    return pmodel->minion.liquid_levels[probe];
}


int16_t model_get_temperatura_tavolo(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->minion.ptc_temperatures[PTC_TEMP2];
}


int16_t model_get_temperatura_bracciolo(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->minion.ptc_temperatures[PTC_TEMP1];
}


uint16_t model_get_adc_ptc(model_t *pmodel, ptc_t ptc) {
    assert(pmodel != NULL);
    return pmodel->minion.ptc_adcs[ptc];
}
