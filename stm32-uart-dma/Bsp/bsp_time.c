#include "bsp_time.h"

TIM_HandleTypeDef htim6;

volatile uint32_t ulHighFrequencyTimerTicks = 0UL; /* 被系统调用 */

void time_init(void)
{
    __HAL_RCC_TIM6_CLK_ENABLE();
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 36 - 1; // 0.5us
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim6.Init.Period = 100 - 1;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim6);
    HAL_NVIC_SetPriority(TIM6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_IRQn);
    HAL_TIM_Base_Start_IT(&htim6);
}

void TIM6_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM6)
    {
        ++ulHighFrequencyTimerTicks;
    }
}


uint32_t millis(void)
{
    __IO uint32_t mills;
    __set_PRIMASK(1);
    mills = (TIM7->CNT) * 2;
    __set_PRIMASK(0);
    return (mills);
}
