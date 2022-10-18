#include "system.h"

TIM_HandleTypeDef htim6;

uint32_t sys_ticker=0;

static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void MPU_Config(void);

/*
**************************************************************************
*     System_Init(void)
*     系統初始化並取得系統狀態
**************************************************************************
*/

void system_init(void)
{
  //配置MPU
  MPU_Config();
  //啟用L1 cache
  CPU_CACHE_Enable();
  //初始化HAL庫
  HAL_Init();
  //配置系統時鐘
  SystemClock_Config();
}

/*
**************************************************************************
*     BSP_Init(void)
*     硬體驅動初始化
**************************************************************************
*/

void BSP_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();
  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);

  // TIM6_Init(HIGH_SPEED_TICKER_SET);
  ADC_Init();
  GPIO_Init();
  // FATFS_Init();
}

/*
**************************************************************************
*     SystemClock_Config(void)
*     系統時鐘初始化
*     System Clock                      480MHz
*     HCLK                              240MHz (AXI & AHBs clocks)
*     AHB prescaler                     2
*     D1 APB3 prescaler                 2 (AP3 clock 120MHz)
*     D2 APB1 prescaler                 2 (AP1 clock 120MHz)
*     D2 APB2 prescaler                 2 (AP2 clock 120MHz)
*     D3 APB4 prescaler                 2 (AP4 clock 120MHz)
*     HSE                               25MHz
*     PLL_M                             5
*     PLL_N                             192
*     PLL_P                             2
*     PLL_Q                             20
*     PLL_R                             2
*     VDD(V)                            3.3
**************************************************************************
*/

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  __HAL_RCC_D2SRAM3_CLK_ENABLE();

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
}

static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disables the MPU */
  HAL_MPU_Disable();
  /**Initializes and configures the Region and the memory to be protected 
  */

  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x30047C00;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  // MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  // MPU_InitStruct.BaseAddress = 0x24000000;
  // MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  // MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  // MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  // MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  // MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  // MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  // MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  // MPU_InitStruct.SubRegionDisable = 0x00;
  // MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

static void CPU_CACHE_Enable(void)
{
	/* enalbe I-Cache */
	SCB_EnableICache();

	/* enable D-Cache */
	SCB_EnableDCache();
}

/*
**************************************************************************
*     void TIM6_Init(uint32_t precision)
*     使用TIM6作為高速計時器
*     tick:觸發頻率，代表該計時器的精度
**************************************************************************
*/
void TIM6_Init(uint32_t precision)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  uint16_t period;
	uint16_t prescaler;

  prescaler=240-1;
  period=1000000/precision-1;

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = prescaler;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = period;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }

  HAL_TIM_Base_Start_IT(&htim6);

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM6)
  {
    __HAL_RCC_TIM6_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  }

}
/*
**************************************************************************
*     void TIM6_DAC_IRQHandler(void)
*     會依照前面所設定的觸發頻率，每秒觸發tick次本函式
**************************************************************************
*/
void TIM6_DAC_IRQHandler(void)
{
  static uint32_t count=0;

  HAL_TIM_IRQHandler(&htim6);
  sys_ticker++;
  count++;

  if(count>=100)
  {
    //每us執行一次
    count=0;
  }
}
/*
**************************************************************************
*     void get_H_precision_ticker(void)
*     取得高精度計時器的數值
**************************************************************************
*/
uint32_t get_H_precision_ticker(void)
{
  return sys_ticker;
}


void Error_Handler(char *file,uint32_t line)
{
  if(line==0)
  {
    return;
  }

  while (1)
  {

  }
}

/*
**************************************************************************
*     void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
*     系統使用TIM2作為HAL時鐘源，每1ms會觸發一次本函式
**************************************************************************
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
}
