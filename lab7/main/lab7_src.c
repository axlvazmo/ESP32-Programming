#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/adc.h>
 
/* ADC task */
void ADCtask(void *pvParameters){
    while(1){
        /* Read adc value @ CHANNEL 6*/
        int val = adc1_get_raw(ADC1_CHANNEL_6);   
        vTaskDelay(100/portTICK_PERIOD_MS); /* 100 ms */
        
        /* Print reading */
        printf("ADC val: %d\n", val);
    }
}
 
/* Set ADC */
void adc_setup(void){
 
    /*Set the ADC with @ 12 bits -> 2^12 = 4096*/
    adc1_config_width(ADC_WIDTH_BIT_12);
 
    /*Set CHANNEL 6 @ 2600 mV*/
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); 
    
}
 
void app_main(void){
    /* set ADC*/
    adc_setup();
    /* Create Tasks */
    xTaskCreate(&ADCtask, "ADCtask", 2048,NULL, 5, NULL);
}