#include <stdbool.h>
#include <assert.h>
#include "gel/timer/timecheck.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "lightmodbus/lightmodbus.h"
#include "minion.h"
#include "esp_log.h"
#include "peripherals/rs485.h"
#include "model/model.h"
#include "utils/utils.h"


#define MODBUS_RESPONSE_03_LEN(data_len) (5 + data_len * 2)
#define MODBUS_RESPONSE_05_LEN           8
#define MODBUS_RESPONSE_06_LEN           8
#define MINION_MESSAGE_QUEUE_SIZE        32
#define MINION_TIMEOUT                   40
#define MODBUS_MAX_PACKET_SIZE           256
#define MINION_COMMUNICATION_ATTEMPTS    5

#define INPUT_REGISTER_FIRMWARE_VERSION 1
#define INPUT_REGISTER_INPUTS           1

#define HOLDING_REGISTER_RELAYS      0
#define HOLDING_REGISTER_FAN_CONTROL 1

#define HEIGHT_REGULATOR_ADDR 247
#define MINION_ADDR           1
#define ADJUSTABLE_LEGS_ADDR  247

#define ADJUSTABLE_LEGS_HR_BAUD_RATE 0x1115


typedef enum {
    TASK_MESSAGE_TAG_READ_FW_VERSION,
    TASK_MESSAGE_TAG_READ_STATE,
    TASK_MESSAGE_TAG_WRITE_OUTPUTS,
} task_message_tag_t;


struct __attribute__((packed)) task_message {
    task_message_tag_t tag;
    union {
        struct {
            uint16_t relays;
            uint8_t  percentage_suction;
            uint8_t  percentage_blow;
        } outputs;
    } as;
};


typedef struct {
    uint16_t exception;
    uint16_t start;
    void    *pointer;
} master_context_t;


static void        minion_task(void *args);
static ModbusError exception_callback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                      ModbusExceptionCode code);
static ModbusError data_callback(const ModbusMaster *master, const ModbusDataCallbackArgs *args);
static int  write_holding_registers(ModbusMaster *master, uint8_t address, uint16_t starting_address, uint16_t *data,
                                    size_t num);
static int  read_holding_registers(ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                   uint16_t count);
static int  write_holding_register(ModbusMaster *master, uint8_t address, uint16_t register_address, uint16_t data);
static int  read_input_registers(ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                 uint16_t count);
static void machine_manage(ModbusMaster *master);
static void adjustable_legs_manage(ModbusMaster *master);


static const char *TAG = "Modbus";
static struct {
    SemaphoreHandle_t sem;
    QueueHandle_t     messageq;
    QueueHandle_t     responseq;

    uint8_t legs_communication_error;
    uint8_t enabled;
    uint8_t new;
    uint16_t position;
} state = {0};


void minion_init(void) {
    static StaticQueue_t static_queue1;
    static uint8_t       queue_buffer1[MINION_MESSAGE_QUEUE_SIZE * sizeof(struct task_message)] = {0};
    state.messageq =
        xQueueCreateStatic(MINION_MESSAGE_QUEUE_SIZE, sizeof(struct task_message), queue_buffer1, &static_queue1);

    static StaticQueue_t static_queue2;
    static uint8_t       queue_buffer2[MINION_MESSAGE_QUEUE_SIZE * sizeof(minion_response_t)] = {0};
    state.responseq =
        xQueueCreateStatic(MINION_MESSAGE_QUEUE_SIZE, sizeof(minion_response_t), queue_buffer2, &static_queue2);

    static StaticSemaphore_t static_sem;
    state.sem = xSemaphoreCreateMutexStatic(&static_sem);

    xTaskCreate(minion_task, TAG, 512 * 6, NULL, 5, NULL);
}


void minion_read_state(void) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_READ_STATE};
    xQueueSend(state.messageq, &msg, 0);
}


void minion_write_outputs(uint16_t relays, uint8_t percentage_suction, uint8_t percentage_blow) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_WRITE_OUTPUTS,
                               .as  = {
                                    .outputs =
                                       {
                                            .relays             = relays,
                                            .percentage_suction = percentage_suction,
                                            .percentage_blow    = percentage_blow,
                                       },
                               }};
    xQueueSend(state.messageq, &msg, 0);
}


void minion_read_firmware_version(void) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_READ_FW_VERSION};
    xQueueSend(state.messageq, &msg, 0);
}


uint8_t minion_get_response(minion_response_t *response) {
    return xQueueReceive(state.responseq, response, 0);
}


void minion_height_regulator_update(uint8_t enabled, uint16_t position) {
    xSemaphoreTake(state.sem, portMAX_DELAY);
    state.new      = 1;
    state.enabled  = enabled;
    state.position = position;
    xSemaphoreGive(state.sem);
}


static void minion_task(void *args) {
    (void)args;
    ModbusMaster    master;
    ModbusErrorInfo err = modbusMasterInit(&master,
                                           data_callback,              // Callback for handling incoming data
                                           exception_callback,         // Exception callback (optional)
                                           modbusDefaultAllocator,     // Memory allocator used to allocate request
                                           modbusMasterDefaultFunctions,        // Set of supported functions
                                           modbusMasterDefaultFunctionCount     // Number of supported functions
    );

    // Check for errors
    assert(modbusIsOk(err) && "modbusMasterInit() failed");

    ESP_LOGI(TAG, "Task starting");

    for (;;) {
        adjustable_legs_manage(&master);
        machine_manage(&master);
    }

    vTaskDelete(NULL);
}


static void machine_manage(ModbusMaster *master) {
    struct task_message message = {0};

    if (xQueueReceive(state.messageq, &message, pdMS_TO_TICKS(10))) {
        switch (message.tag) {
            case TASK_MESSAGE_TAG_READ_STATE: {
                minion_response_t response  = {.tag = MINION_RESPONSE_TAG_READ_STATE};
                uint16_t          values[7] = {};
                if (read_input_registers(master, values, MINION_ADDR, INPUT_REGISTER_INPUTS,
                                         sizeof(values) / sizeof(values[0]))) {
                    response.tag = MINION_RESPONSE_TAG_ERROR;
                } else {
                    response.as.state.inputs_map          = values[0];
                    response.as.state.liquid_levels[0]    = values[1];
                    response.as.state.liquid_levels[1]    = values[2];
                    response.as.state.ptc_adcs[0]         = values[3];
                    response.as.state.ptc_adcs[1]         = values[4];
                    response.as.state.ptc_temperatures[0] = values[5];
                    response.as.state.ptc_temperatures[1] = values[6];
                }
                xQueueSend(state.responseq, &response, portMAX_DELAY);
                break;
            }
            case TASK_MESSAGE_TAG_WRITE_OUTPUTS: {
                uint16_t values[2] = {
                    message.as.outputs.relays,
                    message.as.outputs.percentage_suction | (message.as.outputs.percentage_blow << 8),
                };

                if (write_holding_registers(master, MINION_ADDR, HOLDING_REGISTER_RELAYS, values,
                                            sizeof(values) / sizeof(values[0]))) {
                    minion_response_t response = {.tag = MINION_RESPONSE_TAG_ERROR};
                    xQueueSend(state.responseq, &response, portMAX_DELAY);
                }
                break;
            }

            case TASK_MESSAGE_TAG_READ_FW_VERSION: {
                minion_response_t response = {.tag = MINION_RESPONSE_TAG_FIRMWARE_VERSION};

                uint16_t values[2] = {0};
                if (read_input_registers(master, values, MINION_ADDR, INPUT_REGISTER_FIRMWARE_VERSION, 2)) {
                    response = (minion_response_t){.tag = MINION_RESPONSE_TAG_ERROR};
                } else {
                    response.as.firmware_version.version_major = (values[0] >> 8) & 0xFF;
                    response.as.firmware_version.version_minor = values[0] & 0xFF;
                    response.as.firmware_version.version_patch = values[1] & 0xFF;
                }

                xQueueSend(state.responseq, &response, portMAX_DELAY);
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(MINION_TIMEOUT / 2));
    }
}


static void adjustable_legs_manage(ModbusMaster *master) {
    static unsigned long ts                 = 0;
    static unsigned long legs_config_ts     = 0;
    static bool          legs_first_attempt = true;

    xSemaphoreTake(state.sem, portMAX_DELAY);
    uint8_t new       = state.new;
    uint8_t  enabled  = state.enabled;
    uint16_t position = state.position;
    xSemaphoreGive(state.sem);
    if (enabled) {
        if (new &&is_expired(ts, get_millis(), 100)) {
            if (write_holding_register(master, HEIGHT_REGULATOR_ADDR, 0x1002, 0x0000)) {
                state.legs_communication_error = 1;
            } else {
                vTaskDelay(pdMS_TO_TICKS(20));
                if (write_holding_register(master, HEIGHT_REGULATOR_ADDR, 0x1000, position)) {
                    state.legs_communication_error = 1;
                } else {
                    vTaskDelay(pdMS_TO_TICKS(10));
                    if (write_holding_register(master, HEIGHT_REGULATOR_ADDR, 0x1002, 0x000E)) {
                        state.legs_communication_error = 1;
                    } else {
                        state.legs_communication_error = 0;
                    }
                }
            }

            if (state.legs_communication_error) {
                ESP_LOGW(TAG, "Failed to setup adjustable height");
                minion_response_t response = {
                    .tag    = MINION_RESPONSE_TAG_ERROR,
                    .minion = MINION_ADJUSTABLE_LEGS,
                };
                xQueueSend(state.responseq, &response, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "Height adjusted successfully to %i!", position);
                xSemaphoreTake(state.sem, portMAX_DELAY);
                state.new = 0;     // Clear the position request, otherwise try again
                xSemaphoreGive(state.sem);
            }

            ts = get_millis();
        }

        if (state.legs_communication_error && (legs_first_attempt || is_expired(legs_config_ts, get_millis(), 10000))) {

            rs485_set_baudrate(9600);
            uint16_t baudrate = 3;

            if (write_holding_registers(master, ADJUSTABLE_LEGS_ADDR, ADJUSTABLE_LEGS_HR_BAUD_RATE, &baudrate, 1)) {
                ESP_LOGW(TAG, "Failed to setup baudrate for adjustable legs");
                minion_response_t response = {
                    .tag    = MINION_RESPONSE_TAG_ERROR,
                    .minion = MINION_ADJUSTABLE_LEGS,
                };
                xQueueSend(state.responseq, &response, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "Baudrate for adjustable legs setup");
                state.legs_communication_error = 0;
            }
            rs485_set_baudrate(115200);

            legs_first_attempt = false;
            legs_config_ts     = get_millis();
        }
    }
}


static ModbusError data_callback(const ModbusMaster *master, const ModbusDataCallbackArgs *args) {
    master_context_t *ctx = modbusMasterGetUserPointer(master);

    if (ctx != NULL) {
        switch (args->type) {
            case MODBUS_HOLDING_REGISTER: {
                uint16_t *buffer                 = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_DISCRETE_INPUT: {
                uint8_t *buffer                  = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_INPUT_REGISTER: {
                uint16_t *buffer                 = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_COIL: {
                uint8_t *buffer                  = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }
        }
    }

    return MODBUS_OK;
}


static ModbusError exception_callback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                      ModbusExceptionCode code) {
    master_context_t *ctx = modbusMasterGetUserPointer(master);
    if (ctx) {
        ctx->exception = code;
    }
    ESP_LOGI(TAG, "Received exception (function %d) from slave %d code %d", function, address, code);

    return MODBUS_OK;
}


static int write_holding_register(ModbusMaster *master, uint8_t address, uint16_t register_address, uint16_t data) {
    uint8_t buffer[MODBUS_MAX_PACKET_SIZE] = {0};
    int     res                            = 0;
    size_t  counter                        = 0;

    rs485_flush();

    master_context_t ctx = {};
    modbusMasterSetUserPointer(master, &ctx);

    do {
        ctx.exception = 0;
        res           = 0;

        ModbusErrorInfo err = modbusBuildRequest06RTU(master, address, register_address, data);
        assert(modbusIsOk(err));
        rs485_write((uint8_t *)modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = rs485_read(buffer, MODBUS_RESPONSE_06_LEN, pdMS_TO_TICKS(MINION_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                         buffer, len);

        if (!modbusIsOk(err) || ctx.exception != MODBUS_EXCEP_NONE) {
            ESP_LOGW(TAG, "Write holding register for %i error (%i): %i %i %i", address, len, err.source, err.error,
                     ctx.exception);
            res = -1;
            vTaskDelay(pdMS_TO_TICKS(MINION_TIMEOUT));
        }
    } while (res && ++counter < MINION_COMMUNICATION_ATTEMPTS);

    if (res) {
        ESP_LOGW(TAG, "ERROR!");
    } else {
        ESP_LOGD(TAG, "Success");
    }

    return res;
}



static int write_holding_registers(ModbusMaster *master, uint8_t address, uint16_t starting_address, uint16_t *data,
                                   size_t num) {
    uint8_t buffer[MODBUS_MAX_PACKET_SIZE] = {0};
    int     res                            = 0;
    size_t  counter                        = 0;

    rs485_flush();

    do {
        res                 = 0;
        ModbusErrorInfo err = modbusBuildRequest16RTU(master, address, starting_address, num, data);
        assert(modbusIsOk(err));
        rs485_write((uint8_t *)modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MINION_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                         buffer, len);

        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Write holding registers for %i error (%i): %i %i", address, len, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MINION_TIMEOUT));
        }
    } while (res && ++counter < MINION_COMMUNICATION_ATTEMPTS);

    if (res) {
        ESP_LOGW(TAG, "ERROR!");
    } else {
        ESP_LOGD(TAG, "Success");
    }

    return res;
}


static int read_holding_registers(ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                  uint16_t count) {
    ModbusErrorInfo err;
    int             res                            = 0;
    size_t          counter                        = 0;
    uint8_t         buffer[MODBUS_MAX_PACKET_SIZE] = {0};

    rs485_flush();

    master_context_t ctx = {.pointer = registers, .start = start};
    if (registers == NULL) {
        modbusMasterSetUserPointer(master, NULL);
    } else {
        modbusMasterSetUserPointer(master, &ctx);
    }

    do {
        res = 0;
        err = modbusBuildRequest03RTU(master, address, start, count);
        assert(modbusIsOk(err));

        rs485_write((uint8_t *)modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MINION_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                         buffer, len);

        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Read holding registers for %i error (%i): %i %i", address, len, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MINION_TIMEOUT));
        }
    } while (res && ++counter < MINION_COMMUNICATION_ATTEMPTS);

    return res;
}


static int read_input_registers(ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                uint16_t count) {
    ModbusErrorInfo err;
    int             res                            = 0;
    size_t          counter                        = 0;
    uint8_t         buffer[MODBUS_MAX_PACKET_SIZE] = {0};

    master_context_t ctx = {.pointer = registers, .start = start};
    if (registers == NULL) {
        modbusMasterSetUserPointer(master, NULL);
    } else {
        modbusMasterSetUserPointer(master, &ctx);
    }

    rs485_flush();

    do {
        res = 0;
        err = modbusBuildRequest04RTU(master, address, start, count);
        assert(modbusIsOk(err));

        rs485_write((uint8_t *)modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MINION_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                         buffer, len);

        if (!modbusIsOk(err)) {
            // ESP_LOGW(TAG, "Read input registers for %i error (%i): %i %i", address, len, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MINION_TIMEOUT));
        }
    } while (res && ++counter < MINION_COMMUNICATION_ATTEMPTS);

    if (res) {
        // ESP_LOGW(TAG, "ERROR!");
    } else {
        ESP_LOGD(TAG, "Success");
    }

    return res;
}
