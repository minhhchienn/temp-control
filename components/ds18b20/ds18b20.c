#include <stdio.h>
#include "ds18b20.h"
#include <driver/gpio.h>
#include <stdint.h>
#include <freeRTOS/FreeRTOS.h>
#include <freeRTOS/task.h>
#include <rom/ets_sys.h>
#include <esp_log.h>

//Commands
#define GET_TEMP            0x44
#define SKIP_ROM            0xCC
#define READ_SCRATCH_PAD    0xBE
#define WRITE_SCRATCH_PAD   0x4E

//Device resolution
#define RES_9_BITS  0x1F
#define RES_10_BITS 0x3F
#define RES_11_BITS 0x5F
#define RES_12_BITS 0x7F

uint8_t GPIO_PIN = 4;

const char *TAG = "DS18B20";

void ds18b20_set_gpio_pin(uint8_t pin){
    GPIO_PIN = pin;
    esp_rom_gpio_pad_select_gpio(GPIO_PIN);
}

//Write a bit
void ds18b20_write(uint8_t bit){
    if (bit == 1){
        gpio_set_direction(GPIO_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_PIN, 0);
        ets_delay_us(6);

        gpio_set_level(GPIO_PIN, 1);
        ets_delay_us(64);

        gpio_set_direction(GPIO_PIN, GPIO_MODE_INPUT);
    }
    else {
        gpio_set_direction(GPIO_PIN, GPIO_MODE_OUTPUT);
		gpio_set_level(GPIO_PIN, 0);
		ets_delay_us(60);
		
        gpio_set_level(GPIO_PIN, 1);
		ets_delay_us(10);
        gpio_set_direction(GPIO_PIN, GPIO_MODE_INPUT);
    }
}

//Read a bit
uint8_t ds18b20_read(void){
    uint8_t value = 0;
    gpio_set_direction(GPIO_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_PIN, 0);
	ets_delay_us(6);

    gpio_set_level(GPIO_PIN, 0);
	ets_delay_us(9);

    gpio_set_direction(GPIO_PIN, GPIO_MODE_INPUT);
	value = gpio_get_level(GPIO_PIN);  
	ets_delay_us(55);

	return (value);
}

//Write a byte                                                                                                                                                  
void ds18b20_write_byte(uint8_t data){
    for (int i = 0; i < 8; i++){
        ds18b20_write(data & 0x01);
        data >>= 1;
    }
    ets_delay_us(100);
}

//Read a byte
uint8_t ds18b20_read_byte(void){
    uint8_t data = 0;
    for (int i = 0; i < 8; i++){
        data >>= 1;
        data |= ds18b20_read() << 7;
        ets_delay_us(15);
    }                                                                                                                                                                       
    return data;
}

uint8_t ds18b20_init(){
    uint8_t response;
    gpio_set_direction(GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_PIN, 0);
    ets_delay_us(480);

    gpio_set_level(GPIO_PIN, 1);
    ets_delay_us(70);

    gpio_set_direction(GPIO_PIN, GPIO_MODE_INPUT);
    response = gpio_get_level(GPIO_PIN);
    ets_delay_us(410);
    return response;
}

void ds18b20_request_temp(void){
    while(ds18b20_init());

    ds18b20_write_byte(SKIP_ROM);
    ds18b20_write_byte(GET_TEMP);
    ets_delay_us(800);
    
    while(ds18b20_init());
    ds18b20_write_byte(SKIP_ROM);
    ds18b20_write_byte(READ_SCRATCH_PAD);
}

float ds18b20_get_temp(void){
    ds18b20_request_temp();
    uint16_t temp = ds18b20_read_byte();
    temp = (ds18b20_read_byte() << 8) | temp;
    float result = temp / 16.0f;
    if (result > -50 && result < 125){
        printf("DS18B20: Temp = %.2f\n", result);
    }
    return result;
}