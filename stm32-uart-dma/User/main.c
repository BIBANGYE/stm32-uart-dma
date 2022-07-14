#include "include.h"

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
    bsp_init();

    usart_send_string("USART DMA example: DMA HT & TC + USART IDLE LINE interrupts\r\n");
    usart_send_string("Start sending data to STM32\r\n");

    while(1)
    {
        led_on();
        HAL_Delay(400);// ms
        led_off();
        HAL_Delay(400);
    }
}







