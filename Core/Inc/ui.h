/**
 * @file    ui.h
 * @author  Jose
 * @brief   Mini UI framework for SH1106 screen using STM32
 * @date    20/7/26
 * @project tfg
 */

#ifndef UI_H
#define UI_H

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "sh1106.h"
#include "font_10x16.h"
#include "font_5x8.h"
#include "icons.h"

extern const SH1106_Font_t Font_10x16;
extern const SH1106_Font_t Font_5x8;

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t w;
  uint8_t h;
  uint8_t padding;
} UI_Container_t;

typedef enum {
  // 15x15
  ICON_15x15_HUMIDITY = 0,
  ICON_15x15_NPK,
  ICON_15x15_PH,
  ICON_15x15_TEMPERATURE,

  // 7x7
  ICON_7x7_BATTERY_EMPTY = 100, 
  ICON_7x7_BATTERY_LOW,
  ICON_7x7_BATTERY_HALF,
  ICON_7x7_BATTERY_ALMOST_FULL,
  ICON_7x7_BATTERY_FULL,
  ICON_7x7_NET_CONNECTED,
  ICON_7x7_NET_DISCONNECTED,
} UI_IconIndex_t;

void UI_Container_Split_H(const UI_Container_t *parent, UI_Container_t *left, UI_Container_t *right, uint8_t split_pct);
void UI_Container_Split_V(const UI_Container_t *parent, UI_Container_t *top, UI_Container_t *bottom, uint8_t split_pct);
void UI_Container_Draw_Text(const UI_Container_t *c, uint8_t rel_x, uint8_t rel_y, SH1106_Pixel_State_t state, const SH1106_Font_t *font, const char *format, ...);
void UI_Container_Draw_Button(const UI_Container_t *c, uint8_t rel_x, uint8_t rel_y, uint8_t width, uint8_t height, const char* label, bool is_focused);
void UI_Container_Draw_Icon(const UI_Container_t *c, uint8_t rel_x, uint8_t rel_y, UI_IconIndex_t icon, SH1106_Pixel_State_t state);
void UI_Draw_Status_Bar(const UI_Container_t *c, const char *title, uint8_t battery_level, bool connected);

#endif // UI_H