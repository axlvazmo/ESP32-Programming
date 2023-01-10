#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
 
/* Define pin 13 as a “BLINK_GPIO” */
#define BLINK_GPIO 2    // onboard LED
#define HIGH 1          // variable for LED ON
#define LOW 0           // variable for LED off
 
/* Code for the hello_task task */
void hello_task(void *pvParameters){
    while(1){
        printf("Hello World!");     // print to terminal 
        vTaskDelay(2000/portTICK_PERIOD_MS);    // 2 seconds delay
    }
}
 
/* Code for the blinky task */
void blinky_task(void *pvParameters)
{
    gpio_pad_select_gpio(BLINK_GPIO);       // select port
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);       // set port direction as output
    while(1){
        gpio_set_level(BLINK_GPIO, HIGH);       // turning LED on
        vTaskDelay(5000/portTICK_PERIOD_MS);    // 5seconds delay
        gpio_set_level(BLINK_GPIO, LOW);        // turning LED off
        vTaskDelay(5000/portTICK_PERIOD_MS);    // 5 seconds delay
    }
}
 
void led_sequence_task(void *pvParameters){
    int led[5] = {14, 26, 33, 35, 34};      // ports for LED sequence
    int size = sizeof(led)/sizeof(uint8_t);
    int i;      //counter for increment
    int j = size - 1;       //counter for decrement
    for(i=0;i<size;i++){
        gpio_pad_select_gpio(led[i]);       // select port
        gpio_set_direction(led[i], GPIO_MODE_OUTPUT);       // set port direction as output
    }
    for (i = 0; i < size; i++){
        gpio_set_level(led[i], HIGH);       //turning on LEDs from left to right  
        gpio_set_level(led[j], HIGH);       //turning on LEDs from right to left
        vTaskDelay(300 / portTICK_PERIOD_MS);   
        gpio_set_level(led[i], LOW);        //turning off LEDs
        gpio_set_level(led[j], LOW);        // ""
        j--;
    }
}
 
void app_main(){
    /* Create the task defined by xTaskCreate.*/
    xTaskCreate(&hello_task, "hello_task", 4096, NULL, 5, NULL);
    xTaskCreate(&blinky_task, "blinky_task", 512, NULL, 5, NULL);
 
    /*Create any additional tasks*/
    xTaskCreate(&led_sequence_task, "led_sequence_task", 4096, NULL, 5, NULL);
}

