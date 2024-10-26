#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lightmodbus/lightmodbus.h"
#include "modbus.h"
#include "esp_log.h"
#include "peripherals/rs485.h"
#include "model/model.h"


#define MODBUS_RESPONSE_03_LEN(data_len) (5 + data_len * 2)
#define MODBUS_RESPONSE_05_LEN           8
#define MODBUS_MESSAGE_QUEUE_SIZE        32
#define MODBUS_TIMEOUT                   40
#define MODBUS_MAX_PACKET_SIZE           256
#define MODBUS_COMMUNICATION_ATTEMPTS    5

#define INPUT_REGISTER_FIRMWARE_VERSION 1
#define INPUT_REGISTER_INPUTS           1

#define HOLDING_REGISTER_RELAYS      0
#define HOLDING_REGISTER_FAN_CONTROL 1

#define MINION_ADDR 1


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
    uint16_t start;
    void    *pointer;
} master_context_t;


static void        modbus_task(void *args);
static ModbusError exception_callback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                      ModbusExceptionCode code);
static ModbusError data_callback(const ModbusMaster *master, const ModbusDataCallbackArgs *args);
static int write_holding_registers(ModbusMaster *master, uint8_t address, uint16_t starting_address, uint16_t *data,
                                   size_t num);
static int read_holding_registers(ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                  uint16_t count);
static int read_input_registers(ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                uint16_t count);


static const char   *TAG       = "Modbus";
static QueueHandle_t messageq  = NULL;
static QueueHandle_t responseq = NULL;


void modbus_init(void) {
    static StaticQueue_t static_queue1;
    static uint8_t       queue_buffer1[MODBUS_MESSAGE_QUEUE_SIZE * sizeof(struct task_message)] = {0};
    messageq =
        xQueueCreateStatic(MODBUS_MESSAGE_QUEUE_SIZE, sizeof(struct task_message), queue_buffer1, &static_queue1);

    static StaticQueue_t static_queue2;
    static uint8_t       queue_buffer2[MODBUS_MESSAGE_QUEUE_SIZE * sizeof(modbus_response_t)] = {0};
    responseq = xQueueCreateStatic(MODBUS_MESSAGE_QUEUE_SIZE, sizeof(modbus_response_t), queue_buffer2, &static_queue2);

    xTaskCreate(modbus_task, TAG, 512 * 6, NULL, 5, NULL);
}


void modbus_read_state(void) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_READ_STATE};
    xQueueSend(messageq, &msg, 0);
}


void modbus_write_outputs(uint16_t relays, uint8_t percentage_suction, uint8_t percentage_blow) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_WRITE_OUTPUTS,
                               .as  = {
                                    .outputs =
                                       {
                                            .relays             = relays,
                                            .percentage_suction = percentage_suction,
                                            .percentage_blow    = percentage_blow,
                                       },
                               }};
    xQueueSend(messageq, &msg, 0);
}


void modbus_read_firmware_version(void) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_READ_FW_VERSION};
    xQueueSend(messageq, &msg, 0);
}


uint8_t modbus_get_response(modbus_response_t *response) {
    return xQueueReceive(responseq, response, 0);
}


static void modbus_task(void *args) {
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
    struct task_message message = {0};

    ESP_LOGI(TAG, "Task starting");

    for (;;) {
        if (xQueueReceive(messageq, &message, pdMS_TO_TICKS(100))) {
            switch (message.tag) {
                case TASK_MESSAGE_TAG_READ_STATE: {
                    modbus_response_t response  = {.tag = MODBUS_RESPONSE_TAG_READ_STATE, .error = 0};
                    uint16_t          values[7] = {};
                    if (read_input_registers(&master, values, MINION_ADDR, INPUT_REGISTER_INPUTS,
                                             sizeof(values) / sizeof(values[0]))) {
                        response.error = 1;
                    } else {
                        response.as.state.inputs_map          = values[0];
                        response.as.state.liquid_levels[0]    = values[1];
                        response.as.state.liquid_levels[1]    = values[2];
                        response.as.state.ptc_adcs[0]         = values[3];
                        response.as.state.ptc_adcs[1]         = values[4];
                        response.as.state.ptc_temperatures[0] = values[5];
                        response.as.state.ptc_temperatures[1] = values[6];
                    }
                    xQueueSend(responseq, &response, portMAX_DELAY);
                    break;
                }
                case TASK_MESSAGE_TAG_WRITE_OUTPUTS: {
                    modbus_response_t response  = {.tag = MODBUS_RESPONSE_TAG_OK, .error = 0};
                    uint16_t          values[2] = {
                        message.as.outputs.relays,
                        message.as.outputs.percentage_suction | (message.as.outputs.percentage_blow << 8),
                    };

                    ESP_LOGI(TAG, "Relays %X", message.as.outputs.relays);

                    if (write_holding_registers(&master, MINION_ADDR, HOLDING_REGISTER_RELAYS, values,
                                                sizeof(values) / sizeof(values[0]))) {
                        response.error = 1;
                    }
                    xQueueSend(responseq, &response, portMAX_DELAY);
                    break;
                }

                case TASK_MESSAGE_TAG_READ_FW_VERSION: {
                    modbus_response_t response = {.tag = MODBUS_RESPONSE_TAG_FIRMWARE_VERSION, .error = 0};

                    uint16_t values[2] = {0};
                    if (read_input_registers(&master, values, MINION_ADDR, INPUT_REGISTER_FIRMWARE_VERSION, 2)) {
                        response.error = 1;
                    } else {
                        response.as.firmware_version.version_major = (values[0] >> 8) & 0xFF;
                        response.as.firmware_version.version_minor = values[0] & 0xFF;
                        response.as.firmware_version.version_patch = values[1] & 0xFF;
                    }

                    xQueueSend(responseq, &response, portMAX_DELAY);
                    break;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT / 2));
        }
    }

    vTaskDelete(NULL);
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
    ESP_LOGI(TAG, "Received exception (function %d) from slave %d code %d", function, address, code);

    return MODBUS_OK;
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

        int len = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                         buffer, len);

        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Write holding registers for %i error (%i): %i %i", address, len, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

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

        int len = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                         buffer, len);

        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Read holding registers for %i error (%i): %i %i", address, len, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

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

        int len = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                         buffer, len);

        if (!modbusIsOk(err)) {
            // ESP_LOGW(TAG, "Read input registers for %i error (%i): %i %i", address, len, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

    if (res) {
        // ESP_LOGW(TAG, "ERROR!");
    } else {
        ESP_LOGD(TAG, "Success");
    }

    return res;
}
