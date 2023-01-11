/*
    Author:     Axel Vazquez Montano
                Master of Science in Electrical and Computer Engineering
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/led.h"


#define ESP_INTR_FLAG_DEFAULT   0   /* Interrupt flag configuration */

#define FIRE_LED                2   /* Onboard led */
#define DISABLED_LED            18     

/* Update with custom pins */
#define SMOKE_DETECTOR_PIN      22  /* Smoke detector pin */
#define DISABLE_ALARM_PIN       23  /* Disable alarm pin */

/**
 * \struct fire_alarm_t
 * \brief Fire alarm object
 * @see led.h 
 */
typedef struct{
    led_t indicator;            /*!< led indicator */
    gpio_num_t  smokeDetector;  /*!< smoke detector pin */
    gpio_num_t  disableAlarm;   /*!< disable alarm pin */
    bool active;                /*!< fire alarm state */
}fire_alarm_t;

/* Global fire alarm object */
fire_alarm_t utep_alarm;

/* GPIO interrupt handler */
static void IRAM_ATTR gpio_interrupt_handler(void * arg) {
    
    /* Typecast argument */
    gpio_num_t gpio = (gpio_num_t) arg;

    /* Check if smoke detector pin has been pressed or disable alarm */
    if(gpio == SMOKE_DETECTOR_PIN){
        utep_alarm.active = true;               // if alarm button is pressed, set alarm to active
    }else if (gpio == DISABLE_ALARM_PIN){
        utep_alarm.active = false;              // if disable alarm was pressed, set alarm to deactivate
    }
}


/**
 * @brief Set up fire alarm interrupt
 * 
 * @param alarm pointer to fire alarm object
 * @return None
 */
void fire_alarm_interrupt_setup(fire_alarm_t * const alarm){

    /* IO configuration */
    gpio_config_t io_conf;

    /* Interrupt configuration */
    io_conf.intr_type = GPIO_INTR_POSEDGE;      //interrupt will be activated in positive edge
    io_conf.mode = GPIO_MODE_INPUT;             // set as input 
    io_conf.pin_bit_mask = (1ULL << utep_alarm.smokeDetector) | (1ULL << utep_alarm.disableAlarm);      // mask all other inputs to avoid setting off another interrupt
    io_conf.pull_down_en = 1;       //set as pull down
    io_conf.pull_up_en = 0;
    
    /* Set configuration */
    gpio_config(&io_conf);

    /* Set default interrupt flag */
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);    
   
    /* Add ISR handler */
    gpio_isr_handler_add(alarm->smokeDetector, gpio_interrupt_handler, (void *) alarm->smokeDetector);
    /* Add missing interrupt */
    gpio_isr_handler_add(alarm->disableAlarm, gpio_interrupt_handler, (void *) alarm->disableAlarm);
}

void app_main(void){
    /* Initialize fire alarm object */
    led_t fire_alarm_led = {.pin = FIRE_LED, .state = OFF};
    led_t disabled_alarm_led = {.pin = DISABLED_LED, .state = OFF};
    utep_alarm.disableAlarm = DISABLE_ALARM_PIN;        // passing disable button to structure
    utep_alarm.smokeDetector = SMOKE_DETECTOR_PIN;      //passing smoke detector button to structure

    /* Enable led*/
    led_enable(&fire_alarm_led);        //enabling LEDs with driver
    led_enable(&disabled_alarm_led);

    /* setup GPIOs & interrupts */
    fire_alarm_interrupt_setup(&utep_alarm);        // setup of GPIO and interrupt
    
    while(1){
        
        /* Check if alarm has been activated */
        if(utep_alarm.active){
            /* Toggle indicator led @ 500 ms */
            led_off(&disabled_alarm_led);       // turn off alarm disabled indicator
            led_toggle(&fire_alarm_led);        // start blinking dire alarm indicator
            vTaskDelay(5000/portTICK_PERIOD_MS);
           
        }else if (utep_alarm.active == false){
            led_off(&fire_alarm_led);           // turn off fire alarm indicator
            led_on(&disabled_alarm_led);        // turn on alarm disabled indicator
            vTaskDelay(3000/ portTICK_PERIOD_MS);
        }else{
            /* 100 ms delay */
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

}
