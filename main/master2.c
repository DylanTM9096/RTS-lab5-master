#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "PWM_write.h"
#include "I2C.h"

#define PWM_GPIO GPIO_NUM_26
#define PWM_PERIOD_MS 100

QueueHandle_t pwmQueue;
TimerHandle_t pwmTimer;
TimerHandle_t output;


void poll_adc_and_pwm(TimerHandle_t xTimer) {
    int val = i2c_receive_adc();
    if (val >= 0) {
        xQueueSend(pwmQueue, &val, portMAX_DELAY);
    }
}

void app_main(void) {
    i2c_master_init();

    pwmQueue = xQueueCreate(2, sizeof(int));
    pwm_set_receive_queue(pwmQueue);
    PWM_output_config(PWM_GPIO);

    pwmTimer = xTimerCreate("PWM Timer", pdMS_TO_TICKS(PWM_PERIOD_MS), pdTRUE, NULL, poll_adc_and_pwm);
    if (pwmTimer == NULL || xTimerStart(pwmTimer, 0) != pdPASS) {
        ESP_LOGE("PWM", "Failed to create/start PWM timer");
    }
    output = xTimerCreate("PWM output", pdMS_TO_TICKS(PWM_PERIOD_MS), pdTRUE, NULL, PWM_output_update);
    if (output == NULL || xTimerStart(output, 0) != pdPASS) {
        ESP_LOGE("PWM", "Failed to create/start PWM output timer");
    }
}
