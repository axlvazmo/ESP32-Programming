#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
 
#define LOW  0  /* LOGIC LOW*/
#define HIGH 1  /* LOGIC HIGH*/
#define ONBOARD_LED 2 // ONBOARD LED GPIO Pin
#define debounceDelay 500   //delay for button debouncing
#define delay 100       //general delay

void inputSetup(uint8_t *in, int size){
    /* iterate over the size of the array */
    for (int i = 0; i < size; i++){
        gpio_pad_select_gpio(in[i]);        //select the GPIO pins
        gpio_set_direction(in[i], GPIO_MODE_INPUT);     //set direction as inputs
        if(in[i] == 23 || in[i] == 34){     //checking for the last two buttons
            gpio_set_pull_mode(in[i], GPIO_PULLDOWN_ONLY); //set as pull-up       
        }
        else{
            gpio_set_pull_mode(in[i], GPIO_PULLDOWN_ONLY); //set as pull-down
        }
    }
    return;
}

void outputSetup(uint8_t *out, int size){
    /* iterate over the size of the array */
    for (int i = 0; i < size; i++) {
        gpio_pad_select_gpio(out[i]);        //select the GPIO pins
        gpio_set_direction(out[i], GPIO_MODE_OUTPUT);       //set pins as outputs
    }
    return;
}
 
void sweep(uint8_t *led, int size){
    /* iterate over the size of the array */
    for ( int i = 0; i < size; i++){
        gpio_set_level(led[i], HIGH);       // turning LED on
        vTaskDelay(300 / portTICK_PERIOD_MS); /* 2 second delay */
        gpio_set_level(led[i], LOW);        // turning LED off
    }
    /* iterate over the size of the array */
    for (int i = size - 1; i >= 0; i--){
        gpio_set_level(led[i], HIGH);       // turning LED on
        vTaskDelay(300 / portTICK_PERIOD_MS); /* 2 second delay */
        gpio_set_level(led[i], LOW);        // turning LED off
    }
}
 
void light_show(uint8_t *led, int size){
    int i;      //counter for increment
    int j = size - 1;       //counter for decrement
    for (i = 0; i < size; i++){
        gpio_set_level(led[i], HIGH);       //turning on LEDs from left to right  
        gpio_set_level(led[j], HIGH);       //turning on LEDs from right to left
        vTaskDelay(300 / portTICK_PERIOD_MS);   
        gpio_set_level(led[i], LOW);        //turning off LEDs
        gpio_set_level(led[j], LOW);        // ""
        j--;
    }
}
 
void led_chaser(uint8_t *led, int size){
    /* Turning on all LEDs one by one */
    for (int i = 0; i < size; i++){
        gpio_set_level(led[i], HIGH);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    /* Turning off all LEDs one by one */
    for (int i = 0; i < size; i++){
        gpio_set_level(led[i], LOW);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    /* Turning on LEDs on reverse order */
    for (int i = size - 1; i >= 0; i--){
        gpio_set_level(led[i], HIGH);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
     /* Turning off LEDs on reverse order */
    for (int i = size - 1; i >= 0; i--){
        gpio_set_level(led[i], LOW);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

int toggleOnBoardLED(uint8_t toggle){
    /* Checking toggle flag */
    if(toggle == 0){
        gpio_set_level(ONBOARD_LED, HIGH);  //if flag is not set, then turn on onboard LED and set flag to 1
            toggle = 1;
    }
    else if(toggle == 1){
        gpio_set_level(ONBOARD_LED, LOW);   //if flag is set, then turn off onboard LED and set flag to 0
        toggle = 0;
    }
    return toggle;
}

/* Setting all output ports to LOW */
int turnOffLEDs(uint8_t *led, int size, int toggle){
    for(int i = 0; i < size; i ++){
        gpio_set_level(led[i], LOW);
    }
    toggle = 0;
    return toggle;
}
 
void app_main(void)
{
    /* Select GPIOs output and store in led array */
    uint8_t led[] = {15, 2, 4, 5, 18}; /*replace with your GPIO pins */
    
    /* Select GPIOs inputs and store in button array */
    uint8_t button[] = {19, 21, 22, 23, 34};
 
    /* Use sizeof() to get the size of the arrays */
    int led_size = sizeof(led) / sizeof(uint8_t);
    int button_size = sizeof(button) / sizeof(uint8_t);
 
    /* Initialize Inputs */
    inputSetup(button, button_size);
 
    /* Initialize Outputs */
    outputSetup(led, led_size);
    
    /* setting initial LED output to zero */
    for(int i = 0; i < led_size; i++){
        gpio_set_level(led[i], LOW);
    }
    
    int toggle = 0;      //onboard LED toggle flag
 
    while (1)
    {
        vTaskDelay(debounceDelay/portTICK_PERIOD_MS); /* debounce button */
        /* Polling for button press */
        int button0 = gpio_get_level(button[0]); /* read button 0 */
        int button1 = gpio_get_level(button[1]); /* read button 1 */
        int button2 = gpio_get_level(button[2]); /* read button 2 */
        int button3 = gpio_get_level(button[3]); /* read button 3 */
        int button4 = gpio_get_level(button[4]); /* read button 4 */

        /*checking which button press was actuated */
        if(button0 == 1){
            sweep(led, led_size);       //if button 1 is pressed, then activate sweep sequence (avtive hi - pull down setup)
        }
        else if (button1 == 1){
            led_chaser(led, led_size);  //if button 2 is pressed, then activate chaser sequence (avtive hi - pull down setup)
        }
        else if (button2 == 1){
            toggle = toggleOnBoardLED(toggle);      //if button 3 is pressed, then toggle onboard LED (avtive hi - pull down setup)
        }
        else if(button3 == 0){
            light_show(led, led_size);      //if button 4 is pressed, then activate light show sequence (avtive lo - pull up setup)
        }
        else if(button4 == 0){
            toggle = turnOffLEDs(led, led_size, toggle);        //if button 5 is pressed, then turn off all LEDs (avtive lo - pull up setup)
        }
        else{
          vTaskDelay(delay/portTICK_PERIOD_MS); /* 100ms to avoid WDT errors */
        }
    }
}