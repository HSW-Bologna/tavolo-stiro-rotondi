#include "configuration.h"
#include "peripherals/storage.h"
#include "model/model.h"
#include "gel/data_structures/watcher.h"
#include "utils/utils.h"


#define NUM_WATCHED_PARAMETERS 6


static const char *FOTOCELLULA_KEY        = "FOTOCELL";
static const char *STEP_SOFFIO_KEY        = "VELSOFFIO";
static const char *SETPOINT_TAVOLO_KEY    = "SPTAVOLO";
static const char *SETPOINT_BRACCIOLO_KEY = "SPBRACCIOLO";
static const char *ADC_BOILER_LEVEL_KEY   = "ADBOILVL";
static const char *BOILER_HYSTERESIS_KEY  = "BOILHYST";


static watcher_t watched_parameters[NUM_WATCHED_PARAMETERS + 1] = {0};


void configuration_init(model_t *pmodel) {
    size_t i = 0;

    watched_parameters[i++] = WATCHER(&pmodel->configuration.fotocellula, storage_save_uint8, FOTOCELLULA_KEY);
    watched_parameters[i++] =
        WATCHER_DELAYED(&pmodel->configuration.velocita_soffio, storage_save_uint8, STEP_SOFFIO_KEY, 2000);
    watched_parameters[i++] = WATCHER_DELAYED(&pmodel->configuration.setpoint_temperatura_tavolo, storage_save_uint16,
                                              SETPOINT_TAVOLO_KEY, 2000);
    watched_parameters[i++] = WATCHER_DELAYED(&pmodel->configuration.setpoint_temperatura_bracciolo,
                                              storage_save_uint16, SETPOINT_BRACCIOLO_KEY, 2000);
    watched_parameters[i++] =
        WATCHER_DELAYED(&pmodel->configuration.boiler_adc_threshold, storage_save_uint16, ADC_BOILER_LEVEL_KEY, 2000);
    watched_parameters[i++] =
        WATCHER_DELAYED(&pmodel->configuration.isteresi_caldaia, storage_save_uint16, BOILER_HYSTERESIS_KEY, 2000);
    assert(i == NUM_WATCHED_PARAMETERS);
    watched_parameters[i] = WATCHER_NULL;
    watcher_list_init(watched_parameters);

    storage_load_uint8(&pmodel->configuration.fotocellula, (char *)FOTOCELLULA_KEY);
    storage_load_uint8(&pmodel->configuration.velocita_soffio, (char *)STEP_SOFFIO_KEY);
    storage_load_uint16(&pmodel->configuration.setpoint_temperatura_tavolo, (char *)SETPOINT_TAVOLO_KEY);
    storage_load_uint16(&pmodel->configuration.setpoint_temperatura_bracciolo, (char *)SETPOINT_BRACCIOLO_KEY);
    storage_load_uint16(&pmodel->configuration.boiler_adc_threshold, (char *)ADC_BOILER_LEVEL_KEY);
    storage_load_uint16(&pmodel->configuration.isteresi_caldaia, (char *)BOILER_HYSTERESIS_KEY);
}


void configuration_process_parameters(void) {
    watcher_process_changes(watched_parameters, get_millis());
}