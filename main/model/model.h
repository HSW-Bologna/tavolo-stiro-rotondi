#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>


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


#define NUM_INPUTS      5
#define NUM_SPEED_STEPS 8


typedef enum {
    FOTOCELLULA_SX = 0,
    FOTOCELLULA_DX,
} fotocellula_t;


typedef enum {
    LIQUID_LEVEL_1 = 0,
    LIQUID_LEVEL_2,
    LIQUID_LEVEL_3,
} liquid_level_t;


typedef struct {
    struct {
        uint8_t  fotocellula;
        uint16_t language;
        uint16_t setpoint_temperatura_tavolo;
        uint16_t setpoint_temperatura_bracciolo;
        uint16_t max_temperatura_tavolo;
        uint16_t max_temperatura_bracciolo;
        uint8_t  velocita_soffio;
        uint8_t  percentuali_soffio[NUM_SPEED_STEPS];
        uint16_t isteresi_tavolo;
        uint16_t isteresi_bracciolo;
        uint8_t  numero_sonde;
    } configuration;

    struct {
        uint8_t        luce;
        uint8_t        test;
        uint8_t        ferro_1;
        uint8_t        ferro_2;
        uint8_t        soffio_on;
        uint8_t        aspirazione_on;
        uint8_t        boiler_on;
        uint8_t        pompa_on;
        uint8_t        richiesta_temperatura_tavolo;
        uint8_t        richiesta_temperatura_bracciolo;
        uint8_t        richiesta_boiler;
        uint8_t        tavolo_on;
        uint8_t        bracciolo_on;
        liquid_level_t liquid_level;
        int            temperatura_tavolo;
        int            temperatura_bracciolo;
        uint16_t       adc_level_1;
        uint16_t       adc_level_2;
    } run;

    struct {
        uint8_t  inputs[NUM_INPUTS];
        uint16_t adc_ptc_1;
        uint16_t adc_ptc_2;
    } test;
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

GETTERNSETTER(test, run.test);

GETTER(luce, run.luce);
GETTER(ferro_1, run.ferro_1);
GETTER(ferro_2, run.ferro_2);
GETTER(fotocellula, configuration.fotocellula);

GETTER(max_temperatura_tavolo, configuration.max_temperatura_tavolo);
GETTER(max_temperatura_bracciolo, configuration.max_temperatura_bracciolo);
GETTER(isteresi_tavolo, configuration.isteresi_tavolo);
GETTER(isteresi_bracciolo, configuration.isteresi_bracciolo);
GETTER(richiesta_boiler, run.richiesta_boiler);

GETTERNSETTER(soffio_on, run.soffio_on);
GETTERNSETTER(aspirazione_on, run.aspirazione_on);
GETTERNSETTER(setpoint_temperatura_tavolo, configuration.setpoint_temperatura_tavolo);
GETTERNSETTER(setpoint_temperatura_bracciolo, configuration.setpoint_temperatura_bracciolo);
GETTERNSETTER(velocita_soffio, configuration.velocita_soffio);
GETTERNSETTER(numero_sonde, configuration.numero_sonde);
GETTERNSETTER(temperatura_tavolo, run.temperatura_tavolo);
GETTERNSETTER(temperatura_bracciolo, run.temperatura_bracciolo);
GETTERNSETTER(tavolo_on, run.tavolo_on);
GETTERNSETTER(bracciolo_on, run.bracciolo_on);
GETTERNSETTER(boiler_on, run.boiler_on);
GETTERNSETTER(pompa_on, run.pompa_on);
GETTERNSETTER(richiesta_temperatura_tavolo, run.richiesta_temperatura_tavolo);
GETTERNSETTER(richiesta_temperatura_bracciolo, run.richiesta_temperatura_bracciolo);
GETTERNSETTER(liquid_level, run.liquid_level);
GETTERNSETTER(adc_ptc_1, test.adc_ptc_1);
GETTERNSETTER(adc_ptc_2, test.adc_ptc_2);
GETTERNSETTER(adc_level_1, run.adc_level_1);
GETTERNSETTER(adc_level_2, run.adc_level_2);

TOGGLER(luce, run.luce);
TOGGLER(ferro_1, run.ferro_1);
TOGGLER(ferro_2, run.ferro_2);
TOGGLER(fotocellula, configuration.fotocellula);
TOGGLER(richiesta_temperatura_tavolo, run.richiesta_temperatura_tavolo);
TOGGLER(richiesta_temperatura_bracciolo, run.richiesta_temperatura_bracciolo);
TOGGLER(richiesta_boiler, run.richiesta_boiler);

#endif