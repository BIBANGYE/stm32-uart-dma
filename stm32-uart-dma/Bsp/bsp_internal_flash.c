#include "bsp_internal_flash.h"

HAL_StatusTypeDef _in_flash_erase(uint32_t star_address,uint32_t num_page)
{    
    if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != SET)
    {
        uint32_t SECTORError = 0;
        static FLASH_EraseInitTypeDef EraseInitStruct;
        
        HAL_FLASH_Unlock();/* 解锁 */
        
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;//页擦除
        EraseInitStruct.PageAddress = star_address;
        EraseInitStruct.NbPages = num_page;
        
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
        
        HAL_FLASH_Lock();
        return HAL_OK;
    }
    
    return HAL_ERROR; 
}


HAL_StatusTypeDef _in_flash_program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
{
    if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != SET)
    {
        HAL_FLASH_Unlock();/* 解锁 */
        
        if(HAL_FLASH_Program(TypeProgram,Address,Data) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
        
        HAL_FLASH_Lock();
        return HAL_OK;
    }
    
    return HAL_ERROR;
}


void flash_test(void)
{
    uint32_t buf=0x3233eeaf;
    
    uint32_t star = BASS_ADDRESS+(10*PAGE_SIZE);
    uint32_t end  = BASS_ADDRESS+(40*PAGE_SIZE);
    
    _in_flash_erase(BASS_ADDRESS+(10*PAGE_SIZE),30);
    
    while(star<end)
    {
        if(_in_flash_program(FLASH_TYPEPROGRAM_WORD,star,buf) == HAL_OK)
        {
            star+=4;
        }
        else
        {
            printf("\r\n写入错误\r\n");
            break;
        }
    }
    
    while(star<end)
    {
        if(  *((__IO uint32_t *)star) == buf)
        {
            star+=4;
        }
        else
        {
            printf("\r\n对比错误\r\n");
            break;
        }
    }
    
    printf("\r\n写入对比成功\r\n");
}



