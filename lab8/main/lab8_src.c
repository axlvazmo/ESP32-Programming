#include <stdio.h>
#include <math.h>
#include <driver/dac.h>
#include <driver/adc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
 
/* Create square wave */
void createTriangleWave(void *pvParameters){
 
    /* Enable DAC output through channel 1 */
    esp_err_t ret;
    ret = dac_output_enable(DAC_CHANNEL_1);         // creating DAC error handle
    ESP_ERROR_CHECK(ret);                           // Checking if there was error in enabling DAC port

    /* create variable to store value */
    static int i;
    int n = 0;

    while(1){
        /* Generate triangle wave */
        for ( i = 0; i < 20; i++){
            n = abs((i++ % 6) - 3);                 // Calculating the points for triangle wave
            dac_output_voltage(DAC_CHANNEL_2, n);   // DAC max output: 3.3v 
            vTaskDelay(10/portTICK_PERIOD_MS);      // Wait 10ms 
        }
    }
}

void createSineWave(void *perimeters){

    /* Enable DAC output through channel 2 */
    esp_err_t ret;
    ret = dac_output_enable(DAC_CHANNEL_2);         // creating DAC error handle
    ESP_ERROR_CHECK(ret);                           // Checking if there was error in enabling DAC port

    static int i;       
    int n = 0;

    while(1){
        for(i = 0; i < 20; i++){
            n = (int) ((sin(2*3.1416*i*50/1000)+1.0)*127.5);    // computing the points of the waveform
            dac_output_voltage(DAC_CHANNEL_2, n);   // Outputing calculated value
            vTaskDelay(10/portTICK_PERIOD_MS);      // Wait 10ms
        }
    }
}

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
 
    /*Set the ADC @ 12 bits -> 2^12 = 4096*/
    adc1_config_width(ADC_WIDTH_BIT_12);
 
    /*Set CHANNEL 6 @ 2600 mV*/
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
}
 
void app_main(void){

    /* set ADC*/
    adc_setup();    // used ADC to test values of waveformes

    /* Create Tasks */
    xTaskCreate(&ADCtask, "ADCtask", 2048,NULL, 5, NULL);
    xTaskCreate(&createTriangleWave, "Square Wave Task", 4096, NULL, 5, NULL);
    xTaskCreate(&createSineWave, "Sine Wave Task", 4096, NULL, 5, NULL);


}