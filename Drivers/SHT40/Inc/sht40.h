/**
 * @file    sht40.h
 * @author  Jose
 * @brief   Driver library for SHT40 temperature and humidity sensor
 *          using I2C for STM32F1.
 * @date    14/9/26
 * @project tfg
 */

#ifndef SHT40_H
#define SHT40_H

#define SHT40_ADDR 0x44

void SHT40_Init(void);


#endif // SHT40_H
