#include "bsp_debug_usart.h"

static void uart_idle_IRQHandler(void);
static void usart_process_data(uint8_t *data, size_t len);

UART_HandleTypeDef UartHandle;
DMA_HandleTypeDef  DMA_Handle;      //DMA句柄


uint8_t usart_rx_dma_buffer[64];

/**
 * @brief  DEBUG_USART GPIO 配置,工作模式配置。115200 8-N-1
 * @param  无
 * @retval 无
 */
void DEBUG_USART_Config(void)
{

    UartHandle.Instance          = USART1;

    UartHandle.Init.BaudRate     = 115200;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;

    HAL_UART_Init(&UartHandle);
    __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_IDLE); /*使能串口空闲中断 */

    __HAL_RCC_DMA1_CLK_ENABLE();
    DMA_Handle.Instance = DMA1_Channel5;                        //数据流选择
    DMA_Handle.Init.Direction = DMA_PERIPH_TO_MEMORY;           //存储器到外设HAL_DMA_Init(&DMA_Handle);
    DMA_Handle.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式/* Associate the DMA handle */
    DMA_Handle.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式__HAL_LINKDMA(&UartHandle, hdmatx, DMA_Handle);
    DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
    DMA_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
    DMA_Handle.Init.Mode = DMA_CIRCULAR;                        //外设普通模式
    DMA_Handle.Init.Priority = DMA_PRIORITY_MEDIUM;             //中等优先级

    HAL_DMA_Init(&DMA_Handle);
    /* Associate the DMA handle */
    __HAL_LINKDMA(&UartHandle, hdmarx, DMA_Handle);
    HAL_UART_Receive_DMA(&UartHandle, usart_rx_dma_buffer, ARRAY_LEN(usart_rx_dma_buffer)); // 开启DMA传输
}


/**
  * @brief UART MSP 初始化
  * @param huart: UART handle
  * @retval 无
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    DEBUG_USART_CLK_ENABLE();

    DEBUG_USART_RX_GPIO_CLK_ENABLE();
    DEBUG_USART_TX_GPIO_CLK_ENABLE();

    /* 配置Tx引脚为复用功能  */
    GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed =  GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);

    /* 配置Rx引脚为复用功能 */
    GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;	//模式要设置为复用输入模式！
    HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);	//抢占优先级0，子优先级1
    HAL_NVIC_EnableIRQ(USART1_IRQn );		    //使能USART1中断通道
}


/**
 * \brief           Check for new data received with DMA
 *
 * User must select context to call this function from:
 * - Only interrupts (DMA HT, DMA TC, UART IDLE) with same preemption priority level
 * - Only thread context (outside interrupts)
 *
 * If called from both context-es, exclusive access protection must be implemented
 * This mode is not advised as it usually means architecture design problems
 *
 * When IDLE interrupt is not present, application must rely only on thread context,
 * by manually calling function as quickly as possible, to make sure
 * data are read from raw buffer and processed.
 *
 * 如果读取速度不够快，可能会导致 DMA 未读取的字节溢出
 * 因此应用程序将丢失有用的数据
 *
 * Solutions to this are:
 * - Improve architecture design to achieve faster reads
 * - Increase raw buffer size and allow DMA to write more data before this function is called
 */
void usart_rx_check(void)
{
    static size_t old_pos; // 读指针
    size_t pos; // 写指针

    /* Calculate current position in buffer and check for new data available */
    pos = ARRAY_LEN(usart_rx_dma_buffer) - __HAL_DMA_GET_COUNTER(&DMA_Handle);

    if (pos != old_pos)                         /* Check change in received data */
    {
        if (pos > old_pos)                      /* Current position is over previous one */
        {
            /*
             * Processing is done in "linear" mode.
             *
             * Application processing is fast with single data block,
             * length is simply calculated by subtracting pointers
             *
             * [   0   ]
             * [   1   ] <- old_pos |------------------------------------|
             * [   2   ]            |                                    |
             * [   3   ]            | Single block (len = pos - old_pos) |
             * [   4   ]            |                                    |
             * [   5   ]            |------------------------------------|
             * [   6   ] <- pos
             * [   7   ]
             * [ N - 1 ]
             */
            usart_process_data(&usart_rx_dma_buffer[old_pos], pos - old_pos);
        }
        else
        {
            /*
             * Processing is done in "overflow" mode..
             *
             * Application must process data twice,
             * since there are 2 linear memory blocks to handle
             *
             * [   0   ]            |---------------------------------|
             * [   1   ]            | Second block (len = pos)        |
             * [   2   ]            |---------------------------------|
             * [   3   ] <- pos
             * [   4   ] <- old_pos |---------------------------------|
             * [   5   ]            |                                 |
             * [   6   ]            | First block (len = N - old_pos) |
             * [   7   ]            |                                 |
             * [ N - 1 ]            |---------------------------------|
             */
            usart_process_data(&usart_rx_dma_buffer[old_pos], ARRAY_LEN(usart_rx_dma_buffer) - old_pos);

            if (pos > 0)
            {
                usart_process_data(&usart_rx_dma_buffer[0], pos);
            }
        }

        old_pos = pos;                          /* Save current position as old for next transfers */
    }
}


/**
 * \brief           Process received data over UART
 * \note            Either process them directly or copy to other bigger buffer
 * \param[in]       data: Data to process
 * \param[in]       len: Length in units of bytes
 */
static void usart_process_data(uint8_t *data, size_t len)
{
    /*
     * This function is called on DMA TC or HT events, and on UART IDLE (if enabled) event.
     *
     * For the sake of this example, function does a loop-back data over UART in polling mode.
     * Check ringbuff RX-based example for implementation with TX & RX DMA transfer.
     */
    HAL_UART_Transmit(&UartHandle, (uint8_t *)data, len, 1000);
}

/**
 * \brief           Send string to USART
 * \param[in]       str: String to send
 */
void usart_send_string(char * str)
{
    usart_process_data((uint8_t *)str, strlen(str));
}

void USART1_IRQHandler(void)
{
    if(UartHandle.Instance == USART1)
    {
        uart_idle_IRQHandler();
    }
}

static void uart_idle_IRQHandler(void)
{
    if(__HAL_UART_GET_IT_SOURCE(&UartHandle, UART_IT_IDLE) != RESET)
    {
        if(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_IDLE) != RESET)
        {
            __HAL_UART_CLEAR_IDLEFLAG(&UartHandle);

            HAL_UART_DMAPause(&UartHandle);
            printf("\r\nuart_idle\r\n");
            usart_rx_check();
            // 重启开始DMA传输
            HAL_UART_DMAResume(&UartHandle);
        }
    }
}

//重定向c库函数printf到串口DEBUG_USART，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到串口DEBUG_USART */
    HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 1000);

    return (ch);
}