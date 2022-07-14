#ifndef __BSP_INTERNAL_FLASH_H_
#define __BSP_INTERNAL_FLASH_H_

#include "include.h"


#define BASS_ADDRESS 0x08000000
#define PAGE_SIZE 2048

void flash_test(void);
HAL_StatusTypeDef _in_flash_erase(uint32_t star_address,uint32_t num_page);
HAL_StatusTypeDef _in_flash_program(uint32_t TypeProgram, uint32_t Address, uint64_t Data);

#endif