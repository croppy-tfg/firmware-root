/**
 * @file    power.c
 * @author  Jose
 * @date    23/9/26
 * @brief   Energy saving handling
 * @project tfg
 */

#include "power.h"

#include "stm32f4xx_hal.h"

extern RTC_HandleTypeDef hrtc;
extern void SystemClock_Config(void);

void Power_Enter_Stop_Mode(uint32_t seconds)
{
    // deactivate the RTC
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
    // clear the Wake-Up Timer Event Flag
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
    // charge the timer with 'seconds' seconds @1Hz
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, seconds - 1, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);

    // disable TIM1 update interrupt
    HAL_SuspendTick();

    // enter STOP mode with low power regulator
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    // -----------------------------------
    // here the node should be sleeping...
    // -----------------------------------

    // restore main oscilator on wake up
    SystemClock_Config();

    // resume TIM1 update interrupt
    HAL_ResumeTick();

    // deactivate the timer to avoid random interrupts
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
}

