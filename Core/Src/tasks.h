/**
* @file    tasks.c
 * @author  Jose
 * @brief   FreeRTOS tasks creation
 * @date    15/9/26
 * @project tfg
 */

#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"

typedef enum {
 SENSOR_TYPE_TSL2591 = 0,
 SENSOR_TYPE_SHT40
} SensorType_t;

typedef struct {
 SensorType_t type;
 uint32_t timestamp_ms;
 union {
  struct {
   float lux;
   uint16_t ch0_raw;
   uint16_t ch1_raw;
  } tsl2591;
  struct {
   float temperature;
   float humidity;
  } sht40;
 } data;
} SensorDataMsg_t;

extern QueueHandle_t xSensorQueue;

void TSL2591_Measure_Task(void *pvParameters);
void SHT40_Measure_Task(void *pvParameters);
void UI_Task(void *pvParameters);

bool Tasks_Init(void);

#endif //TASKS_H
