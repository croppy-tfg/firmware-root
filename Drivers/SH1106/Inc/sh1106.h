/**
 * @file    sh1106.h
 * @author  Jose
 * @brief   Driver library for SH1106 OLED display (128x64) using I2C for STM32F1.
 * @date    13/9/26
 * @project tfg
 */

#ifndef SH1106_H
#define SH1106_H

#define SH1106_COMMAND_ADDR         0x00
#define SH1106_DATA_ADDR            0x40
#define SH1106_WIDTH                128
#define SH1106_HEIGHT               64
#define SH1106_BUFFER_SIZE          (SH1106_WIDTH * SH1106_HEIGHT / 8)
#define SH1106_I2C_TIMEOUT          100

#include <stdint.h>
#include "stm32f4xx_hal.h"

typedef enum {
    SH1106_PIXEL_OFF = 0x00,
    SH1106_PIXEL_ON = 0x01
} SH1106_Pixel_State_t;

typedef enum {
    SH1106_MIRROR_NONE = 0,
    SH1106_MIRROR_HORIZONTAL = 0x01,
    SH1106_MIRROR_VERTICAL   = 0x02,
    SH1106_MIRROR_BOTH       = 0x03
} SH1106_MirrorMode_t;

typedef enum {
    SH1106_PUMP_6_4V = 0x30,
    SH1106_PUMP_7_4V = 0x31,
    SH1106_PUMP_8_0V = 0x32, // POR
    SH1106_PUMP_9_0V = 0x33
} SH1106_PumpVoltage_t;

typedef enum {
    SH1106_COM_SEQUENTIAL = 0x02,
    SH1106_COM_ALTERNATIVE  = 0x12 // POR
} SH1106_ComConfig_t;

typedef enum {
    SH1106_FONT_5x8 = 0,
    SH1106_FONT_10x16 = 0x01
} SH1106_Font_Size_t;

typedef struct {
    const void *map;
    uint8_t width;
    uint8_t height;
    SH1106_Font_Size_t size;
} SH1106_Font_t;

typedef struct {
    I2C_HandleTypeDef    *hi2c;                 // pointer to I2C handle
    uint16_t             i2c_addr;              // I2C address
    uint8_t              contrast;              // constrast [0x00, 0xFF]
    uint8_t              multiplex_ratio;       // multiplex ratio [0x00, 0x3F]
    uint8_t              display_offset;        // vertical offset [0x00, 0x3F]
    uint8_t              start_line;            // start line [0x40, 0x7F]
    uint8_t              osc_freq_div_ratio;    // clock divide ratio
    SH1106_PumpVoltage_t pump_voltage;          // internal DC-DC voltage 
    SH1106_ComConfig_t   com_pins_config;       // COM pins (rows) configuration
    SH1106_MirrorMode_t  mirror;                // H/V mirror
} SH1106_Config_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint16_t i2c_addr;
    uint8_t buffer[SH1106_BUFFER_SIZE];
} SH1106_Handle_t;

/**
  * @brief Initializes the display sending the appropiate I2C commands
  *
  * @param None
  * @retval None
  */
HAL_StatusTypeDef SH1106_Init(SH1106_Handle_t *h, const SH1106_Config_t *config);

/**
  * @brief Writes the content of the frame buffer into de GDDRAM
  *
  * @param None
  * @retval None
  */
HAL_StatusTypeDef SH1106_Update_Screen(SH1106_Handle_t *h);

/**
  * @brief Clears the display
  *
  * @param None
  * @retval None
  */
void SH1106_Clear(SH1106_Handle_t *h);

/**
 * @brief Writes a formatted string from a font map into the display at the given coordinates.
 *
 * @param x: X
 * @param y: Y
 * @param format: the formatted text to write
 * @retval None
 */
void SH1106_Draw_Text(SH1106_Handle_t *h, uint8_t x, uint8_t y, SH1106_Pixel_State_t state, const SH1106_Font_t *font, const char *text);

/**
  * @brief Turns on/off the selected pixel.
  *        The byte index in the buffer is calculated using:
  *        
  *        index = x + (y / 8) * 128
  *
  *        Meaning: The page number (y / 8, as each page is 8 pixels high) 
  *        is multiplied by the page width (128 bytes) to skip all previous 
  *        pages. Then, the column offset (x) is added to find the exact byte,
  *        then the exact bit is selected using mod 8.
  *
  * @param x: column [0, 127]
  * @param y: row [0, 31]
  * @param state: SH1106_PIXEL_ON (0x01) or SH1106_PIXEL_OFF (0x00)
  * @retval None
  */
void SH1106_Draw_Pixel(SH1106_Handle_t *h, uint8_t x, uint8_t y, SH1106_Pixel_State_t state);

/**
  * @brief Draws a monochrome bitmap on the screen
  *
  * @param x: starting column [0, 127]
  * @param y: starting row [0, 31]
  * @param bitmap: pointer to the image array
  * @param width: image width in pixels
  * @param height: image height in pixels
  * @retval None
  */
void SH1106_Draw_Bitmap(SH1106_Handle_t *h, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height, SH1106_Pixel_State_t state);

/**
 * @brief Draws a line wether it is X/Y only or it has
 *        a slope, using the Bresenham algorithm
 * @param x1: X1
 * @param y1: Y1
 * @param x2: X2
 * @param y2: Y2
 * @param state: SH1106_PIXEL_ON/SH1106_PIXEL_OFF
 */
void SH1106_Draw_Line(SH1106_Handle_t *h, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_Pixel_State_t state);

/**
 * @brief Draws a rectangle of a given dimensions from (x, y)
 *
 * @param x: X start
 * @param y: Y start
 * @param width: X + width
 * @param height: Y + width
 * @param state: SH1106_PIXEL_ON/SH1106_PIXEL_OFF
 */
void SH1106_Draw_Rectangle(SH1106_Handle_t *h, uint8_t x, uint8_t y, uint8_t width, uint8_t height, SH1106_Pixel_State_t state);

#endif // SH1106_H