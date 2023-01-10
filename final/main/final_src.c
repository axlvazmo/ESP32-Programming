/**
 * Final Project - Four-Way Semaphore With Pedestrian Cross Request
 * Microprocessor System II
 * Axel Vazquez Montano
 * 
 * Project description:
 *  This code simulates a four way semaphore (East - West, North - South)
 *  that is able to take in user input to cross the street.
*/

/* Importing Libraries */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/led.h"

/* Global Variable Definitions */
#define ESP_INTR_FLAG_DEFAULT   0   // interrupt flag configuration
#define E_W_G1      4               // ESP32 ports used for LED LEDs
#define E_W_G2      33
#define E_W_Y1      2
#define E_W_Y2      16
#define E_W_R1      15
#define E_W_R2      18
#define N_S_G1      22
#define N_S_G2      13
#define N_S_Y1      21
#define N_S_Y2      14
#define N_S_R1      19
#define N_S_R2      26
#define P_W         12              // pedestrian walk LED
#define P_S         27              // pedestrian stop LED
#define EW_G_D      "green EW"      // direction of the current green LED
#define NS_G_D      "green on NS"   // direction of the green LED
#define GPIO_OUTPUT_PIN_SEL ((1ULL << E_W_G1) | (1ULL << E_W_Y1) | (1ULL << E_W_R1) | (1ULL << N_S_G1) | (1ULL << N_S_R1) | (1ULL << N_S_Y1) | (1ULL << P_S) | (1ULL << P_W))   // output port select for setup

/* Interrupt Variables */
#define PEDESTRIAN_BUTTON   23      // port for interrupt
#define GPIO_INPUT_PIN_SEL  (1ULL << PEDESTRIAN_BUTTON)     // input port select for setup

/* PWM Global Variables */
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (6962) // Set duty to 85%. ((2 ** 13) - 1) * 85% = 6962
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

/**
 * \struct traffic_lights_t
 * @brief Traffic LEDs Object - Contains the output port config and the current direction of green LEDs.
 * @see led.h
 */
typedef struct{
    led_t EWG1;                 // contains the configuration of output ports for LEDs
    led_t EWG2;                 // uses custom LED driver
    led_t EWY1;            
    led_t EWY2;
    led_t EWR1;            
    led_t EWR2;
    led_t NSG1;            
    led_t NSG2;
    led_t NSY1;
    led_t NSY2;
    led_t NSR1;
    led_t NSR2;
    led_t PEW;
    led_t PES;
    char green_direction[11];   // contains the direction of the green LED, used to change between EW - NS
}traffic_lights_t;

/* Global Traffic LEDs Object */
traffic_lights_t utep_traffic_light;

/* Create Gloabl Semaphore Handles*/
SemaphoreHandle_t semaphore1 = NULL;
SemaphoreHandle_t semaphore2 = NULL;

/* Global Task Handle */
TaskHandle_t xHandle;

/* GPIO interrupt handler */
static void IRAM_ATTR pedestrian_button_interrupt_handler(void * arg) {
    vTaskDelete(xHandle);           // deleting task from the scheduler
    xSemaphoreGive(semaphore2);     // giving semaphore to the pedestrian sequence task
}

/**
 * @brief Task that implements algorithm for cars EW - NS
 * 
 * @param None
 * @return None
*/
void car_sequence_task(void* pvParameters){
    int green_light_timer = 10, yellow_light_timer = 5;     // timer variables
    bool caution_flag = false;          // indicates change from green to yellow
    while(1){
        if(xSemaphoreTake(semaphore1, 100/portTICK_PERIOD_MS) == pdTRUE){           // checking if semaphore is received
            if(strcmp(utep_traffic_light.green_direction, EW_G_D) == 0){            // checking if direction is EW
                if(utep_traffic_light.EWG1.state == OFF && caution_flag == false){  // if EW sequence is starting over
                    led_on(&utep_traffic_light.EWG1);       // turn on needed LEDs
                    led_on(&utep_traffic_light.EWG2);
                    led_on(&utep_traffic_light.NSR1);
                    led_on(&utep_traffic_light.NSR2);
                    led_on(&utep_traffic_light.PES);
                    led_off(&utep_traffic_light.EWY1);      // turn off the rest of LEDs
                    led_off(&utep_traffic_light.EWY2);
                    led_off(&utep_traffic_light.EWR1);
                    led_off(&utep_traffic_light.EWR2);
                    led_off(&utep_traffic_light.NSG1);
                    led_off(&utep_traffic_light.NSG2);
                    led_off(&utep_traffic_light.NSY1);
                    led_off(&utep_traffic_light.NSY2);
                    led_off(&utep_traffic_light.PEW); 
                }
                while(green_light_timer != 0){                  
                    green_light_timer = green_light_timer - 1;  // 10 seconds in green LED
                    vTaskDelay(1000/portTICK_PERIOD_MS);        // 1s delay
                }
                while(yellow_light_timer != 0){
                    caution_flag = true;                        // setting the cautin flag
                    if(utep_traffic_light.EWG1.state != OFF){   // checking if the green LEDs are off to avoid calling the function repeatedly
                        led_off(&utep_traffic_light.EWG1);
                        led_off(&utep_traffic_light.EWG2);
                        led_on(&utep_traffic_light.EWY1);
                        led_on(&utep_traffic_light.EWY2);
                    }
                    yellow_light_timer = yellow_light_timer - 1;    // update counter
                    vTaskDelay(1000/portTICK_PERIOD_MS);            // 1s delay
                }
                strcpy(utep_traffic_light.green_direction, NS_G_D); // changing the green LED direction from EW to NS
                green_light_timer = 10;     // restart counter
                yellow_light_timer = 5;     // restart counter
                caution_flag = false;       // reset caution flag
            } else {
                if(utep_traffic_light.NSG1.state == OFF && caution_flag == false){ // checking if green LED direction is NS
                    led_on(&utep_traffic_light.NSG1);       // turn on needed LEDs
                    led_on(&utep_traffic_light.NSG2);
                    led_on(&utep_traffic_light.EWR1);
                    led_on(&utep_traffic_light.EWR2);
                    led_on(&utep_traffic_light.PES);
                    led_off(&utep_traffic_light.NSY1);      // turn off the rest of LEDs
                    led_off(&utep_traffic_light.NSY2);
                    led_off(&utep_traffic_light.NSR1);
                    led_off(&utep_traffic_light.NSR2);
                    led_off(&utep_traffic_light.EWG1);
                    led_off(&utep_traffic_light.EWG2);
                    led_off(&utep_traffic_light.EWY1);
                    led_off(&utep_traffic_light.EWY2);
                    led_off(&utep_traffic_light.PEW);
                }
                while(green_light_timer != 0){
                    green_light_timer = green_light_timer - 1;  // 10s in green LED
                    vTaskDelay(1000/portTICK_PERIOD_MS);        // 1s delay
                }
                while(yellow_light_timer != 0){
                    caution_flag = true;                        // setting the cautin flag
                    if(utep_traffic_light.NSG1.state != OFF){   // checking if the green LEDs are off to avoid calling the function repeatedly
                        led_off(&utep_traffic_light.NSG1);
                        led_off(&utep_traffic_light.NSG2);
                        led_on(&utep_traffic_light.NSY1);
                        led_on(&utep_traffic_light.NSY2);
                    }
                    yellow_light_timer = yellow_light_timer - 1; // update counter
                    vTaskDelay(1000/portTICK_PERIOD_MS);        // 1s delay
                }
                strcpy(utep_traffic_light.green_direction, EW_G_D); // changing the green LED direction from EW to NS
                green_light_timer = 10;             // restart counter
                yellow_light_timer = 5;             // restart counter
                caution_flag = false;               // reset caution flag
            }
            xSemaphoreGive(semaphore1);             // give semaphore to start again 
            vTaskDelay(100/portTICK_PERIOD_MS);
        } else {

            vTaskDelay(100/portTICK_PERIOD_MS);
        }
    }
}


/**
 * @brief Task that implements algorithm for pedestrians
 * 
 * @param None
 * @return None
*/
void pedestrian_sequence_task(void* pvParameters){
    int caution = 10;               // counter for flashing yellow LEDs
    int all_stop = 5;               // counter for all red LEDs
    bool pedestrian_go = false;     // flag indicates when pedestrians can cross the street
    while(1){    
        if(xSemaphoreTake(semaphore2, 100/portTICK_PERIOD_MS) == pdTRUE){   // checking if we have received the semaphore
            if(strcmp(utep_traffic_light.green_direction,EW_G_D) == 0){     // if the direction is EW
                led_off(&utep_traffic_light.EWG1);                          // turn off all semaphore LEDs
                led_off(&utep_traffic_light.EWG2);
                led_off(&utep_traffic_light.EWY1);
                led_off(&utep_traffic_light.EWY2);
                led_off(&utep_traffic_light.EWR1);
                led_off(&utep_traffic_light.EWR2);
                while(caution != 0){
                    led_toggle(&utep_traffic_light.EWY1);                   // toggle yellow LEDs of EW for 10s
                    led_toggle(&utep_traffic_light.EWY2);
                    caution = caution - 1;
                    vTaskDelay(1000/portTICK_PERIOD_MS);                    // 1s delay
                }
                while(all_stop != 0){
                    if(utep_traffic_light.EWR1.state != ON){                // if the red LEDs in EW are not on
                        led_off(&utep_traffic_light.EWY1);                  // turn off yellow LEDs and turn on red LEDs for 5s
                        led_off(&utep_traffic_light.EWY2);
                        led_on(&utep_traffic_light.EWR1);
                        led_on(&utep_traffic_light.EWR2);
                    }
                    all_stop = all_stop - 1;
                    vTaskDelay(1000/portTICK_PERIOD_MS);                    // 1s delay
                }
                pedestrian_go = true;
            } else if(strcmp(utep_traffic_light.green_direction, NS_G_D) == 0){ // if the direction is NS
                led_off(&utep_traffic_light.NSG1);                          // turn off all semaphore LEDs
                led_off(&utep_traffic_light.NSG2);
                led_off(&utep_traffic_light.NSR1);
                led_off(&utep_traffic_light.NSR2);
                while(caution != 0){
                    led_toggle(&utep_traffic_light.NSY1);                   // toggle yellow LEDs of NS for 10s
                    led_toggle(&utep_traffic_light.NSY2);
                    caution = caution - 1;
                    vTaskDelay(1000/portTICK_PERIOD_MS);                    // 1s delay
                }
                while(all_stop != 0){
                    if(utep_traffic_light.NSR1.state != ON){                // if the red LEDs in NS are not on
                        led_off(&utep_traffic_light.NSY1);                  // turn off yellow LEDs and turn on red LEDs for 5s
                        led_off(&utep_traffic_light.NSY2);
                        led_on(&utep_traffic_light.NSR1);
                        led_on(&utep_traffic_light.NSR2);
                    }                    
                    all_stop = all_stop - 1;
                    vTaskDelay(1000/portTICK_PERIOD_MS);                    // 1s delay
                }
                pedestrian_go = true;                                       // set the pedestrian flag to allow pedestrians to cross
            }
            if(pedestrian_go == true){                                      // if the pedestrian flag is set

                led_off(&utep_traffic_light.PES);                           // turn off the do not walk LED
                led_on(&utep_traffic_light.PEW);                            // turn on the walk LED

                /* Set duty to 50% */
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY)); // play auditory signal on buzzer through PWM
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));         // update duty to apply the new value
                vTaskDelay(1000/portTICK_PERIOD_MS);                                // 1s delay
                ESP_ERROR_CHECK(ledc_stop(LEDC_MODE,LEDC_CHANNEL, 0));              // stop auditory signal

                for( int i = 0; i < 4; i++){
                    vTaskDelay(1000/portTICK_PERIOD_MS);                            // solid walk LED for 5s
                }
                for(int i = 0; i < 10; i++){                                        // blinking walk LED for 10s and toggle auditory signal
                    if(utep_traffic_light.PEW.state == OFF){                        // checking if the walk LEDs are off
                        led_on(&utep_traffic_light.PEW);                            // turn on walk LED
                        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY)); // play auditory signal on buzzer through PWM
                        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));         // update duty to apply the new value
                        vTaskDelay(1000/portTICK_PERIOD_MS);                                // 1s delay
                    } else if(utep_traffic_light.PEW.state == ON){                  // checking if the walk LEDs are on
                        led_off(&utep_traffic_light.PEW);                           // turn walk LED off
                        ESP_ERROR_CHECK(ledc_stop(LEDC_MODE,LEDC_CHANNEL, 0));      // turn off auditory signal
                        vTaskDelay(1000/portTICK_PERIOD_MS);                        // 1s delay
                    }
                }
                led_off(&utep_traffic_light.PEW);                                   // turn off walk LED
                led_on(&utep_traffic_light.PES);                                    // turn on do not walk LED
                ESP_ERROR_CHECK(ledc_stop(LEDC_MODE,LEDC_CHANNEL, 0));              // turn off auditory signal
            }
            pedestrian_go = false;          // resetting flag
            caution = 10;                   // restart counter
            all_stop = 5;                   // restart counter
            xTaskCreate(&car_sequence_task, "Car Sequence Task", 2048, NULL, 4, &xHandle);  // re-create car sequence task
            vTaskDelay(2000/portTICK_PERIOD_MS);    // waiting 2s before returning to car sequence
            xSemaphoreGive(semaphore1);             // giving semaphore to car sequence
        } else {
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
    }
}

/**
 * @brief Set up traffic light interrupt
 * 
 * @param None
 * @return None
 */
void gpio_setup(void){

    /* Interrupt Configuration */
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_POSEDGE;  //interrupt of rising edge
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 1;       //set as pull down
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    /* Set default interrupt flag */
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    /* Add ISR handler */
    gpio_isr_handler_add(PEDESTRIAN_BUTTON, pedestrian_button_interrupt_handler, (void *) PEDESTRIAN_BUTTON);

    /* Structure Initialization */
    utep_traffic_light.EWG1.pin = E_W_G1;   // initializing structure members with port numbers and initial state
    utep_traffic_light.EWG1.state = OFF;
    utep_traffic_light.EWG2.pin = E_W_G2;
    utep_traffic_light.EWG2.state = OFF;
    utep_traffic_light.EWY1.pin = E_W_Y1;
    utep_traffic_light.EWY1.state = OFF;
    utep_traffic_light.EWY2.pin = E_W_Y2;
    utep_traffic_light.EWY2.state = OFF;
    utep_traffic_light.EWR1.pin = E_W_R1;
    utep_traffic_light.EWR1.state = OFF;
    utep_traffic_light.EWR2.pin = E_W_R2;
    utep_traffic_light.EWR2.state = OFF;
    utep_traffic_light.NSG1.pin = N_S_G1;
    utep_traffic_light.NSG1.state = OFF;
    utep_traffic_light.NSG2.pin = N_S_G2;
    utep_traffic_light.NSG2.state = OFF;
    utep_traffic_light.NSY1.pin = N_S_Y1;
    utep_traffic_light.NSY1.state = OFF;
    utep_traffic_light.NSY2.pin = N_S_Y2;
    utep_traffic_light.NSY2.state = OFF;
    utep_traffic_light.NSR1.pin = N_S_R1;
    utep_traffic_light.NSR1.state = OFF;
    utep_traffic_light.NSR2.pin = N_S_R2;
    utep_traffic_light.NSR2.state = OFF;
    utep_traffic_light.PEW.pin = P_W;
    utep_traffic_light.PEW.state = OFF;
    utep_traffic_light.PES.pin = P_S;
    utep_traffic_light.PES.state = OFF;
    strcpy(utep_traffic_light.green_direction, EW_G_D);

    /* LED Initialization */
    led_enable(&utep_traffic_light.EWG1);       //enabling LEDs with driver
    led_enable(&utep_traffic_light.EWG2);
    led_enable(&utep_traffic_light.EWY1);
    led_enable(&utep_traffic_light.EWY2);
    led_enable(&utep_traffic_light.EWR1);
    led_enable(&utep_traffic_light.EWR2);
    led_enable(&utep_traffic_light.NSG1);
    led_enable(&utep_traffic_light.NSG2);
    led_enable(&utep_traffic_light.NSY1);
    led_enable(&utep_traffic_light.NSY2);
    led_enable(&utep_traffic_light.NSR1);
    led_enable(&utep_traffic_light.NSR2);
    led_enable(&utep_traffic_light.PEW);
    led_enable(&utep_traffic_light.PES);
    led_off(&utep_traffic_light.EWG1);          // turning all LEDs off
    led_off(&utep_traffic_light.EWG2);
    led_off(&utep_traffic_light.EWY1);
    led_off(&utep_traffic_light.EWY2);
    led_off(&utep_traffic_light.EWR1);
    led_off(&utep_traffic_light.EWR2);
    led_off(&utep_traffic_light.NSG1);
    led_off(&utep_traffic_light.NSG2);
    led_off(&utep_traffic_light.NSY1);
    led_off(&utep_traffic_light.NSY2);
    led_off(&utep_traffic_light.NSR1);
    led_off(&utep_traffic_light.NSR2);
    led_off(&utep_traffic_light.PEW);
    led_off(&utep_traffic_light.PES);

}

/**
 * @brief Set up PWM using ledc driver
 * 
 * @param None
 * @return None
 */
static void ledc_setup(void){

    /* Prepare and Apply PWM Channel Config */
    ledc_timer_config_t ledc_timer = {
        .speed_mode         = LEDC_MODE,
        .timer_num          = LEDC_TIMER,
        .duty_resolution    = LEDC_DUTY_RES,
        .freq_hz            = LEDC_FREQUENCY,   // set output frequency at 5 kHz
        .clk_cfg            = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));    // applying timer config

    /* Prepare and Apply PWM Channel Configuration */
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL,
        .timer_sel  = LEDC_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = LEDC_OUTPUT_IO,
        .duty       = 0,                    // set duty cycle to 0%
        .hpoint     = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));    // applying channel config
}

void app_main(void){

    /* setup GPIOs & interrupts */
    gpio_setup();       // setup of GPIO and interrupt
    ledc_setup();       // set up PWM

    /* Creating Binary Semaphore */
    semaphore1 = xSemaphoreCreateBinary();      // semaphore for car sequence
    semaphore2 = xSemaphoreCreateBinary();      // semaphore for pedestrian sequence

    /* Creating Tasks */
    xTaskCreate(&car_sequence_task, "Car Sequence Task", 2048, NULL, 4, &xHandle);              // task for car sequence
    xTaskCreate(&pedestrian_sequence_task, "Pedestrian Sequence Task", 2048, NULL, 4, NULL);    // task for pedestrian sequence
    
    xSemaphoreGive(semaphore1);     // give semaphore to car sequence to start flow of the program
}