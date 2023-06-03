#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "uart_t.h"
#include "driver/gpio.h"

static const char *TAG = "uart_events";

static QueueHandle_t uart0_queue;
static uart_handle_t p_uart_handle =NULL;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for(;;) {
        if(xQueueReceive(uart0_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            switch(event.type) {
                case UART_DATA: // nhận được dữ liệu
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);// Giups đọc dữ liệu từ ring_buffer vào trong dtmp (bộ đệm)
                    vTaskDelay(200/portTICK_PERIOD_MS);
                    p_uart_handle(dtmp, event.size);
                    break;
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                 case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;          
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}
void uart_init_cmd(void)
{
     ESP_LOGI(TAG, "UART.......................");
    esp_log_level_set(TAG, ESP_LOG_INFO);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1, 
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);// pin 1, 3 là UART0 pin
    //Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 21, NULL);
}
void uart_set_callback(void *cb)
{
    if(cb){
        p_uart_handle=cb;
    }
}
void uart_put(char *dta, int len){
    uart_write_bytes(EX_UART_NUM, (char*)dta, len);
}