#ifndef __BSP_H_
#define __BSP_H_

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "bsp_debug_usart.h"
#include "bsp_led.h"
#include "bsp_button.h"
#include "bsp_time.h"
#include "bsp_internal_flash.h"
#include "bsp_iwdg.h"

void bsp_init(void);
void SystemClock_Config(void);

#endif

