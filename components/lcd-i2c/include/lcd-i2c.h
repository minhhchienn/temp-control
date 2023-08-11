#pragma once
#ifndef __LCD_I2C__
#define __LCD_I2C__

#include <esp_err.h>
#include <stdint.h>

typedef struct {
    uint8_t lcd_addr; //Address of LCD
    uint8_t port;     //I2C connection port to lcd  
    uint8_t scl_pin;  //SCL pin connect to lcd
    uint8_t sda_pin;  //SDA pin connect to lcd
    uint8_t rows;     //Number of rows of lcd
    uint8_t cols;     //Number of cols of lcd
} LCD;

void lcd_init(LCD *lcd);
void lcd_write_char(LCD *lcd, char ch);
void lcd_write_string(LCD *lcd, char* str);
void lcd_home(LCD* lcd);
void lcd_set_cursor(LCD *lcd, uint8_t row, uint8_t col);
void test(LCD* lcd);

#endif