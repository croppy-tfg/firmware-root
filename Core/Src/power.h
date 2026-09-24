/**
 * @file    power.h
 * @author  Jose
 * @date    23/9/26
 * @brief   Energy saving handling
 * @project tfg
 */

#ifndef POWER_H
#define POWER_H

#include <stdint.h>

/**
 * @brief Make the chip enter STOP mode
 * @param seconds The seconds the chip is going to stay in STOP mode
 * @return void
 */
void Power_Enter_Stop_Mode(uint32_t seconds);

#endif //POWER_H
