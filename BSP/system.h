#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Hardware/inc/adc.h"
#include "Hardware/inc/gpio.h"
#include "Hardware/inc/i2c.h"
#include "Hardware/inc/QSPI.h"
#include "Hardware/inc/usart.h"

#include "Driver/inc/AE800.h"
#include "Driver/inc/w25q256.h"
#include "Driver/inc/AD5171.h"

//#include "../Libraries/CMSIS/RTOS2/Include/cmsis_os2.h"
#include "../Libraries/FreeRTOS/Source/include/FreeRTOS.h"
// #include "../Libraries/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.h"
#include "../Libraries/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.h"
#include "../Libraries/FatFs/Target/user_diskio.h"
#include "../Libraries/FatFs/Target/user_diskio.h"
#include "../Libraries/FatFs/src/ff.h"
#include "../Libraries/FatFs/src/ff_gen_drv.h"

#include "../Libraries/STM32_USB_Device_Library/Class/CDC/Inc/usbd_cdc.h"
#include "../Libraries/STM32_USB_Device_Library/Class/CDC/Inc/usbd_cdc_if.h"

#include "../Libraries/STM32_USB_Device_Library/Core/Inc/usbd_core.h"
#include "../Libraries/STM32_USB_Device_Library/Core/Inc/usbd_ctlreq.h"
#include "../Libraries/STM32_USB_Device_Library/Core/Inc/usbd_def.h"
#include "../Libraries/STM32_USB_Device_Library/Core/Inc/usbd_ioreq.h"
#include "../Libraries/STM32_USB_Device_Library/Core/Inc/usbd_conf.h"
#include "../Libraries/STM32_USB_Device_Library/Core/Inc/usbd_desc.h"

#include "../Users/app/inc/fatfs.h"
#include "../Users/app/inc/usb_device.h"

#define HIGH_SPEED_TICKER_SET 100000

/*
******************************************************************
    index	    資料                  

    0           機碼(1)-0x55
    1           機碼(2)-0x66	
    2           轉向切換方式-0:3檔式(STOP/CW/CCW) 1:遠端(設定用)
    3           轉向設定(方式須設為遠端才會作用)-0:stop 1:CW 2:CCW
    4           馬達種類-0:BLDC 1:BDC
    5           檢測模式-0:CHA-馬達電流 CHB-馬達電流 1:CHA-馬達電流 CHB-HB 2:CHA-HA CHB-HB
    6	        初始化狀態(0:500mA 1:5A 2:開始自動切換)
                **設定電壓、電流為設定值*100，ex:24.00V->2400
                **                             12.05A->1205
    7	        AE800設定電壓(上八位)
    8	        AE800設定電壓(下八位)
    9	        AE800設定電流(上八位)
    10	        AE800設定電流(下八位)
    11          AE800輸出設定(0:關閉輸出 1:開啟輸出)
    12          GAIN檔調整(0-63)
    13          FAULT flag清除(當初flag為1時，輸出將被禁止)
    14          保留
    ---------------------以下為讀取用---------------------
    15	        AE800輸出電壓*100(上八位)
    16	        AE800輸出電壓*100(下八位)
    17	        AE800輸出電流*1000(上八位)
    18	        AE800輸出電流*1000(下八位)
    19          AE800模組溫度
    20          AE800初始化狀態(0:尚未初始化完成 1:初始化完成)
    21          AE800工作狀態(0:正常 1:工作異常 2:通訊異常)
    22          AE800狀態碼
                **   Bit-0 -> OVP Shutdown.
                **   Bit-1 -> OLP Shutdown.
                **   Bit-2 -> OTP Shutdown.
                **   Bit-3 -> FAN Failure.
                **   Bit-4 -> AUX or SMPS Fail.
                **   Bit-5 -> HI-TEMP Alarm.
                **   Bit-6 -> AC input power down.
                **   Bit-7 -> AC input failure.     
    23	        模組狀態-0:OK 1:driver fault 2:編碼器輸出短路
    24			本地馬達設定輸出方向(由控制線控制)
    25	        目前電流採樣檔(0:500mA/1:5A)
    26          PC狀態(內部使用)-0:關機 1:開機 2:睡眠
                **關機:S0:HIGH  S3:LOW  S4:LOW
                **開機:S0:HIGH  S3:HIGH S4:HIGH
                **睡眠:S0:HIGH  S3:LOW  S4:HIGH
    27          FAULT flag
    28          目前Gain*100(上八位)
    29          目前Gain*100(下八位)
    30          目前Gain檔
    31          version-XXX=X.XX	
    32          date-YY
    33          date-MM
    34          data-DD
    35          結束碼-0xFF
******************************************************************
*/

void system_init(void);
void BSP_Init();
uint32_t get_H_precision_ticker(void);
void Error_Handler(char *file,uint32_t line);

#endif