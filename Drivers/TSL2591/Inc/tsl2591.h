/**
 * @file    tsl2591.h
 * @author  Jose
 * @brief   Driver library for TSL2591 light-to-digital sensor using I2C for STM32.
 * @date    14/9/26
 * @project tfg
 */

#ifndef TSL2591_H
#define TSL2591_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#define TSL2591_ADDRESS (0x29 << 1)

/* Command Register masks */
#define TSL2591_CMD_BIT              0x80  /* Bit 7 must be '1' */
#define TSL2591_CMD_TRANSACTION_NORM 0x20  /* Normal Operation: 10 -> [6:5] */
#define TSL2591_CMD_TRANSACTION_SPEC 0x60  /* Special Functions: 11 -> [6:5] */

/* Register Addresses */
#define TSL2591_REG_ENABLE           0x00
#define TSL2591_REG_CONFIG           0x01
#define TSL2591_REG_AILTL            0x04
#define TSL2591_REG_AILTH            0x05
#define TSL2591_REG_AIHTL            0x06
#define TSL2591_REG_AIHTH            0x07
#define TSL2591_REG_PERSIST          0x0C
#define TSL2591_REG_PID              0x11
#define TSL2591_REG_ID               0x12
#define TSL2591_REG_STATUS           0x13
#define TSL2591_REG_C0DATAL          0x14
#define TSL2591_REG_C0DATAH          0x15
#define TSL2591_REG_C1DATAL          0x16
#define TSL2591_REG_C1DATAH          0x17

/* Expected Device ID */
#define TSL2591_DEVICE_ID            0x50

/* ENABLE Register Masks */
#define TSL2591_ENABLE_NPIEN         0x80
#define TSL2591_ENABLE_SAI           0x40
#define TSL2591_ENABLE_AIEN          0x10
#define TSL2591_ENABLE_AEN           0x02
#define TSL2591_ENABLE_PON           0x01
#define TSL2591_ENABLE_POWEROFF      0x00

/* STATUS Register Masks */
#define TSL2591_STATUS_AVALID        0x01
#define TSL2591_STATUS_AINT          0x10
#define TSL2591_STATUS_NPINTR        0x20

/* Special Function Commands */
#define TSL2591_SPEC_CLEAR_ALS_INT   0x06

typedef enum {
    TSL2591_GAIN_LOW  = 0x00, /* 1x Gain */
    TSL2591_GAIN_MED  = 0x10, /* 25x Gain */
    TSL2591_GAIN_HIGH = 0x20, /* 428x Gain */
    TSL2591_GAIN_MAX  = 0x30  /* 9876x Gain */
} TSL2591_Gain_t;

typedef enum {
    TSL2591_INTEGRATIONTIME_100MS = 0x00, /* 100 ms (Max count: 37888) */
    TSL2591_INTEGRATIONTIME_200MS = 0x01, /* 200 ms (Max count: 65535) */
    TSL2591_INTEGRATIONTIME_300MS = 0x02, /* 300 ms (Max count: 65535) */
    TSL2591_INTEGRATIONTIME_400MS = 0x03, /* 400 ms (Max count: 65535) */
    TSL2591_INTEGRATIONTIME_500MS = 0x04, /* 500 ms (Max count: 65535) */
    TSL2591_INTEGRATIONTIME_600MS = 0x05  /* 600 ms (Max count: 65535) */
} TSL2591_IntegrationTime_t;

typedef struct {
    I2C_HandleTypeDef          *hi2c;
    uint16_t                    i2c_addr;
    TSL2591_Gain_t              gain;
    TSL2591_IntegrationTime_t   integration_time;
} TSL2591_Config_t;

typedef struct {
    I2C_HandleTypeDef          *hi2c;
    uint16_t                    addr;
    TSL2591_Gain_t              gain;
    TSL2591_IntegrationTime_t   integration_time;
} TSL2591_Handle_t;

/**
 * @brief Initializes the TSL2591 sensor and verifies its device ID.
 * @param config Pointer to configuration struct.
 * @retval HAL_OK on success, HAL_ERROR or timeout otherwise.
 */
HAL_StatusTypeDef TSL2591_Init(TSL2591_Handle_t *h, const TSL2591_Config_t *config);

/**
 * @brief Powers ON internal oscillator and enables ALS engine.
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef TSL2591_Enable(TSL2591_Handle_t *h);

/**
 * @brief Powers OFF internal oscillator and ALS engine to conserve energy.
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef TSL2591_Disable(TSL2591_Handle_t *h);

/**
 * @brief Configures sensor gain and integration time.
 * @param gain Sensor analog gain.
 * @param it Integration time.
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef TSL2591_Set_Timing_Gain(TSL2591_Handle_t *h, TSL2591_Gain_t gain, TSL2591_IntegrationTime_t it);

/**
 * @brief Checks if a measurement conversion has finished.
 * @retval true if data is ready, false otherwise.
 */
bool TSL2591_Is_Data_Ready(TSL2591_Handle_t *h);

/**
 * @brief Reads the raw ADC data of both photodiode channels simultaneously.
 * @param ch0 Pointer to store Channel 0 (Visible + IR) raw data.
 * @param ch1 Pointer to store Channel 1 (IR) raw data.
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef TSL2591_Read_Raw_Data(TSL2591_Handle_t *h, uint16_t *ch0, uint16_t *ch1);

/**
 * @brief Calculates ambient light level in Lux based on raw ADC values.
 * @param ch0 Raw ADC count for Channel 0.
 * @param ch1 Raw ADC count for Channel 1.
 * @retval Calculated illuminance in Lux (float).
 */
float TSL2591_Calculate_Lux(TSL2591_Handle_t *h, uint16_t ch0, uint16_t ch1);

#endif // TSL2591_H
