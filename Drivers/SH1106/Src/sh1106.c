/**
 * @file    sh1106.c
 * @author  Jose
 * @brief   Driver library for SH1106 OLED display (128x64) using I2C for STM32F1.
 * @date    13/9/26
 * @project tfg
 */

#include "sh1106.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief The size of the display is 128x64 px. The display data 
 * organization consists in 8 pages of 128x8 px each, so the buffer 
 * below needs to be 1024B long to store all the pages.
 */
static uint8_t SH1106_Buffer[SH1106_BUFFER_SIZE];

static I2C_HandleTypeDef *hi2c = NULL;
static uint16_t addr = 0;

static void SH1106_Write_Command(uint8_t command);
static void SH1106_Write_Data(uint8_t *data, uint16_t size);
static void SH1106_Write_Letter(uint8_t index, uint8_t x, uint8_t y, SH1106_Pixel_State_t state, const  SH1106_Font_t *font);

/**
  * @brief Sends a command to the display via I2C
  *
  * @param command: command to be sent
  * @retval None
  */
static void SH1106_Write_Command(uint8_t command)
{
    // 0x00 -> sending a command
    HAL_I2C_Mem_Write(hi2c, addr, SH1106_COMMAND_ADDR, 1,
        &command, 1, HAL_MAX_DELAY);
}

/**
  * @brief Sends I2C data to the display
  *
  * @param data: the address of the first byte
  * @param size: the number of bytes from data
  * @retval None
  */
static void SH1106_Write_Data(uint8_t *data, uint16_t size)
{
    // 0x40 -> sending data to GDDRAM
    HAL_I2C_Mem_Write(hi2c, addr, SH1106_DATA_ADDR, 1,
        data, size, HAL_MAX_DELAY);
}

/**
  * @brief Writes a letter from a font map into the display.
  *
  * @param index: the position of the letter in the map
  * @param x: column [0, 127]
  * @param y: row [0, 31]
  * @retval None
  */
static void SH1106_Write_Letter(uint8_t index, uint8_t x, uint8_t y, SH1106_Pixel_State_t state, const SH1106_Font_t *font) 
{
    for (uint8_t i = 0; i < font->height; i++) {
        uint16_t row = 0;
        uint8_t initial_bit = 0;

        switch (font->size) {
            case SH1106_FONT_10x16:
                const char (*fontmap16)[32] = (const char (*)[32])font->map;
                uint8_t l = fontmap16[index][(i * 2)];
                uint8_t h = fontmap16[index][(i * 2) + 1];
                row = (h << 8) | l;
                initial_bit = 6;
                break;
            case SH1106_FONT_5x8:
                const char (*fontmap8)[8] = (const char (*)[8])font->map;
                row = fontmap8[index][i];
                initial_bit = 4;
                break;
            default:
                return;    
        }

        for (uint8_t j = 0; j < font->width; j++) {
            if ((row >> (initial_bit + j)) & 0x01)
                SH1106_Draw_Pixel(x + j, y + i, state);
        }
    }
}

void SH1106_Init(const SH1106_Config_t *config)
{
    if (config == NULL || config->hi2c == NULL) return;

    hi2c = config->hi2c;
    addr = config->i2c_addr;

    HAL_Delay(100);

    // Display OFF
    SH1106_Write_Command(0xAE);

    // Display Clock Divide Ratio
    SH1106_Write_Command(0xD5);
    SH1106_Write_Command(config->osc_freq_div_ratio);

    // Multiplex Ratio
    SH1106_Write_Command(0xA8);
    SH1106_Write_Command(config->multiplex_ratio);

    // Display Offset
    SH1106_Write_Command(0xD3);
    SH1106_Write_Command(config->display_offset);

    // Start line
    SH1106_Write_Command(config->start_line);

    // DC-DC Pump
    SH1106_Write_Command(0xAD);
    SH1106_Write_Command(config->pump_voltage);

    // COM Pins (rows) Config
    SH1106_Write_Command(0xDA);
    SH1106_Write_Command(config->com_pins_config);

     // Contrast Control
    SH1106_Write_Command(0x81);
    SH1106_Write_Command(config->contrast);

    // Pre-charge Period
    SH1106_Write_Command(0xD9);
    SH1106_Write_Command(0x22); // POR

    // VCOMH Deselect Level
    SH1106_Write_Command(0xDB);
    SH1106_Write_Command(0x35); // POR

    // Resume content from GDDRAM
    SH1106_Write_Command(0xA4); 

    // Set normal display
    SH1106_Write_Command(0xA6);

    // Mirror H
    if (config->mirror & SH1106_MIRROR_HORIZONTAL) {
        SH1106_Write_Command(0xA1);
    } else {
        SH1106_Write_Command(0xA0);
    }

    // Mirror V
    if (config->mirror & SH1106_MIRROR_VERTICAL) {
        SH1106_Write_Command(0xC8);
    } else {
        SH1106_Write_Command(0xC0);
    }

    // Initial GDDRAM Clear
    SH1106_Clear();
    SH1106_Update_Screen();

    // Display ON
    SH1106_Write_Command(0xAF);
}

void SH1106_Update_Screen(void) 
{
    for (uint8_t i = 0; i < 8; i++) {
        SH1106_Write_Command(0xB0 + i); // Page address
        SH1106_Write_Command(0x02); // Lower column offset
        SH1106_Write_Command(0x10); // Higher column offset

        // Dumps the content of the buffer to the display page by page 
        SH1106_Write_Data(&SH1106_Buffer[SH1106_WIDTH * i], SH1106_WIDTH);
    }
}

void SH1106_Clear(void) 
{
    memset(SH1106_Buffer, 0x00, SH1106_BUFFER_SIZE);
}

void SH1106_Draw_Text(uint8_t x, uint8_t y, SH1106_Pixel_State_t state, const SH1106_Font_t *font, const char *text)
{
    for (uint8_t i = 0; i < strlen(text); i++) {
        uint8_t offset = i * (font->width + 1);
        SH1106_Write_Letter(text[i], x + offset, y, state, font);
    }
}

void SH1106_Draw_Pixel(uint8_t x, uint8_t y, SH1106_Pixel_State_t state) 
{
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return; // out of range

    if (state == SH1106_PIXEL_ON)
        SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] |= (1 << (y % 8));
    else
        SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y % 8));
}

// column major, MSB
void SH1106_Draw_Bitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height, SH1106_Pixel_State_t state)
{
    uint8_t bytes_per_row = (width + 7) / 8;

    for (uint8_t col = 0; col < width; col++) {
        for (uint8_t row = 0; row < height; row++) {
            uint8_t index = (col * bytes_per_row) + (row / 8);
            uint8_t is_on = bitmap[index] & (0x80 >> (row % 8));
            if (is_on)
                SH1106_Draw_Pixel(x + row, y + col, state);
        }
    }
}

void SH1106_Draw_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_Pixel_State_t state)
{
    // x-y diffs
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    
    // left->right or right->left
    // up->down or down->up
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    
    // error accumulator
    int err = dx - dy;

    // init local vars
    int x = x1;
    int y = y1;

    while (1) {
        SH1106_Draw_Pixel(x, y, state);

        // if the destination is reached
        if (x == x2 && y == y2) break;

        // temporary error variable
        int e2 = 2 * err;

        // check if we need to step along the X axis
        if (e2 > -dy) {
            err -= dy; // adjust error based on the Y-axis distance
            x += sx;   // move x in the determined direction (left or right)
        }
        
        // check if we need to step along the Y axis
        if (e2 < dx) {
            err += dx; // adjust error based on the X-axis distance
            y += sy;   // move y in the determined direction (up or down)
        }
    }
}

void SH1106_Draw_Rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, SH1106_Pixel_State_t state)
{
    if (width == 0 || height == 0) return;

    uint8_t x_end = x + width - 1;
    uint8_t y_end = y + height - 1;

    for (uint8_t i = x; i <= x_end; i++) {
        SH1106_Draw_Pixel(i, y, state);
        SH1106_Draw_Pixel(i, y_end, state);
    }

    for (uint8_t j = y + 1; j < y_end; j++) {
        SH1106_Draw_Pixel(x, j, state);
        SH1106_Draw_Pixel(x_end, j, state);
    }
}
