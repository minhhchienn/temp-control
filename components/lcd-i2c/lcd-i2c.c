/*
    LCD library of ESP32. Created by minhhchienn.
    Support for LCD1602.
*/

#include <stdio.h>
#include <lcd-i2c.h>
#include <driver/i2c.h>
#include <freeRTOS/task.h>
#include <rom/ets_sys.h>
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>

/*
    Connection pinout
    P0 - RS
    P1 - R/W
    P2 - E
    P3 - Backlight
    P4 - D4
    P5 - D5
    P6 - D6
    P7 - D7
*/

#define LCD_COMMAND     0x00    //Set RS pin to 0
#define LCD_DATA        0x01    //Set RS pin to 1
#define LCD_ENABLE      0x04    //Set E pin to 1
#define LCD_BACKLIGHT   0x08    //Turn on the backlight

//Instruction set
#define LCD_CLEAR       0x01    //Clear screen
#define LCD_HOME        0x02    //Go to first of line 1
#define SHIFT_LEFT      0x04    //Shift left the cursor after write character
#define SHIFT_RIGHT     0x06    //Shift right the cursor after write character
#define DISPLAY_LEFT    0x05    //Shift left the display
#define DISPLAY_RIGHT   0x07    //Shift right the display
#define DISPLAY_OFF     0x08    //Turn of the display, cursor
#define DISPLAY_ON      0x0C    //Turn on the display, turn of the cursor

#define LINE_ONE        0x80    //First position of line one
#define LINE_TWO        0xC0    //First position of line two

void lcd_write_nibble(LCD *lcd, uint8_t data, uint8_t mode);
void lcd_write_byte(LCD *lcd, uint8_t data, uint8_t mode);
void lcd_send_command(LCD *lcd, uint8_t cmd);
void lcd_send_data(LCD *lcd, uint8_t data);

//Init I2C interface for ESP32
void i2c_master_init(LCD *lcd){
    i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = lcd->sda_pin,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_io_num = lcd->scl_pin,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 100000,
    .clk_flags = 0,
    };
    i2c_param_config(lcd->port, &conf);
	i2c_driver_install(lcd->port, I2C_MODE_MASTER, 0, 0, 0);
}


//Write a nibble to LCD. Data format: abcd0000 (abcd is a nibble)
//Mode: 0x00 - Command, 0x01 - Data
void lcd_write_nibble(LCD *lcd, uint8_t data, uint8_t mode){
    /*
        TODO: Write 4 bits data to LCD
        Input: lcd - pointer to lcd
               data - 4 bits data. Format: abcd0000 (abcd is a nibble)
               mode - writing mode. If mode = 0x00 is write command, if mode = 0x01 is write data
        Output: 4 bits have been written to lcd through LCD
        Algorithm: -Set the value 0 for RS pin if write command, 1 if write data
                   -Set the value 0 for R/W pin
                   -Set data for D4-7
                   -Pull the Enable pin to 1 and then down to 0
    */
    uint8_t byte = (data & 0xF0) | mode | LCD_BACKLIGHT;   
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (lcd->lcd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, byte, true);     //Pull the Enable pin to 1
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(lcd->port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    //Pull up enable
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (lcd->lcd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, byte | LCD_ENABLE, true);     //Pull the Enable pin to 1
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(lcd->port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    ets_delay_us(1); //Delay 1us. (Min 450ns in datasheet)

    //Pull down enable
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (lcd->lcd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, byte &~ LCD_ENABLE, true);                           //Pull the Enable pin to 0
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(lcd->port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(1 / portTICK_PERIOD_MS);
}

//Write a byte to LCD.
//Mode: 0 - Command, 1 - Data
void lcd_write_byte(LCD *lcd, uint8_t data, uint8_t mode){
    lcd_write_nibble(lcd, data & 0xF0, mode); //Send 4 bits high
    lcd_write_nibble(lcd, (data << 4) & 0xF0, mode); //Send 4 bits low
}

//Send command to LCD
void lcd_send_command(LCD *lcd, uint8_t cmd){
    lcd_write_byte(lcd, cmd, 0x00);
}

//Send data to LCD
void lcd_send_data(LCD *lcd, uint8_t data){
    lcd_write_byte(lcd, data, 0x01);
}

//LCD Init
void lcd_init(LCD *lcd){
    //Init i2c interface
    i2c_master_init(lcd);
    vTaskDelay(100 / portTICK_PERIOD_MS); //Delay 100ms. (Min 15ms in datasheet)

    lcd_write_nibble(lcd, 0x30, LCD_COMMAND);
    vTaskDelay(10 / portTICK_PERIOD_MS); //Delay 10ms. (Min 4.1ms in datasheet)

    lcd_write_nibble(lcd, 0x30, LCD_COMMAND);
    ets_delay_us(200);  //Delay 200us. (Min 100us in datasheet)

    lcd_write_nibble(lcd, 0x30, LCD_COMMAND);

    lcd_write_nibble(lcd, 0x20, LCD_COMMAND);
    ets_delay_us(80);

    lcd_send_command(lcd, 0x28);        //4bits mode, 2 lines, 5x8 pixels
    ets_delay_us(80);

    lcd_send_command(lcd, LCD_CLEAR);        //Clear screen
    vTaskDelay(2 / portTICK_PERIOD_MS);
    
    lcd_send_command(lcd, SHIFT_RIGHT); //Shift right the cursor after write character
    ets_delay_us(80);
    
    lcd_send_command(lcd, DISPLAY_ON);  //Turn on the display
}

void lcd_set_cursor(LCD *lcd, uint8_t row, uint8_t col){
    if (row > lcd->rows - 1 || col > lcd->cols - 1){
        printf("Invalid position");
        return;
    }
    uint8_t row_offsets[] = {LINE_ONE, LINE_TWO};
    lcd_send_command(lcd, LINE_ONE | (col + row_offsets[row]));
}

//Move the cursor to the first of line
void lcd_home(LCD* lcd){
    lcd_send_command(lcd, 0x02);
    vTaskDelay(2 / portTICK_PERIOD_MS);
}

//Write a character at the cursor position
void lcd_write_char(LCD *lcd, char ch){
    lcd_send_data(lcd, ch);
}

//Write a string from the cursor position
void lcd_write_string(LCD *lcd, char* str){
    while (*str) {
        lcd_write_char(lcd, *str++);
    }
}


//Test function
void test(LCD* lcd){
    uint8_t data = 0x00;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (lcd->lcd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);     //Pull the Enable pin to 1
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(lcd->port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (lcd->lcd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, ~data, true);     //Pull the Enable pin to 1
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(lcd->port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}