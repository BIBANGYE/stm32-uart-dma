#include "bsp_iwdg.h"

// 看门狗喂狗时间 time(ms) = (Prescaler/40Khz ) * Reload

IWDG_HandleTypeDef iwdg;

void iwdg_config(uint32_t Prescaler,uint32_t Reload)
{
    
    iwdg.Instance = IWDG;
    iwdg.Init.Prescaler = Prescaler;
    iwdg.Init.Reload = Reload;
 
    HAL_IWDG_Init(&iwdg);
}

void iwdg_feed(void)
{
    HAL_IWDG_Refresh(&iwdg);
}