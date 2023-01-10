#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/led.h"

#define ONBOARD_LED_PORT     2
led_t ONBOARD_LED = {.pin = ONBOARD_LED_PORT, .state= OFF};

typedef struct {
    char str[10];
    int32_t val;
    char studentName[25];
    uint32_t studentID;
    bool isLedOn;
}Graduate_t;

/* Global Queue handle */
QueueHandle_t xQueue;
 
/* Example task 1 */
void task1(void *pvParameters){
    
    /* store data */
    Graduate_t *taskPtr = (Graduate_t *) pvParameters;

    while(1){
        /* Send data to back of the queue */
        xQueueSendToBack(xQueue, &taskPtr, 0);
        /* 1 second delay */
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
 
/* Example task 2 */
void task2(void *pvParameters){
    /* variable to store data */
    Graduate_t *rcvPtr;
 
    while(1){
        /* Wait 100 ticks to receive queue, store in storeData */
        if(xQueueReceive(xQueue, &rcvPtr, (TickType_t)100) == pdPASS){
            printf("Data received from Task 1\n");
            printf("String: %s\n", rcvPtr->str); /* display data receive */
                printf("Value: %d\n", rcvPtr->val);
            printf("Student Name: %s\n", rcvPtr->studentName);
            printf("Student ID: %d\n", rcvPtr->studentID);
            led_toggle(&ONBOARD_LED);
            vTaskDelay(100/portTICK_PERIOD_MS); /* 100 ms */ 
        }
        vTaskDelay(500/portTICK_PERIOD_MS); /* 500 ms */
    }
}
 
void app_main(void){

    /* Create data to be sent: global or static variables */
    static char string[10] = "Fall 2022";
    static int32_t value = 123;
    static char name[15] = "Fulano Mengano";
    static int  id_no = 87788778;
    static bool led_value = false;

    /* initializing data structure */
    Graduate_t graduate_struct;
    Graduate_t *ptr;    // defining pointer to structure 
    ptr = &graduate_struct;     // setting the pointer to the address of the structure
    strcpy(graduate_struct.str, string);    // initializing structure members
    graduate_struct.val = value;
    strcpy(graduate_struct.studentName, name);
    graduate_struct.studentID = id_no;
    graduate_struct.isLedOn = led_value;

    /* Onboard LED Setup */
    led_enable(&ONBOARD_LED);
    led_off(&ONBOARD_LED);

    /* Create a queue of size 5 */
    xQueue = xQueueCreate(5, sizeof(int));
    
    /* Create tasks */
    xTaskCreate(&task1, "Task 1", 2048, (void *) ptr, 4, NULL);     // passing the pointer to the task
    xTaskCreate(&task2, "Task 2", 2048, NULL, 4, NULL);
    
}