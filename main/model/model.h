#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>


#define NUM_OUTPUTS 10


#define GETTER(name, field)                                                                                            \
    static inline                                                                                                      \
        __attribute__((always_inline, const)) typeof(((model_t *)0)->field) model_get_##name(model_t *pmodel) {        \
        assert(pmodel != NULL);                                                                                        \
        return pmodel->field;                                                                                          \
    }

#define SETTER(name, field)                                                                                            \
    static inline __attribute__((always_inline))                                                                       \
    uint8_t model_set_##name(model_t *pmodel, typeof(((model_t *)0)->field) value) {                                   \
        assert(pmodel != NULL);                                                                                        \
        if (pmodel->field != value) {                                                                                  \
            pmodel->field = value;                                                                                     \
            return 1;                                                                                                  \
        } else {                                                                                                       \
            return 0;                                                                                                  \
        }                                                                                                              \
    }

#define TOGGLER(name, field)                                                                                           \
    static inline __attribute__((always_inline)) void model_toggle_##name(model_t *pmodel) {                           \
        assert(pmodel != NULL);                                                                                        \
        pmodel->field = !pmodel->field;                                                                                \
    }

#define GETTERNSETTER(name, field)                                                                                     \
    GETTER(name, field)                                                                                                \
    SETTER(name, field)


#define NUM_INPUTS                5
#define NUM_SPEED_STEPS           8
#define DIGOUT_GUN                DIGOUT_POMPA
#define HEIGHT_REGULATION_PRESETS 3


typedef enum {
    FAN_CONFIG_SWITCH,
    FAN_CONFIG_BOTH,
    FAN_CONFIG_SUCTION_ONLY,
} fan_config_t;


typedef enum {
    DIGOUT_POMPA = 0,
    DIGOUT_RISCALDAMENTO_VAPORE,
    DIGOUT_RISCALDAMENTO_PIANO,
    DIGOUT_RISCALDAMENTO_BRACCIOLO,
    DIGOUT_RISCALDAMENTO_FERRO_2,
    DIGOUT_RISCALDAMENTO_FERRO_1,
    DIGOUT_LUCE,
    DIGOUT_RECUPERATOR,
    DIGOUT_AUX = 8,
    DIGOUT_TAGLIOLA_1,
    DIGOUT_TAGLIOLA_2,
} digout_t;


#define DIGIN_SUCTION_PEDAL   DIGIN_IN1
#define DIGIN_BLOWING_PEDAL   DIGIN_IN2
#define DIGIN_AIR_FLOW_SWITCH DIGIN_IN3


typedef enum {
    DIGIN_IN1 = 0,
    DIGIN_IN2,
    DIGIN_IN3,
    DIGIN_IN4,
    DIGIN_VAP,
} digin_t;


typedef enum {
    FOTOCELLULA_SX = 0,
    FOTOCELLULA_DX,
} fotocellula_t;


typedef enum {
    LIQUID_LEVEL_PROBE_2 = 0,
    LIQUID_LEVEL_PROBE_1,
#define LIQUID_LEVEL_PROBE_NUM 2
} liquid_level_probe_t;


typedef enum {
    PTC_TEMP1 = 0,
    PTC_TEMP2,
#define PTC_NUM 2
} ptc_t;


typedef enum {
    MACHINE_MODEL_388_398_399 = 0,
    MACHINE_MODEL_400,
    MACHINE_MODEL_2000,
    MACHINE_MODEL_2500,
    MACHINE_MODEL_3000,
    MACHINE_MODEL_3300,
    MACHINE_MODEL_NONE = 0xFF,
#define NUM_MACHINE_MODELS 6
} machine_model_t;


typedef enum {
    MACHINE_STATE_ON = 0,
    MACHINE_STATE_TEST,
    MACHINE_STATE_STANDBY,
} machine_state_t;


typedef struct {
    struct {
        uint8_t  machine_model;
        uint8_t  second_iron_enabled;
        uint8_t  heated_arm_enabled;
        uint8_t  steam_gun_enabled;
        uint8_t  light_enabled;
        uint8_t  boiler_enabled;
        uint16_t language;
        uint16_t setpoint_temperatura_tavolo;
        uint16_t setpoint_temperatura_bracciolo;
        uint16_t max_temperatura_tavolo;
        uint16_t max_temperatura_bracciolo;
        uint8_t  velocita_soffio;
        uint8_t  percentuali_soffio[NUM_SPEED_STEPS];
        uint8_t  velocita_aspirazione;
        uint8_t  percentuali_aspirazione[NUM_SPEED_STEPS];
        uint16_t isteresi_tavolo;
        uint16_t isteresi_bracciolo;
        uint16_t isteresi_caldaia;
        uint8_t  numero_sonde;
        uint16_t boiler_adc_threshold;
        uint8_t  fan_config;
        uint8_t  height_regulation;
        uint16_t selected_height_preset;
        uint16_t height_regulation_presets[HEIGHT_REGULATION_PRESETS];
    } configuration;

    struct {
        machine_state_t machine_state;

        uint8_t luce;
        uint8_t ferro_1;
        uint8_t ferro_2;
        uint8_t soffio_on;
        uint8_t aspirazione_on;
        uint8_t boiler_on;
        uint8_t pompa_on;
        uint8_t richiesta_temperatura_tavolo;
        uint8_t richiesta_temperatura_bracciolo;
        uint8_t richiesta_boiler;
        uint8_t tavolo_on;
        uint8_t bracciolo_on;
        uint8_t test_percentage_suction;
        uint8_t test_percentage_blow;
        uint8_t alarm_communication;
        uint8_t gun_state;
    } run;

    struct {
        // Outputs
        uint16_t relays;

        // Inputs
        uint8_t  inputs[NUM_INPUTS];
        uint16_t liquid_levels[LIQUID_LEVEL_PROBE_NUM];
        uint16_t ptc_adcs[PTC_NUM];
        int16_t  ptc_temperatures[PTC_NUM];
    } minion;
} model_t;


void     model_init(model_t *pmodel);
uint16_t model_get_language(model_t *pmodel);
void     model_toggle_aspirazione(model_t *pmodel);
void     model_toggle_soffio(model_t *pmodel);
uint8_t  model_get_percentuale_soffio(model_t *pmodel);
uint8_t  model_get_percentuale_aspirazione(model_t *pmodel);
uint8_t  model_should_activate_table(model_t *pmodel);
uint8_t  model_should_deactivate_table(model_t *pmodel);
uint8_t  model_should_activate_arm(model_t *pmodel);
uint8_t  model_should_deactivate_arm(model_t *pmodel);
uint8_t  model_liquid_threshold_1_reached(model_t *pmodel);
uint8_t  model_boiler_pieno(model_t *pmodel);
uint8_t  model_update_minion_state(model_t *pmodel, uint16_t input_map, uint16_t *liquid_levels, uint16_t *ptc_adcs,
                                   uint16_t *ptc_temperatures);
void     model_set_relay(model_t *pmodel, size_t relay, uint8_t value);
uint8_t  model_digin_read(model_t *pmodel, digin_t digin);
uint16_t model_get_probe_level(model_t *pmodel, liquid_level_probe_t probe);
int16_t  model_get_ptc_temperature(model_t *pmodel, ptc_t ptc);
int16_t  model_get_temperatura_tavolo(model_t *pmodel);
int16_t  model_get_temperatura_bracciolo(model_t *pmodel);
uint16_t model_get_adc_ptc(model_t *pmodel, ptc_t ptc);
uint8_t  model_get_light(model_t *pmodel);
uint8_t  model_get_second_iron(model_t *pmodel);
uint8_t  model_is_blow_fan_configured(model_t *pmodel);
uint8_t  model_get_steam_gun(model_t *pmodel);
void     model_set_machine_model(model_t *pmodel, machine_model_t machine_model);
uint8_t  model_is_in_test(model_t *pmodel);
uint8_t  model_is_in_standby(model_t *pmodel);
void     model_set_machine_standby(model_t *pmodel);
void     model_set_machine_on(model_t *pmodel);
void     model_set_machine_test(model_t *pmodel);
void     model_set_blow_trap(model_t *model, uint8_t value);
void     model_set_suction_trap(model_t *model, uint8_t value);

GETTER(minion_relays, minion.relays);
GETTER(ferro_1, run.ferro_1);

GETTER(max_temperatura_tavolo, configuration.max_temperatura_tavolo);
GETTER(max_temperatura_bracciolo, configuration.max_temperatura_bracciolo);
GETTER(isteresi_tavolo, configuration.isteresi_tavolo);
GETTER(isteresi_bracciolo, configuration.isteresi_bracciolo);
GETTER(isteresi_caldaia, configuration.isteresi_caldaia);
GETTER(richiesta_boiler, run.richiesta_boiler);

GETTERNSETTER(alarm_communication, run.alarm_communication);
GETTERNSETTER(gun_state, run.gun_state);
GETTERNSETTER(soffio_on, run.soffio_on);
GETTERNSETTER(aspirazione_on, run.aspirazione_on);
GETTERNSETTER(setpoint_temperatura_tavolo, configuration.setpoint_temperatura_tavolo);
GETTERNSETTER(setpoint_temperatura_bracciolo, configuration.setpoint_temperatura_bracciolo);
GETTERNSETTER(velocita_soffio, configuration.velocita_soffio);
GETTERNSETTER(velocita_aspirazione, configuration.velocita_aspirazione);
GETTERNSETTER(numero_sonde, configuration.numero_sonde);
GETTERNSETTER(boiler_adc_threshold, configuration.boiler_adc_threshold);
GETTERNSETTER(tavolo_on, run.tavolo_on);
GETTERNSETTER(bracciolo_on, run.bracciolo_on);
GETTERNSETTER(boiler_on, run.boiler_on);
GETTERNSETTER(pompa_on, run.pompa_on);
GETTERNSETTER(richiesta_temperatura_tavolo, run.richiesta_temperatura_tavolo);
GETTERNSETTER(richiesta_temperatura_bracciolo, run.richiesta_temperatura_bracciolo);
GETTERNSETTER(test_percentage_suction, run.test_percentage_suction);
GETTERNSETTER(test_percentage_blow, run.test_percentage_blow);

TOGGLER(luce, run.luce);
TOGGLER(gun, run.gun_state);
TOGGLER(ferro_1, run.ferro_1);
TOGGLER(ferro_2, run.ferro_2);
TOGGLER(richiesta_temperatura_tavolo, run.richiesta_temperatura_tavolo);
TOGGLER(richiesta_temperatura_bracciolo, run.richiesta_temperatura_bracciolo);
TOGGLER(richiesta_boiler, run.richiesta_boiler);

#endif
