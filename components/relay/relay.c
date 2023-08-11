#include <stdio.h>
#include "relay.h"
#include <driver/gpio.h>

void init_relay(){
    gpio_set_direction(relay1_num, GPIO_MODE_OUTPUT);
    gpio_set_level(relay1_num, 0);
    gpio_set_direction(relay2_num, GPIO_MODE_OUTPUT);
    gpio_set_level(relay2_num, 0);
}

void toggle_relay(uint8_t relay_number){
    switch (relay_number)
    {
    case 1:
        if (relay1_state){
            relay1_state = !relay1_state;
            gpio_set_level(relay1_num, relay1_state);
            
        }
        else{
            relay1_state = 1;
            gpio_set_level(relay1_num, 1);
            
        }
        break;

    case 2:
        if (relay2_state){
            gpio_set_level(relay2_num, 0);
            relay2_state = 0;
        }
        else{
            gpio_set_level(relay2_num, 1);
            relay2_state = 1;
        }
        break;

    default:
        break;
    }
}