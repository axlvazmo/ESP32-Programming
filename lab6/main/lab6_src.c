#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
 
#define ESP_INTR_FLAG_DEFAULT   0   /* Interrupt flag configuration */

#define BUTTON1 23
 
/* Global Queue handle */
QueueHandle_t xQueue;
 
/* GPIO interrupt handler */
static void IRAM_ATTR gpio_isr_handler(void *arg){
    gpio_num_t gpio = (gpio_num_t) arg;     // passing the port number as a value

    while(1){
        xQueueSendToBack(xQueue, &gpio, 0); // sending the port number to the back of the queue
        vTaskDelay(1000/portTICK_PERIOD_MS);    // wait 1 second
    }
}
 
/* Print task */
void print_task(void *pvParameters){
    int store_data;     // buffer for queue data
    while (1){
        if(xQueueReceive(xQueue, &store_data, (TickType_t)100) == pdPASS){
            printf("GPIO %d Received\n", store_data);   // printing recieved data
            vTaskDelay(100/portTICK_PERIOD_MS);     // waiting 100 ms

        }
        vTaskDelay(500/portTICK_PERIOD_MS);     // waiting 500 ms
    }
}
 
/* GPIO interrupt setup */
void gpio_interrupt_setup(void){

    /* IO Configuration */
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;  /* Set up as Positive Edge */ 
    io_conf.mode = GPIO_MODE_INPUT;     /* Set pins as input */
    io_conf.pin_bit_mask = (1ULL << BUTTON1);  /* Add input bit mask */
    io_conf.pull_down_en = 1;   /* Enable pulldown */
    io_conf.pull_up_en = 0;     /* Disable pullup */

    /* Set configuration */
    gpio_config(&io_conf);
 
    /* Set default interrupt flag */
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
 
    /* Add ISR handler */
    gpio_isr_handler_add(BUTTON1, gpio_isr_handler, (void*) BUTTON1); 
}
 
void app_main()
{
    /* Create a queue */
    xQueue = xQueueCreate(5, sizeof(int));
 
    /* Call gpio interrupt setup */
    gpio_interrupt_setup(); 
 
    /* Create task */
    xTaskCreate(&print_task, "print_task", 2048, NULL, 10, NULL);   
}