#include "../../system.h"

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

uint8_t __attribute__((section (".usart_buffer")))recieveBuf1[USART1_RX_BUF_SIZE]  ={0};
uint8_t __attribute__((section (".usart_buffer")))tx_buf1[USART1_TX_BUF_SIZE]      ={0};

void uart_init(usart_objectTypeDef *object);
void uart_transmit_DMA(usart_objectTypeDef *uart,uint8_t *data,uint16_t length,uint8_t mode);

rx_handler uart1_rx_handle;

void USART_object_Init(usart_objectTypeDef *object,usart_objectAttr attr,rx_handler handle)
{
  object->attr=attr;
  if(attr.Instance==USART1)
  {
    object->huart=&huart1;
    object->rxBuf=recieveBuf1;
    object->txBuf=tx_buf1;
    object->rxBuf_size=USART1_RX_BUF_SIZE;
    object->txBuf_size=USART1_TX_BUF_SIZE;
    uart1_rx_handle=handle;
  }

  object->uart_init=uart_init;
  object->uart_transmit_DMA=uart_transmit_DMA;

  object->uart_init(object);
}

void uart_init(usart_objectTypeDef *object)
{
  object->huart->Instance = object->attr.Instance;
  object->huart->Init.BaudRate = object->attr.BaudRate;
  object->huart->Init.WordLength = object->attr.WordLength;
  object->huart->Init.StopBits = object->attr.StopBits;
  object->huart->Init.Parity = object->attr.Parity;
  object->huart->Init.Mode = object->attr.Mode;
  object->huart->Init.HwFlowCtl = object->attr.HwFlowCtl;
  object->huart->Init.OverSampling = object->attr.OverSampling;
  object->huart->Init.OneBitSampling = object->attr.OneBitSampling;
  object->huart->Init.ClockPrescaler = object->attr.ClockPrescaler;
  object->huart->AdvancedInit.AdvFeatureInit = object->attr.AdvFeatureInit;
  if (HAL_UART_Init(object->huart) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  if (HAL_UARTEx_SetTxFifoThreshold(object->huart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  if (HAL_UARTEx_SetRxFifoThreshold(object->huart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  if (HAL_UARTEx_DisableFifoMode(object->huart) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }

  __HAL_UART_ENABLE_IT(object->huart, UART_IT_IDLE);
  HAL_UART_Receive_DMA(object->huart, (uint8_t*)object->rxBuf, object->rxBuf_size);
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(huart->Instance==USART1)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler(__FILE__,__LINE__);
    }

    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PB7     ------> USART1_RX
    PA9     ------> USART1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Stream0;
    hdma_usart1_rx.Init.Request = DMA_REQUEST_USART1_RX;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler(__FILE__,__LINE__);
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Stream1;
    hdma_usart1_tx.Init.Request = DMA_REQUEST_USART1_TX;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler(__FILE__,__LINE__);
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart1_tx);

    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
}

void USART1_IRQHandler(void)
{
    if(huart1.Instance==USART1)                                 
    {
      SCB_InvalidateDCache_by_Addr((uint32_t *)recieveBuf1, sizeof(uint32_t)*USART1_RX_BUF_SIZE);

      if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE)!=RESET)   //判斷是否空閒中
      {
          __HAL_UART_CLEAR_IDLEFLAG(&huart1);                     //清除空閒中flag

          HAL_UART_DMAStop(&huart1);                                                     //停止DMA傳輸
          
          uint8_t data_length=USART1_RX_BUF_SIZE-__HAL_DMA_GET_COUNTER(&hdma_usart1_rx); //計算接收到的數據長度
          
          // tx_buf=malloc(data_length*sizeof(uint8_t));

          // memcpy(tx_buf1,recieveBuf1,data_length);
          // HAL_UART_Transmit_DMA(&huart1,tx_buf1,data_length);                     //將接收到的數據print出去
          if(uart1_rx_handle!=NULL)
          {
            uart1_rx_handle(recieveBuf1,data_length);
          }
          
          memset(recieveBuf1,0,data_length);                                            //清空接收緩沖區
          data_length = 0;
          HAL_UART_Receive_DMA(&huart1, (uint8_t*)recieveBuf1, USART1_RX_BUF_SIZE);                    //重啟DMA傳輸
      }
    }
    
    HAL_UART_IRQHandler(&huart1);
}


void uart_transmit_DMA(usart_objectTypeDef *object,uint8_t *data,uint16_t length,uint8_t mode)
{
  //ASCII 13=CR,10=LF
  
  if(length>(object->txBuf_size-2))
  {
    return;
  }
  memset(object->txBuf,0,length);
  memcpy(object->txBuf,data,length);
  if(mode==USART_NO_SUFFIX)
  {
    HAL_UART_Transmit_DMA(object->huart,(uint8_t*)object->txBuf,length);
  }
  else if(mode==USART_SUFFIX_CR)
  {
    object->txBuf[length]=13;
    HAL_UART_Transmit_DMA(object->huart,(uint8_t*)object->txBuf,length+1);
  }
  else if(mode==USART_SUFFIX_LF)
  {
    object->txBuf[length]=10;
    HAL_UART_Transmit_DMA(object->huart,(uint8_t*)object->txBuf,length+1);
  }
  else if(mode==USART_SUFFIX_CR_LF)
  {
    object->txBuf[length]=13;
    object->txBuf[length+1]=10;
    HAL_UART_Transmit_DMA(object->huart,(uint8_t*)object->txBuf,length+2);
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}