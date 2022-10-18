2022.05.17

1.USB驅動注意事項：
    (1)usbconf.c->HAL_PCD_MspInit中加入
        ．PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
        ．PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    (2)system clock配置要注意下方幾點
        ．RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
        ．RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;

2022.05.18

1.UART注意事項，將緩存區放在D2 RAM中，要加入下列程式
    (1)在接收中斷處理中加入：SCB_InvalidateDCache_by_Addr((uint32_t *)recieveBuf1, sizeof(uint32_t)*USART1_RX_BUF_SIZE);
    (2)MPU_config加入如下程式：
        static void MPU_Config(void)
        {
            MPU_Region_InitTypeDef MPU_InitStruct;

            /* Disables the MPU */
            HAL_MPU_Disable();
                /**Initializes and configures the Region and the memory to be protected 
                */
            MPU_InitStruct.Enable = MPU_REGION_ENABLE;
            MPU_InitStruct.Number = MPU_REGION_NUMBER1;
            MPU_InitStruct.BaseAddress = 0x30047C00;
            MPU_InitStruct.Size = MPU_REGION_SIZE_1KB;
            MPU_InitStruct.SubRegionDisable = 0x0;
            MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
            MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
            MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
            MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
            MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
            MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

            HAL_MPU_ConfigRegion(&MPU_InitStruct);

            /* Enables the MPU */
            HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
        }

2022.05.24

1.馬達驅動板預計修改：
    (1)加入另一組放大電路，一組倍率大(0-100mA)，一組倍率小(0-2A)
    (x)(2)將brake腳廢除，此腳功能將用於選擇電流採樣源，並依AE800的輸出電流自動選擇採樣源(標準為50mA)

2022.06.14

1.更動：
    (1)加入I2C驅動、數位電阻相關程式
    (2)完成數位電阻功能後增加增益調整功能
    (3)切換500mA、5A量測的相關引腳功能
        a.依AE800的輸出電流，自動切換引腳(以500mA為分界)
    (4)通訊格式調整，加入下列資料
        a.訊號增益
        b.目前所使用的量測基準(0:500mA 1:5A)

2022.06.20-21
    (1)加入短路偵測功能
    (2)增加寄存器及加入保留位
    (3)基本功能皆測試完成
    (4)剩開機校正功能待完成(ADC部分)

2022.06.22
    (1)待修改項目：
        a.短路偵測只在AE800輸出時進行(done)
        b.AE800繼電器輸出關閉加入延時(ex:偵測到電腦關閉5秒後才關閉輸出)(done)
        c.AE800輸出關閉後確認重置所有相關參數(done)

2022.07.29
    (1)改版電路板加入下列IO
        a.PA8 5v編碼器電源偵測(input)
        b.PD7 CSD/PHD切換腳2(output)
        c.PC8 按鈕LED紅燈 12v控制腳(output)
        d.PD0 風扇開啟-偵測到開機時開啟風扇(output)