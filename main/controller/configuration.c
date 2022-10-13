#include "configuration.h"
#include "peripherals/storage.h"
#include "model/model.h"
#include "gel/data_structures/watcher.h"
#include "utils/utils.h"


#define NUM_WATCHED_PARAMETERS 5


static const char *FOTOCELLULA_KEY        = "FOTOCELL";
static const char *STEP_SOFFIO_KEY        = "VELSOFFIO";
static const char *STEP_ASPIRAZIONE_KEY   = "VELASPIRAZ";
static const char *SETPOINT_TAVOLO_KEY    = "SPTAVOLO";
static const char *SETPOINT_BRACCIOLO_KEY = "SPBRACCIOLO";


static watcher_t watched_parameters[NUM_WATCHED_PARAMETERS + 1] = {0};


void configuration_init(model_t *pmodel) {
    size_t i = 0;

    watched_parameters[i++] = WATCHER(&pmodel->configuration.fotocellula, storage_save_uint8, FOTOCELLULA_KEY);
    watched_parameters[i++] =
        WATCHER_DELAYED(&pmodel->configuration.velocita_soffio, storage_save_uint8, STEP_SOFFIO_KEY, 2000);
    watched_parameters[i++] =
        WATCHER_DELAYED(&pmodel->configuration.velocita_aspirazione, storage_save_uint8, STEP_ASPIRAZIONE_KEY, 2000);
    watched_parameters[i++] =
        WATCHER_DELAYED(&pmodel->configuration.setpoint_temperatura_tavolo, storage_save_uint16, SETPOINT_TAVOLO_KEY, 2000);
    watched_parameters[i++] =
        WATCHER_DELAYED(&pmodel->configuration.setpoint_temperatura_bracciolo, storage_save_uint16, SETPOINT_BRACCIOLO_KEY, 2000);
    assert(i == NUM_WATCHED_PARAMETERS);
    watched_parameters[i] = WATCHER_NULL;
    watcher_list_init(watched_parameters);

    storage_load_uint8(&pmodel->configuration.fotocellula, (char *)FOTOCELLULA_KEY);
    storage_load_uint8(&pmodel->configuration.velocita_soffio, (char *)STEP_SOFFIO_KEY);
    storage_load_uint8(&pmodel->configuration.velocita_aspirazione, (char *)STEP_ASPIRAZIONE_KEY);
    storage_load_uint16(&pmodel->configuration.setpoint_temperatura_tavolo, (char *)SETPOINT_TAVOLO_KEY);
    storage_load_uint16(&pmodel->configuration.setpoint_temperatura_bracciolo, (char *)SETPOINT_BRACCIOLO_KEY);
}


void configuration_process_parameters(void) {
    watcher_process_changes(watched_parameters, get_millis());
}