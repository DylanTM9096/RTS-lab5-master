#include "I2C.h"


static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t slave_dev;

static QueueHandle_t i2c_output_queue = NULL; // Queue for sending i2c value

void i2c_send_queue(QueueHandle_t queue1) {
    i2c_output_queue = queue1;
}

void i2c_master_init(void){
    ESP_LOGI(TAG_I2C, "Initializing I2C master...");

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SLAVE_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &slave_dev));
}

void i2c_receive_adc(void *pvParameters) {
    uint8_t send[] = {0x01};
    uint8_t data[2];
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Wait for start fropm timer
        if (i2c_master_transmit_receive(slave_dev, send, sizeof(send), data, sizeof(data), -1) == ESP_OK) {
            int val = (data[0] << 8) | data[1];
            ESP_LOGI(TAG_I2C, "Received ADC: %d", val);

            if (val >= 0) {
                xQueueSend(i2c_output_queue, &val, 0);
            }
        }
    }
}