/**
 * @file    tsl2591.c
 * @author  Jose
 * @brief   Driver library for TSL2591 light-to-digital sensor using I2C for STM32.
 * @date    14/9/26
 * @project tfg
 */

#include "tsl2591.h"
#include <stdint.h>

#define TSL2591_I2C_TIMEOUT 100

static I2C_HandleTypeDef *hi2c_dev = NULL;
static uint16_t dev_addr = 0;
static TSL2591_Gain_t current_gain = TSL2591_GAIN_MED;
static TSL2591_IntegrationTime_t current_it = TSL2591_INTEGRATIONTIME_100MS;

/* Private low-level bus communication helpers */
static HAL_StatusTypeDef TSL2591_Write_Register(uint8_t reg, uint8_t value);
static HAL_StatusTypeDef TSL2591_Read_Register(uint8_t reg, uint8_t *value);
static HAL_StatusTypeDef TSL2591_Read_Data(uint8_t start_reg, uint8_t *buffer, uint16_t size);

/**
 * @brief Writes a byte to a TSL2591 internal register using Normal Operation command format.
 */
static HAL_StatusTypeDef TSL2591_Write_Register(uint8_t reg, uint8_t value)
{
    uint8_t cmd_header = TSL2591_CMD_BIT | TSL2591_CMD_TRANSACTION_NORM | (reg & 0x1F);
    return HAL_I2C_Mem_Write(hi2c_dev, dev_addr, cmd_header, I2C_MEMADD_SIZE_8BIT, &value, 1, TSL2591_I2C_TIMEOUT);
}

/**
 * @brief Reads a single byte from a TSL2591 internal register.
 */
static HAL_StatusTypeDef TSL2591_Read_Register(uint8_t reg, uint8_t *value)
{
    uint8_t cmd_header = TSL2591_CMD_BIT | TSL2591_CMD_TRANSACTION_NORM | (reg & 0x1F);
    return HAL_I2C_Mem_Read(hi2c_dev, dev_addr, cmd_header, I2C_MEMADD_SIZE_8BIT, value, 1, TSL2591_I2C_TIMEOUT);
}

/**
 * @brief Reads a sequential burst of bytes starting at `start_reg`.
 */
static HAL_StatusTypeDef TSL2591_Read_Data(uint8_t start_reg, uint8_t *buffer, uint16_t size)
{
    uint8_t cmd_header = TSL2591_CMD_BIT | TSL2591_CMD_TRANSACTION_NORM | (start_reg & 0x1F);
    return HAL_I2C_Mem_Read(hi2c_dev, dev_addr, cmd_header, I2C_MEMADD_SIZE_8BIT, buffer, size, TSL2591_I2C_TIMEOUT);
}

HAL_StatusTypeDef TSL2591_Init(const TSL2591_Config_t *config)
{
    if (config == NULL || config->hi2c == NULL) return HAL_ERROR;

    hi2c_dev = config->hi2c;
    dev_addr = config->i2c_addr;

    uint8_t id = 0;

    if (TSL2591_Read_Register(TSL2591_REG_ID, &id) != HAL_OK) return HAL_ERROR;
    if (id != TSL2591_DEVICE_ID) return HAL_ERROR;
    if (TSL2591_Set_Timing_Gain(config->gain, config->integration_time) != HAL_OK) return HAL_ERROR;

    return TSL2591_Enable();
}

HAL_StatusTypeDef TSL2591_Enable(void)
{
    /* PON enables the oscillator; AEN enables the two integrating ADCs */
    uint8_t enable_val = TSL2591_ENABLE_PON | TSL2591_ENABLE_AEN;
    return TSL2591_Write_Register(TSL2591_REG_ENABLE, enable_val);
}

HAL_StatusTypeDef TSL2591_Disable(void)
{
    return TSL2591_Write_Register(TSL2591_REG_ENABLE, TSL2591_ENABLE_POWEROFF);
}

HAL_StatusTypeDef TSL2591_Set_Timing_Gain(TSL2591_Gain_t gain, TSL2591_IntegrationTime_t it)
{
    current_gain = gain;
    current_it = it;

    uint8_t config_val = (uint8_t)gain | (uint8_t)it;
    return TSL2591_Write_Register(TSL2591_REG_CONFIG, config_val);
}

bool TSL2591_Is_Data_Ready(void)
{
    uint8_t status = 0;
    if (TSL2591_Read_Register(TSL2591_REG_STATUS, &status) != HAL_OK) return false;
    return (status & TSL2591_STATUS_AVALID) != 0;
}

HAL_StatusTypeDef TSL2591_Read_Raw_Data(uint16_t *ch0, uint16_t *ch1)
{
    if (ch0 == NULL || ch1 == NULL) return HAL_ERROR;

    uint8_t buffer[4];

    /* Reads C0DATAL, C0DATAH, C1DATAL and C1DATAH in a single 4-byte burst */
    if (TSL2591_Read_Data(TSL2591_REG_C0DATAL, buffer, 4) != HAL_OK) return HAL_ERROR;

    *ch0 = ((uint16_t)buffer[1] << 8) | buffer[0];
    *ch1 = ((uint16_t)buffer[3] << 8) | buffer[2];

    return HAL_OK;
}

float TSL2591_Calculate_Lux(uint16_t ch0, uint16_t ch1)
{
    /* Check for sensor saturation */
    uint16_t max_count = (current_it == TSL2591_INTEGRATIONTIME_100MS) ? 37888 : 65535;
    if (ch0 >= max_count || ch1 >= max_count) return -1.0f; /* Overflow / saturated */

    float atime_ms = 100.0f * (float)(current_it + 1);

    float again = 1.0f;
    switch (current_gain) {
        case TSL2591_GAIN_LOW:  again = 1.0f;    break;
        case TSL2591_GAIN_MED:  again = 25.0f;   break;
        case TSL2591_GAIN_HIGH: again = 428.0f;  break;
        case TSL2591_GAIN_MAX:  again = 9876.0f; break;
        default: break;
    }

    /* Standard lux equation scaling factors for TSL2591 (DF = 408.0f) */
    float cpl = (atime_ms * again) / 408.0f;
    if (cpl == 0.0f) return 0.0f;

    float lux = (((float)ch0 - (float)ch1)) * (1.0f - ((float)ch1 / (float)ch0)) / cpl;

    return (lux < 0.0f) ? 0.0f : lux;
}
