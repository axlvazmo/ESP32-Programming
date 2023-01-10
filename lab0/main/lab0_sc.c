#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
 
#define LOW  0  /* LOGIC LOW*/
#define HIGH 1  /* LOGIC HIGH*/
 
void setOutputs(uint8_t *out, int size){
    /* iterate over the size of the array */
    for (int i = 0; i < size; i++) 
    {
        gpio_pad_select_gpio(out[i]); /* select the GPIO pins */
        gpio_set_direction(out[i], GPIO_MODE_OUTPUT); /* set direction as outputs */
    }
    return;
}
 
void sweep(uint8_t *led, int size){
    /* iterate over the size of the array */
    for ( int i = 0; i < size; i++){
        gpio_set_level(led[i], HIGH);   //turning on the LED
        vTaskDelay(300 / portTICK_PERIOD_MS); /* 300ms delay */
        gpio_set_level(led[i], LOW);        //turning off the LED
    }
    /* iterate over the size of the array */
    for (int i = size - 1; i >= 0; i--){
        gpio_set_level(led[i], HIGH);       //turning on the LED
        vTaskDelay(300 / portTICK_PERIOD_MS); /* 2 second delay */
        gpio_set_level(led[i], LOW);        //turning off the LED
    }
}
 
void led_chaser(uint8_t *led, int size){
    /* turning all LEDs on  from low to high index */
    for (int i = 0; i < size; i++){
        gpio_set_level(led[i], HIGH);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    /* turning all LEDs off from low to high index */
    for (int i = 0; i < size; i++){
        gpio_set_level(led[i], LOW);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    /* turning all LEDs on from high to low index */
    for (int i = size - 1; i >= 0; i--){
        gpio_set_level(led[i], HIGH);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    /* turning all LEDs off from high to low index */
    for (int i = size - 1; i >= 0; i--){
        gpio_set_level(led[i], LOW);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}
 
void lightShow(uint8_t *led, int size){
    int i;      // counter for incrementing 
    int j = size - 1;   // counter for decrementing
    for (i = 0; i < size; i++){
        gpio_set_level(led[i], HIGH);   //turning on the LEDs from low to high index
        gpio_set_level(led[j], HIGH);   //turning on the LEDs from high to low index
        vTaskDelay(300 / portTICK_PERIOD_MS);
        gpio_set_level(led[i], LOW);    //turning on the LEDs from low to high index
        gpio_set_level(led[j], LOW);    //turning on the LEDs from high to low index
        j--;
    }
}

void transsition(uint8_t *led, int size){
    for(int i = 0; i < size; i++){
        gpio_set_level(led[0], HIGH);
        vTaskDelay(200 / portTICK_PERIOD_MS); /* 200ms second delay */
        gpio_set_level(led[0], LOW);
        vTaskDelay(200 / portTICK_PERIOD_MS); /* 200ms second delay */
    }
}

void app_main(void){
    /*GPIOs pins*/
    uint8_t led[] = {15, 2, 4, 5, 18}; /* create an array of single led */
    int size = sizeof(led)/sizeof(uint8_t); /* get size of the array, use sizeof() to allow scalabilty */
    setOutputs(led,size); /* intialize GPIOs pins */
    while (1){
        /* Added led patterns */
        sweep(led, size);       //calling function for sweep sequence
        transsition(led, size);     //calling function for transsition sequence to indicate a change of sequence
        led_chaser(led, size);      //calling function for led chaser sequence
        transsition(led, size);
        lightShow(led, size);       //calling function for lightshow sequence
        transsition(led, size);
    }
}

idf.py -p COM5 flash