/**
 * @file    tasks.c
 * @author  Jose
 * @brief   FreeRTOS tasks creation
 * @date    15/9/26
 * @project tfg
 */

#include "tasks.h"
#include "task.h"
#include "tsl2591.h"
#include "sh1106.h"
#include "ui.h"
#include "util.h"

/* Queue definition */
#define SENSOR_QUEUE_LENGTH 8
#define SENSOR_QUEUE_ITEM_SIZE sizeof(SensorDataMsg_t)

QueueHandle_t xSensorQueue = NULL;
TaskHandle_t uiTaskHandle;
TaskHandle_t tsl2591TaskHandle;

bool Tasks_Init(TSL2591_Handle_t *tsl_h, SH1106_Handle_t *disp_h)
{
    if (tsl_h == NULL || disp_h == NULL) return false;
    xSensorQueue = xQueueCreate(SENSOR_QUEUE_LENGTH, SENSOR_QUEUE_ITEM_SIZE);
    if (xSensorQueue == NULL) return false;

    BaseType_t ret;

    ret = xTaskCreate(
        TSL2591_Measure_Task,
        "TSL2591 Task",
        256,
        tsl_h,
        2,
        &tsl2591TaskHandle
    );
    if (ret != pdPASS) return false;

    ret = xTaskCreate(
        UI_Task,
        "UI Task",
        512,
        disp_h,
        1,
        &uiTaskHandle
    );
    if (ret != pdPASS) return false;

    return true;
}

void TSL2591_Measure_Task(void *pvParameters)
{
    TSL2591_Handle_t *tsl_h = (TSL2591_Handle_t *)pvParameters;
    if (tsl_h == NULL) vTaskDelete(NULL);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);

    uint16_t ch0 = 0, ch1 = 0;
    SensorDataMsg_t msg;
    msg.type = SENSOR_TYPE_TSL2591;

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        if (TSL2591_Read_Raw_Data(tsl_h, &ch0, &ch1) == HAL_OK) {
            msg.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            msg.data.tsl2591.ch0_raw = ch0;
            msg.data.tsl2591.ch1_raw = ch1;
            msg.data.tsl2591.lux = TSL2591_Calculate_Lux(tsl_h, ch0, ch1);

            xQueueSend(xSensorQueue, &msg, 0);
        }
    }
}

void SHT40_Measure_Task(void *pvParameters)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void UI_Task(void *pvParameters)
{
    SH1106_Handle_t *disp_h = (SH1106_Handle_t *)pvParameters;
    if (disp_h == NULL) vTaskDelete(NULL);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFramePeriod = pdMS_TO_TICKS(66);

    /* Cached latest data */
    float last_temp = 0.0f;     // TODO real temperature
    float last_hum = 0.0f;      // TODO real humidity
    float last_lux = 0.0f;
    uint8_t battery_pct = 85;   // TODO real battery level
    bool lora_connected = true; // TODO real connection status

    SensorDataMsg_t msg;

    UI_Container_t root_screen = {0, 0, 128, 64, 2};
    UI_Container_t status_bar_view, main_view;

    for (;;) {
        while (xQueueReceive(xSensorQueue, &msg, 0) == pdPASS) {
            switch (msg.type) {
                case SENSOR_TYPE_TSL2591:
                    last_lux = msg.data.tsl2591.lux;
                    break;

                case SENSOR_TYPE_SHT40:
                    last_temp = msg.data.sht40.temperature;
                    last_hum = msg.data.sht40.humidity;
                    break;

                default:
                    break;
            }
        }

        SH1106_Clear(disp_h);

        UI_Container_Split_V(&root_screen, &status_bar_view, &main_view, 18);
        UI_Draw_Status_Bar(disp_h, &status_bar_view, "Croppy v1.0", battery_pct, lora_connected);

        FloatSplit_t hum_split = Utils_Split_Float(last_hum, 1);
        UI_Container_Draw_Icon(disp_h, &main_view, 0, 2, ICON_15x15_HUMIDITY, SH1106_PIXEL_ON);
        UI_Container_Draw_Text(disp_h, &main_view, 17, 2, SH1106_PIXEL_ON, &Font_10x16,
                      "Hum:  %d.%u%%", hum_split.integer_part, hum_split.decimal_part);

        FloatSplit_t temp_split = Utils_Split_Float(last_temp, 1);
        UI_Container_Draw_Icon(disp_h, &main_view, 0, 18, ICON_15x15_TEMPERATURE, SH1106_PIXEL_ON);
        UI_Container_Draw_Text(disp_h, &main_view, 17, 18, SH1106_PIXEL_ON, &Font_10x16,
                      "Temp: %s%d.%uC", temp_split.is_negative ? "-" : "",
                      temp_split.integer_part, temp_split.decimal_part);

        FloatSplit_t lux_split = Utils_Split_Float(last_lux, 0);
        UI_Container_Draw_Icon(disp_h, &main_view, 0, 34, ICON_15x15_PH, SH1106_PIXEL_ON); // TODO make lux icon
        UI_Container_Draw_Text(disp_h, &main_view, 17, 34, SH1106_PIXEL_ON, &Font_10x16,
                      "Lux:  %u", lux_split.integer_part);

        (void)SH1106_Update_Screen(disp_h);

        vTaskDelayUntil(&xLastWakeTime, xFramePeriod);
    }
}