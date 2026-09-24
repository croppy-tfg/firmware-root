/**
 * @file    util.c
 * @author  Jose
 * @date    17/9/26
 * @project firmware-root
 */

#include "util.h"
#include <math.h>

FloatSplit_t Util_Split_Float(float value, uint8_t precision)
{
    FloatSplit_t result;
    result.is_negative = (value < 0.0f);

    float abs_val = fabsf(value);

    static const float scales[] = {1.0f, 10.0f, 100.0f, 1000.0f, 10000.0f};
    if (precision > 4) {
        precision = 4;
    }
    float scale = scales[precision];

    float rounded_val = roundf(abs_val * scale) / scale;

    result.integer_part = (int32_t)rounded_val;
    result.decimal_part = (uint32_t)roundf((rounded_val - (float)result.integer_part) * scale);

    return result;
}




