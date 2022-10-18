#include "../../system.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
uint16_t convCplt_times=0;
uint8_t init_flag=0;
/*
**************************************************************************
*       DATA[0]-[1]為
*       [0]=AE800 output Voltage
*       [1]=AE800 output current
**************************************************************************
*/
uint16_t __attribute__((section (".ADC_DMA_Buffer")))ADC_Buffer_value[2];
uint32_t ADC_data_accumulation[2];
uint16_t ADC_data_smoothed[2][SAMPLE_TIMES];
uint16_t ADC_data_smoothed_afterAVG[2];

/*
**************************************************************************
*       ADC 腳位定義
*       PA6(ADC1_INP3)  ------> AE800 voltage sensing
*       PB1(ADC1_INP5)  ------> AE800 current sensing
**************************************************************************
*/

/*
**************************************************************************
*     void ADC_Init(void)
*     初始化ADC及其DMA配置
**************************************************************************
*/
void ADC_Init(void)
{
  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_Buffer_value, 2) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if(hadc->Instance==ADC1)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.PLL2.PLL2M = 4;
    PeriphClkInitStruct.PLL2.PLL2N = 9;
    PeriphClkInitStruct.PLL2.PLL2P = 2;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    /* Peripheral clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA6     ------> ADC1_INP3
    PB1     ------> ADC1_INP5
    */

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hdma_adc1.Instance = DMA1_Stream2;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);

  }

}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  // /* Invalidate Data Cache to get the updated content of the SRAM on the first half of the ADC converted data buffer: 32 bytes */ 
  SCB_InvalidateDCache_by_Addr((uint32_t *)ADC_Buffer_value, sizeof(ADC_Buffer_value));
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  //  /* Invalidate Data Cache to get the updated content of the SRAM on the second half of the ADC converted data buffer: 32 bytes */ 
  SCB_InvalidateDCache_by_Addr((uint32_t *)ADC_Buffer_value, sizeof(ADC_Buffer_value));

  ADC_data_accumulation[0]+=ADC_Buffer_value[0];
  ADC_data_accumulation[1]+=ADC_Buffer_value[1];

  convCplt_times++;
  if(convCplt_times>=SAMPLE_TIMES)
  {
    uint32_t smoothed_data_sum[2]={0};
    uint32_t averaged_data[2]={0};

    averaged_data[0]=ADC_data_accumulation[0]/SAMPLE_TIMES;
    averaged_data[1]=ADC_data_accumulation[1]/SAMPLE_TIMES;
    
    //第一次處理時將獲得的平均值初始化到陣列中
    if(init_flag==0)
    {
      for(int i=0;i<SAMPLE_TIMES;i++)
      {
        ADC_data_smoothed[0][i]=averaged_data[0];
        ADC_data_smoothed[1][i]=averaged_data[1];
      }
    }

    for(int i=0;i<SAMPLE_TIMES-1;i++)
    {
      ADC_data_smoothed[0][i]=ADC_data_smoothed[0][i+1];
      ADC_data_smoothed[1][i]=ADC_data_smoothed[1][i+1];
    }

    ADC_data_smoothed[0][SAMPLE_TIMES-1]=averaged_data[0];
    ADC_data_smoothed[1][SAMPLE_TIMES-1]=averaged_data[1];

    for(int i=0;i<SAMPLE_TIMES;i++)
    {
      smoothed_data_sum[0]+=ADC_data_smoothed[0][i];
      smoothed_data_sum[1]+=ADC_data_smoothed[1][i];
    }

    ADC_data_smoothed_afterAVG[0]=smoothed_data_sum[0]/SAMPLE_TIMES;
    ADC_data_smoothed_afterAVG[1]=smoothed_data_sum[1]/SAMPLE_TIMES;

    memset(smoothed_data_sum,0,2);
    memset(averaged_data,0,2);
    
    ADC_data_accumulation[0]=0;
    ADC_data_accumulation[1]=0;

    init_flag=1;
    convCplt_times=0;
  }
}

/*
**************************************************************************
*     uint16_t get_ADC_buffer_value(int index)
*     輸入index回傳該index的ADC值
**************************************************************************
*/
uint16_t get_ADC_buffer_value(int index)
{
  SCB_InvalidateDCache_by_Addr((uint32_t *)ADC_Buffer_value, sizeof(ADC_Buffer_value));
  if(index<2)
  {
    return ADC_Buffer_value[index];
  }
  else
  {
    return 0xFFFF;
  }
}

/*
**************************************************************************
*     uint16_t get_ADC_smoothed_data(uint8_t index)
*     取得平滑處理後的資料
**************************************************************************
*/
uint16_t get_ADC_smoothed_data(uint8_t index)
{
  return ADC_data_smoothed_afterAVG[index];
}