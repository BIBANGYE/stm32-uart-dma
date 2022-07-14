#ifndef __BSP_IWDG_H_
#define __BSP_IWDG_H_

#include "include.h"

void iwdg_config(uint32_t Prescaler,uint32_t Reload);
void iwdg_feed(void);

#endif