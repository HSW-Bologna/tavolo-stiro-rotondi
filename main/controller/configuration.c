#include "configuration.h"
#include "peripherals/storage.h"
#include "model/model.h"
#include "gel/data_structures/watcher.h"
#include "utils/utils.h"


#define NUM_WATCHED_PARAMETERS 17


static const char *STEP_SOFFIO_KEY                = "VELSOFFIO";
static const char *SETPOINT_TAVOLO_KEY            = "SPTAVOLO";
static const char *SETPOINT_BRACCIOLO_KEY         = "SPBRACCIOLO";
static const char *ADC_BOILER_LEVEL_KEY           = "ADBOILVL";
static const char *BOILER_HYSTERESIS_KEY          = "BOILHYST";
static const char *MACHINE_MODEL_KEY              = "MACMODEL";
static const char *SECOND_IRON_KEY                = "SECONDIRON";
static const char *HEATED_ARM_KEY                 = "HEATEDARM";
static const char *STEAM_GUN_KEY                  = "STEAMGUN";
static const char *LIGHT_KEY                      = "LIGHT";
static const char *HEIGHT_REGULATION_KEY          = "HEIGHTREG";
static const char *HEIGHT_REGULATION_PRESET_1_KEY = "HEIGHTREG1";
static const char *HEIGHT_REGULATION_PRESET_2_KEY = "HEIGHTREG2";
static const char *HEIGHT_REGULATION_PRESET_3_KEY = "HEIGHTREG3";
static const char *SELECTED_HEIGHT_PRESET_KEY     = "SELHEIGHT";
static const char *BOILER_KEY                     = "BOILER";
static const char *FAN_CONFIG_KEY                 = "FANCONFIG";


static watcher_t watched_parameters[NUM_WATCHED_PARAMETERS + 1] = {0};


void configuration_init(model_t *pmodel) {
    size_t i = 0;

    watched_parameters[i++] =
        WATCHER_DELAYED(&pmodel->configuration.velocita_soffio, storage_save_uint8, STEP_SOFFIO_KEY, 2000);
    watched_parameters[i++] = WATCHER_DELAYED(&pmodel->configuration.setpoint_temperatura_tavolo, storage_save_uint16,
                                              SETPOINT_TAVOLO_KEY, 2000);
    watched_parameters[i++] = WATCHER_DELAYED(&pmodel->configuration.setpoint_temperatura_bracciolo,
                                              storage_save_uint16, SETPOINT_BRACCIOLO_KEY, 2000);
    watched_parameters[i++] =
        WATCHER(&pmodel->configuration.boiler_adc_threshold, storage_save_uint16, ADC_BOILER_LEVEL_KEY);
    watched_parameters[i++] =
        WATCHER(&pmodel->configuration.isteresi_caldaia, storage_save_uint16, BOILER_HYSTERESIS_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.second_iron_enabled, storage_save_uint8, SECOND_IRON_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.heated_arm_enabled, storage_save_uint8, HEATED_ARM_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.steam_gun_enabled, storage_save_uint8, STEAM_GUN_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.light_enabled, storage_save_uint8, LIGHT_KEY);
    watched_parameters[i++] =
        WATCHER(&pmodel->configuration.height_regulation, storage_save_uint8, HEIGHT_REGULATION_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.height_regulation_presets[0], storage_save_uint16,
                                      HEIGHT_REGULATION_PRESET_1_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.height_regulation_presets[1], storage_save_uint16,
                                      HEIGHT_REGULATION_PRESET_2_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.height_regulation_presets[2], storage_save_uint16,
                                      HEIGHT_REGULATION_PRESET_3_KEY);
    watched_parameters[i++] = WATCHER_DELAYED(&pmodel->configuration.selected_height_preset, storage_save_uint16,
                                              SELECTED_HEIGHT_PRESET_KEY, 2000);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.boiler_enabled, storage_save_uint8, BOILER_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.fan_config, storage_save_uint8, FAN_CONFIG_KEY);
    watched_parameters[i++] = WATCHER(&pmodel->configuration.machine_model, storage_save_uint8, MACHINE_MODEL_KEY);
    assert(i == NUM_WATCHED_PARAMETERS);
    watched_parameters[i] = WATCHER_NULL;
    watcher_list_init(watched_parameters);

    storage_load_uint8(&pmodel->configuration.machine_model, (char *)MACHINE_MODEL_KEY);
    storage_load_uint8(&pmodel->configuration.second_iron_enabled, (char *)SECOND_IRON_KEY);
    storage_load_uint8(&pmodel->configuration.heated_arm_enabled, (char *)HEATED_ARM_KEY);
    storage_load_uint8(&pmodel->configuration.steam_gun_enabled, (char *)STEAM_GUN_KEY);
    storage_load_uint8(&pmodel->configuration.light_enabled, (char *)LIGHT_KEY);
    storage_load_uint8(&pmodel->configuration.height_regulation, (char *)HEIGHT_REGULATION_KEY);
    storage_load_uint16(&pmodel->configuration.height_regulation_presets[0], (char *)HEIGHT_REGULATION_PRESET_1_KEY);
    storage_load_uint16(&pmodel->configuration.height_regulation_presets[1], (char *)HEIGHT_REGULATION_PRESET_2_KEY);
    storage_load_uint16(&pmodel->configuration.height_regulation_presets[2], (char *)HEIGHT_REGULATION_PRESET_3_KEY);
    storage_load_uint16(&pmodel->configuration.selected_height_preset, (char *)SELECTED_HEIGHT_PRESET_KEY);
    storage_load_uint8(&pmodel->configuration.boiler_enabled, (char *)BOILER_KEY);
    storage_load_uint8(&pmodel->configuration.velocita_soffio, (char *)STEP_SOFFIO_KEY);
    storage_load_uint8(&pmodel->configuration.fan_config, (char *)FAN_CONFIG_KEY);
    storage_load_uint16(&pmodel->configuration.setpoint_temperatura_tavolo, (char *)SETPOINT_TAVOLO_KEY);
    storage_load_uint16(&pmodel->configuration.setpoint_temperatura_bracciolo, (char *)SETPOINT_BRACCIOLO_KEY);
    storage_load_uint16(&pmodel->configuration.boiler_adc_threshold, (char *)ADC_BOILER_LEVEL_KEY);
    storage_load_uint16(&pmodel->configuration.isteresi_caldaia, (char *)BOILER_HYSTERESIS_KEY);
}


void configuration_process_parameters(void) {
    watcher_process_changes(watched_parameters, get_millis());
}
