/**
 * @file    util.h
 * @author  Jose
 * @date    17/9/26
 * @project tfg
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool is_negative;
    int32_t integer_part;
    uint32_t decimal_part;
} FloatSplit_t;

/**
 * @brief Separates a float into its integer and decimal parts
 * @param value Float input
 * @param precision Desired decimal positions
 * @return FloatSplit_t with calculated sign, integer and decimal part
 */
FloatSplit_t Utils_Split_Float(float value, uint8_t precision);

#endif //UTIL_H
