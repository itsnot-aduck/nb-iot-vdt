/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "uart_t.h"
#include "string.h"


//mqtt AT command
const char* TAG = "SIM";
const char AT[] = "AT\r\n";
const char POWERDOWN[]="AT+CPOWD=1\r\n";
const char MQTT_NEW[]="AT+CMQNEW=\"mqtt.innoway.vn\",\"1883\",12000,1024\r\n";
const char MQTT_CONNECT[]="AT+CMQCON=0,3,\"VHT\",600,1,0,\"b07875e3-2175-42ee-a18c-a402d56082c5\",\"Y2z0as2KPG4TUkETFyQKFBlF0eVHmd2l\"\r\n";
const char MQTT_PUBLISH[]="AT+CMQPUB=0,\"messages/831789eb-3431-4e29-8288-a2721c5d0d7b/attributets\",1,0,0,8,\"{\\\"st\\\":1}\"\r\n";
const char MQTT_PUBLISH2[]="AT+CMQPUB=0,\"messages/831789eb-3431-4e29-8288-a2721c5d0d7b/attributets\",1,0,0,8,\"12345678\"\r\n";
const char MQTT_DISCONNECT[]="AT+CMQDISCON=0\r\n";
uint8_t count = 0;
uint8_t count2 = 0;
char req[255];
uint8_t mqttFlag = 0;
uint8_t resetFlag = 0;

QueueHandle_t uartQueue;

static void Led_init(void){
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = 1<< GPIO_NUM_2;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_up_en = 0;
	io_conf.pull_down_en = 1;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf);
}

void nb_iot_mqtt_stop(){
    uart_put(POWERDOWN, (int)strlen(POWERDOWN));
    ESP_LOGI(TAG, "MODULE POWER DOWN");
}

void nb_iot_mqtt_disconnect(){
    uart_put(MQTT_DISCONNECT, (int)strlen(MQTT_DISCONNECT));
    ESP_LOGI(TAG, "MQTT Disconnected");
    nb_iot_mqtt_stop();
}

void nb_iot_mqtt_publish(){
    ESP_LOGI(TAG, "MQTT Publishing");
    ESP_LOGI(TAG, "PUBLISH command: %s", MQTT_PUBLISH);
    uart_put(MQTT_PUBLISH, (int)strlen(MQTT_PUBLISH));
    vTaskDelay(1000/portTICK_PERIOD_MS);
    while(!resetFlag)
    {
        xQueueReceive(uartQueue, req, portMAX_DELAY);
        if(strstr(req,"OK"))
        {
            ESP_LOGI(TAG, "MQTT Published");
            nb_iot_mqtt_disconnect();
            break;
        }
        else
        {
            ESP_LOGI(TAG, "Re-Publishing");
            uart_put(MQTT_PUBLISH, (int)strlen(MQTT_PUBLISH));
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }
}

void nb_iot_mqtt_connect(){
    uart_put(MQTT_CONNECT, (int)strlen(MQTT_CONNECT));
    vTaskDelay(1000/portTICK_PERIOD_MS);
    while(!resetFlag)
    {
        xQueueReceive(uartQueue, req, portMAX_DELAY);
        if (strstr(req, "OK"))
        {
            ESP_LOGI(TAG, "MQTT Connected");
            nb_iot_mqtt_publish();
            break;
        }
        else
        {
            ESP_LOGI(TAG, "Re-connect");
            uart_put(MQTT_CONNECT, (int)strlen(MQTT_CONNECT));
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }
}

void nb_iot_mqtt_start(){
    resetFlag = 0;
    //Wake up the NB IoT
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    gpio_set_level(GPIO_NUM_2, 0);
    ESP_LOGI(TAG, "MODULE POWER UP");
    while(uxQueueMessagesWaiting(uartQueue)){
        xQueueReceive(uartQueue, req, portMAX_DELAY);
    }
    //Start NB IoT
    ESP_LOGI(TAG, "MQTT Started");
    uart_put(MQTT_NEW, (int)strlen(MQTT_NEW));
    vTaskDelay(1000/portTICK_PERIOD_MS);
    while(!resetFlag){
        //áp cho mọi trường hợp nhận - watchdog timer message
        if (xQueueReceive(uartQueue, req, 5000/portTICK_PERIOD_MS)==0)
        {
            resetFlag = 1;
            ESP_LOGI(TAG, "UART receiving error detected!");
            break;
        }
        if (strstr(req, "+CMQNEW: 0"))
        {
            xQueueReceive(uartQueue, req, 5000/portTICK_PERIOD_MS); //Sau cái CMQNEW thì là OK, lấy được 1 tín hiệu coi như ok
            ESP_LOGI(TAG, "MQTT Detected");
            nb_iot_mqtt_connect();
            break;
        }
        else
        {
            ESP_LOGI(TAG, "Re-start");
            uart_put(MQTT_NEW, (int)strlen(MQTT_NEW));
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }
}

void nb_iot_mqtt_reset(){
    count = 0;
    resetFlag = 1;
    //Chạy lỗi, tiến hành khởi động lại 
    ESP_LOGI(TAG, "Reset operation");
    uart_put(POWERDOWN, (int)strlen(POWERDOWN));
    //restart
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(1500/portTICK_PERIOD_MS);
    gpio_set_level(GPIO_NUM_2, 0);
    nb_iot_mqtt_start();
}

void uart_handle(uint8_t *data, uint16_t length){
    ESP_LOGI(TAG, "Received from UART: %s", data);
    xQueueSend(uartQueue, (void*) data, portMAX_DELAY);
}



void timer_handler(){
    vTaskDelay(200/portTICK_PERIOD_MS);
    nb_iot_mqtt_start();
}

void restart_check(void *arg){
    while(1){
        if(resetFlag == 1){
        ESP_LOGI(TAG, "Restart");
        //Báo lỗi 10 lần, tiến hành restart lại chu trình
        nb_iot_mqtt_reset();
        }
    }
}

void app_main(void)
{
    uartQueue = xQueueCreate(10, sizeof(req));
    Led_init();
    uart_init_cmd();
    uart_set_callback(uart_handle);
    //chạy một lần trước hàm khởi động
    timer_handler();
    TimerHandle_t timer = xTimerCreate("Timer", 60000/portTICK_PERIOD_MS, pdTRUE, (void*) 0, timer_handler);
    xTimerStart(timer, portMAX_DELAY);
    xTaskCreate(restart_check, "restart_check", 2048, NULL, 0, NULL);
}
