#ifndef __USART_H
#define __USART_H

#define USART1_RX_BUF_SIZE   30
#define USART1_TX_BUF_SIZE   30

#define USART2_RX_BUF_SIZE   255
#define USART2_TX_BUF_SIZE   257

enum USART_SUFFIX_MODE
{
    USART_NO_SUFFIX=0,
    USART_SUFFIX_CR=1,
    USART_SUFFIX_LF=2,
    USART_SUFFIX_CR_LF=3
};

typedef struct 
{
    USART_TypeDef       *Instance;
    uint32_t            BaudRate;
    uint32_t            WordLength;
    uint32_t            StopBits; 
    uint32_t            Parity; 
    uint32_t            Mode;
    uint32_t            HwFlowCtl;
    uint32_t            OverSampling;
    uint32_t            OneBitSampling;
    uint32_t            ClockPrescaler;
    uint32_t            AdvFeatureInit;
}usart_objectAttr;

typedef void (*rx_handler)(uint8_t* data,int length);

typedef struct usart_structDef
{
    UART_HandleTypeDef *huart;
    usart_objectAttr attr;

    uint8_t *rxBuf;
    uint8_t *txBuf;
    uint16_t rxBuf_size;
    uint16_t txBuf_size;

    void (*uart_init)           (struct usart_structDef *object);
    void (*uart_transmit_DMA)   (struct usart_structDef *object,uint8_t *data,uint16_t length,uint8_t mode)
}usart_objectTypeDef;

void USART_object_Init(usart_objectTypeDef *object,usart_objectAttr attr,rx_handler handle);


void USART1_Init(void);
void usart1_transmit_DMA(uint8_t *data,uint16_t length,uint8_t mode);

#endif