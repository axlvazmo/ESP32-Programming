# Final Project: Four-way Semaphore with Pedestrian Cross Request

## Objective

For this project I designed a four-way semaphore with pedestrian request to cross the street. The Pedestrian input will be processes after 15 seconds, during the first 
10 seconds the green lights will switch to a flashing yellow to warn traffic to come to a stop. During the remaining 5 seconds, the red lights in all directions will be
presented. Once the previous sequence is completed, there will be an auditory alarm to signal the pedestrians that they can cross and at the same time the pedestrian walk
LED will turn on and the pedestrian stop light will be turned off. After 5 seconds, the pedestrian walk light will start to blink and the auditory alarm will start to beep 
every second signaling that there is only 10 seconds left to cross the street. Afterwards the pedestrian walk LED will turn off, the pedestrian stop light will turn on and
the trafic will resume their sequence.

| Task                | Description                                                                                      |
|---------------------|--------------------------------------------------------------------------------------------------|
| car sequence        | Contains the algorithm for the car sequence                                                      |
| pedestrian sequence | Contains the algorithm for the pedestrian sequence                                               |
| ISR                 | Delete the car sequence task from the task scheduler and give a semaphore to the pedestrian task |

| Device        | Port   | Function                                 | Mode      |
|---------------|--------|------------------------------------------|-----------|
| LED1 & LED2   | 4, 33  | EW green LEDs                            | N/A       |
| LED3 & LED4   | 2, 16  | EW yellow LEDs                           | N/A       |
| LED5 & LED6   | 15, 18 | EW red LEDs                              | N/A       |
| LED7 & LED8   | 22, 13 | NS green LEDs                            | N/A       |
| LED9 & LED10  | 21, 14 | NS yellow LEDs                           | N/A       |
| LED11 & LED12 | 19, 26 | NS red LEDs                              | N/A       |
| LED13 & LED14 | 12, 27 | Pedestrian walk and pedestrian stop LEDs | N/A       |
| B1            | 23     | Pedestrian request button                | Pull-down |

## **ESP32 Pinout**

~~~
                                         +-----------------------+
                                         | O      | USB |      O |
                                         |        -------        |
                                     3V3 | [ ]               [ ] | VIN
                                     GND | [ ]               [ ] | GND
     Touch3 / HSPI_CS0 / ADC2_3 / GPIO15 | [ ]               [ ] | GPIO13 / ADC2_4 / HSPI_ID / Touch4
 CS / Touch2 / HSPI_WP / ADC2_2 /  GPIO2 | [ ]               [ ] | GPIO12 / ADC2_5 / HSPI_Q / Touch5
      Touch0 / HSPI_HD / ADC2_0 /  GPIO4 | [ ]               [ ] | GPIO14 / ADC2_6 / HSPI_CLK / Touch6
                         U2_RXD / GPIO16 | [ ]               [ ] | GPIO27 / ADC2_7 / Touch7
                         U2_TXD / GPIO17 | [ ]               [ ] | GPIO26 / ADC2_9 / DAC2
                      V_SPI_CS0 /  GPIO5 | [ ]  ___________  [ ] | GPIO25 / ADC2_8 / DAC1
                SCK / V_SPI_CLK / GPIO18 | [ ] |           | [ ] | GPIO33 / ADC1_5 / Touch8 / XTAL32
        U0_CTS / MSIO / V_SPI_Q / GPIO19 | [ ] |           | [ ] | GPIO32 / ADC1_4 / Touch9 / XTAL32
                 SDA / V_SPI_HD / GPIO21 | [ ] |           | [ ] | GPIO35 / ADC1_7 
                  CLK2 / U0_RXD /  GPIO3 | [ ] |           | [ ] | GPIO34 / ADC1_6 
                  CLK3 / U0_TXD /  GPIO1 | [ ] |           | [ ] | GPIO39 / ADC1_3 / SensVN 
        SCL / U0_RTS / V_SPI_WP / GPIO22 | [ ] |           | [ ] | GPIO36 / ADC1_0 / SensVP 
                MOSI / V_SPI_WP / GPIO23 | [ ] |___________| [ ] | EN 
                                         |                       |
                                         |  |  |  ____  ____  |  |
                                         |  |  |  |  |  |  |  |  |
                                         |  |__|__|  |__|  |__|  |
                                         | O                   O |
                                         +-----------------------+
~~~

## Code Explanation


