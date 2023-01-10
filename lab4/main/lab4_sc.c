#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/led.h"

#define ONBOARD_LED     2
 
/* Create global instances of sempaphore */
SemaphoreHandle_t mySemaphore1 = NULL; 
SemaphoreHandle_t mySemaphore2 = NULL;
SemaphoreHandle_t mySemaphore3 = NULL;
SemaphoreHandle_t mySemaphore4 = NULL;
SemaphoreHandle_t mySemaphore5 = NULL;
SemaphoreHandle_t mySemaphore6 = NULL;

 
/* Example task 1 */
void task1(void *pvParameters){
    while(1){
        /* Check if semaphore was received every 100 ticks */
        if((xSemaphoreTake(mySemaphore1, (TickType_t)100 ) == pdTRUE)){
              printf("Task 1 sent flag to task 2 & 3!\n");
              vTaskDelay(5000 / portTICK_PERIOD_MS); // 2 second delay
              xSemaphoreGive(mySemaphore2); /* give semaphore to run task 2 */
              xSemaphoreGive(mySemaphore3);
              
        }else{
              vTaskDelay(100 / portTICK_PERIOD_MS); /* 100 ms delay */
        }
    }
}
 
/* Example task 2 */
void task2(void *pvParameters){
    while(1){
         /* wait for atleast 100 ms to receive semaphore */
         if(xSemaphoreTake(mySemaphore2, 100/portTICK_PERIOD_MS) == pdTRUE){ 
             printf("Task 2 is running!!!!\n");
             vTaskDelay(1000/portTICK_PERIOD_MS); /* 100 ms */
             xSemaphoreGive(mySemaphore1); /* give back semaphore to task 1 */
         }else{
             vTaskDelay(100/portTICK_PERIOD_MS); /* 100 ms delay */
         }
    }
}

void task3(void *pvParameters){
    while (1){
        /* wait for atleast 100 ms to receive semaphore */
        if(xSemaphoreTake(mySemaphore3, 100/portTICK_PERIOD_MS) == pdTRUE){ 
            printf("Task 3 is running!!!!\n");
            vTaskDelay(500/portTICK_PERIOD_MS); /* 100 ms */
            xSemaphoreGive(mySemaphore4);
        }else{
            vTaskDelay(100/portTICK_PERIOD_MS); /* 100 ms delay */
        }
    } 
}

void task4(void *pvParameters){
    while (1){
        /* wait for atleast 100 ms to receive semaphore */
        if(xSemaphoreTake(mySemaphore4, 100/portTICK_PERIOD_MS) == pdTRUE){ 
            printf("Task 4 is running!!!!\n");
            vTaskDelay(500/portTICK_PERIOD_MS); /* 100 ms */
            xSemaphoreGive(mySemaphore5); /* give back semaphore to task 1 */
        }else{
            vTaskDelay(100/portTICK_PERIOD_MS); /* 100 ms delay */
        }
    }   
}

void task5(void *pvParameters){
    while (1){
        /* wait for atleast 100 ms to receive semaphore */
        if(xSemaphoreTake(mySemaphore5, 100/portTICK_PERIOD_MS) == pdTRUE){ 
            printf("Task 5 is running!!!!\n");
            vTaskDelay(1000/portTICK_PERIOD_MS); /* 100 ms */
            xSemaphoreGive(mySemaphore6); /* give back semaphore to task 1 */
        }else{
            vTaskDelay(100/portTICK_PERIOD_MS); /* 100 ms delay */
        }
    }   
}

void task6(void *pvParameters){
    while (1){
        /* wait for atleast 100 ms to receive semaphore */
        if(xSemaphoreTake(mySemaphore6, 100/portTICK_PERIOD_MS) == pdTRUE){ 
            led_toggle(ONBOARD_LED);
            printf("Task 6 is running!!!!\n");
            vTaskDelay(1000/portTICK_PERIOD_MS); /* 100 ms */
            xSemaphoreGive(mySemaphore5); /* give back semaphore to task 1 */
            // xSemaphoreGive(mySemaphore4);
        }else{
            vTaskDelay(100/portTICK_PERIOD_MS); /* 100 ms delay */
        }
    }   
}
 
void app_main(void){

    /* Enabling the LED using the driver */
    led_enable(ONBOARD_LED);
 
    /* Create two semaphores binary semaphores */
    mySemaphore1 = xSemaphoreCreateBinary(); 
    mySemaphore2 = xSemaphoreCreateBinary(); 
    mySemaphore3 = xSemaphoreCreateBinary();
    mySemaphore4 = xSemaphoreCreateBinary();
    mySemaphore5 = xSemaphoreCreateBinary();
    mySemaphore6 = xSemaphoreCreateBinary();
 
    /* Create tasks */
    xTaskCreate(&task1, "task 1", 2048, NULL, 4, NULL); 
    xTaskCreate(&task2, "task 2", 2048, NULL, 4, NULL);
    xTaskCreate(&task3, "task 3", 2048, NULL, 2, NULL);
    xTaskCreate(&task4, "task 4", 2048, NULL, 2, NULL);
    xTaskCreate(&task5, "task 5", 2048, NULL, 2, NULL);
    xTaskCreate(&task6, "task 6", 2048, NULL, 2, NULL); 
    
    /* Give the semaphore for task 1 to run */
    xSemaphoreGive(mySemaphore1); 
}