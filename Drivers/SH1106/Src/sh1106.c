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

static HAL_StatusTypeDef SH1106_Write_Command(SH1106_Handle_t *h, uint8_t command);
static HAL_StatusTypeDef SH1106_Write_Data(SH1106_Handle_t *h, uint8_t *data, uint16_t size);
static void SH1106_Write_Letter(SH1106_Handle_t *h, uint8_t index, uint8_t x, uint8_t y, SH1106_Pixel_State_t state, const SH1106_Font_t *font);

/**
  * @brief Sends a command to the display via I2C
  *
  * @param command: command to be sent
  * @retval None
  */
static HAL_StatusTypeDef SH1106_Write_Command(SH1106_Handle_t *h, uint8_t command)
{
    if (h == NULL || h->hi2c == NULL) return HAL_ERROR;
    return HAL_I2C_Mem_Write(h->hi2c, h->i2c_addr, SH1106_COMMAND_ADDR, 1, &command, 1, SH1106_I2C_TIMEOUT);
}

/**
  * @brief Sends I2C data to the display
  *
  * @param data: the address of the first byte
  * @param size: the number of bytes from data
  * @retval None
  */
static HAL_StatusTypeDef SH1106_Write_Data(SH1106_Handle_t *h, uint8_t *data, uint16_t size)
{
    if (h == NULL || h->hi2c == NULL || data == NULL) return HAL_ERROR;
    return HAL_I2C_Mem_Write(h->hi2c, h->i2c_addr, SH1106_DATA_ADDR, 1, data, size, SH1106_I2C_TIMEOUT);
}

/**
  * @brief Writes a letter from a font map into the display.
  *
  * @param index: the position of the letter in the map
  * @param x: column [0, 127]
  * @param y: row [0, 31]
  * @retval None
  */
static void SH1106_Write_Letter(SH1106_Handle_t *h, uint8_t index, uint8_t x, uint8_t y, SH1106_Pixel_State_t state, const SH1106_Font_t *font)
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
                SH1106_Draw_Pixel(h, x + j, y + i, state);
        }
    }
}

HAL_StatusTypeDef SH1106_Init(SH1106_Handle_t *h, const SH1106_Config_t *config)
{
    if (h == NULL || config == NULL || config->hi2c == NULL) return HAL_ERROR;
    h->hi2c = config->hi2c;
    h->i2c_addr = config->i2c_addr;
    memset(h->buffer, 0x00, SH1106_BUFFER_SIZE);

    HAL_Delay(100);

    // Display OFF
    if (SH1106_Write_Command(h, 0xAE) != HAL_OK) return HAL_ERROR;

    // Display Clock Divide Ratio
    if (SH1106_Write_Command(h, 0xD5) != HAL_OK) return HAL_ERROR;
    if (SH1106_Write_Command(h, config->osc_freq_div_ratio) != HAL_OK) return HAL_ERROR;

    // Multiplex Ratio
    if (SH1106_Write_Command(h, 0xA8) != HAL_OK) return HAL_ERROR;
    if (SH1106_Write_Command(h, config->multiplex_ratio) != HAL_OK) return HAL_ERROR;

    // Display Offset
    if (SH1106_Write_Command(h, 0xD3) != HAL_OK) return HAL_ERROR;
    if (SH1106_Write_Command(h, config->display_offset) != HAL_OK) return HAL_ERROR;

    // Start line
    if (SH1106_Write_Command(h, config->start_line) != HAL_OK) return HAL_ERROR;

    // DC-DC Pump
    if (SH1106_Write_Command(h, 0xAD) != HAL_OK) return HAL_ERROR;
    if (SH1106_Write_Command(h, config->pump_voltage) != HAL_OK) return HAL_ERROR;

    // COM Pins (rows) Config
    if (SH1106_Write_Command(h, 0xDA) != HAL_OK) return HAL_ERROR;
    if (SH1106_Write_Command(h, config->com_pins_config) != HAL_OK) return HAL_ERROR;

     // Contrast Control
    if (SH1106_Write_Command(h, 0x81) != HAL_OK) return HAL_ERROR;
    if (SH1106_Write_Command(h, config->contrast) != HAL_OK) return HAL_ERROR;

    // Pre-charge Period
    if (SH1106_Write_Command(h, 0xD9) != HAL_OK) return HAL_ERROR;
    if (SH1106_Write_Command(h, 0x22) != HAL_OK) return HAL_ERROR;

    // VCOMH Deselect Level
    if (SH1106_Write_Command(h, 0xDB) != HAL_OK) return HAL_ERROR;
    if (SH1106_Write_Command(h, 0x35) != HAL_OK) return HAL_ERROR;

    // Resume content from GDDRAM
    if (SH1106_Write_Command(h, 0xA4) != HAL_OK) return HAL_ERROR;

    // Set normal display
    if (SH1106_Write_Command(h, 0xA6) != HAL_OK) return HAL_ERROR;

    // Mirror H
    if (config->mirror & SH1106_MIRROR_HORIZONTAL) {
        if (SH1106_Write_Command(h, 0xA1) != HAL_OK) return HAL_ERROR;
    } else {
        if (SH1106_Write_Command(h, 0xA0) != HAL_OK) return HAL_ERROR;
    }

    // Mirror V
    if (config->mirror & SH1106_MIRROR_VERTICAL) {
        if (SH1106_Write_Command(h, 0xC8) != HAL_OK) return HAL_ERROR;
    } else {
        if (SH1106_Write_Command(h, 0xC0) != HAL_OK) return HAL_ERROR;
    }

    // Initial GDDRAM Clear
    SH1106_Clear(h);
    if (SH1106_Update_Screen(h) != HAL_OK) return HAL_ERROR;

    // Display ON
    if (SH1106_Write_Command(h, 0xAF) != HAL_OK) return HAL_ERROR;
    return HAL_OK;
}

HAL_StatusTypeDef SH1106_Update_Screen(SH1106_Handle_t *h)
{
    if (h == NULL) return HAL_ERROR;
    for (uint8_t i = 0; i < 8; i++) {
        if (SH1106_Write_Command(h, 0xB0 + i) != HAL_OK) return HAL_ERROR;
        if (SH1106_Write_Command(h, 0x02) != HAL_OK) return HAL_ERROR;
        if (SH1106_Write_Command(h, 0x10) != HAL_OK) return HAL_ERROR;

        // Dumps the content of the buffer to the display page by page 
        if (SH1106_Write_Data(h, &h->buffer[SH1106_WIDTH * i], SH1106_WIDTH) != HAL_OK) return HAL_ERROR;
    }
    return HAL_OK;
}

void SH1106_Clear(SH1106_Handle_t *h)
{
    if (h == NULL) return;
    memset(h->buffer, 0x00, SH1106_BUFFER_SIZE);
}

void SH1106_Draw_Text(SH1106_Handle_t *h, uint8_t x, uint8_t y, SH1106_Pixel_State_t state, const SH1106_Font_t *font, const char *text)
{
    if (h == NULL || font == NULL || text == NULL) return;
    for (uint8_t i = 0; i < strlen(text); i++) {
        uint8_t offset = i * (font->width + 1);
        SH1106_Write_Letter(h, text[i], x + offset, y, state, font);
    }
}

void SH1106_Draw_Pixel(SH1106_Handle_t *h, uint8_t x, uint8_t y, SH1106_Pixel_State_t state)
{
    if (h == NULL) return;
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return; // out of range

    if (state == SH1106_PIXEL_ON)
        h->buffer[x + (y / 8) * SH1106_WIDTH] |= (1 << (y % 8));
    else
        h->buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y % 8));
}

// column major, MSB
void SH1106_Draw_Bitmap(SH1106_Handle_t *h, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height, SH1106_Pixel_State_t state)
{
    if (h == NULL || bitmap == NULL) return;
    uint8_t bytes_per_row = (width + 7) / 8;

    for (uint8_t col = 0; col < width; col++) {
        for (uint8_t row = 0; row < height; row++) {
            uint8_t index = (col * bytes_per_row) + (row / 8);
            uint8_t is_on = bitmap[index] & (0x80 >> (row % 8));
            if (is_on)
                SH1106_Draw_Pixel(h, x + row, y + col, state);
        }
    }
}

void SH1106_Draw_Line(SH1106_Handle_t *h, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_Pixel_State_t state)
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
        SH1106_Draw_Pixel(h, x, y, state);

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

void SH1106_Draw_Rectangle(SH1106_Handle_t *h, uint8_t x, uint8_t y, uint8_t width, uint8_t height, SH1106_Pixel_State_t state)
{
    if (width == 0 || height == 0) return;

    uint8_t x_end = x + width - 1;
    uint8_t y_end = y + height - 1;

    for (uint8_t i = x; i <= x_end; i++) {
        SH1106_Draw_Pixel(h, i, y, state);
        SH1106_Draw_Pixel(h, i, y_end, state);
    }

    for (uint8_t j = y + 1; j < y_end; j++) {
        SH1106_Draw_Pixel(h, x, j, state);
        SH1106_Draw_Pixel(h, x_end, j, state);
    }
}
