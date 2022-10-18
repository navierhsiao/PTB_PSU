/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "../../../BSP/system.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
w25q256_objectTypeDef flash_object;
/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (BYTE pdrv)
{
  /* USER CODE BEGIN INIT */


  W25Q256_Init(&flash_object);
  flash_object.w25q256_resetDevice(&flash_object);
  
  Stat = RES_OK;
  return Stat;
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (BYTE pdrv)
{
  /* USER CODE BEGIN STATUS */
  Stat = RES_OK;
  return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (BYTE pdrv,BYTE *buff,DWORD sector,UINT count)
{
  /* USER CODE BEGIN READ */
  Stat = RES_OK;

  flash_object.w25q256_readBuffer(&flash_object,buff,sector<<12,count<<12);
  return Stat;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (BYTE pdrv,const BYTE *buff,DWORD sector,UINT count)
{
  /* USER CODE BEGIN WRITE */
  /* USER CODE HERE */
  Stat = RES_OK;
  uint32_t addr=0;

  for(int i=0;i<count;i++)
  {
    addr = (sector+i) << 12;

    flash_object.w25q256_eraseSector(&flash_object,addr);

    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*0, addr + 256*0, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*1, addr + 256*1, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*2, addr + 256*2, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*3, addr + 256*3, 256);	

    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*4, addr + 256*4, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*5, addr + 256*5, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*6, addr + 256*6, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*7, addr + 256*7, 256);	

    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*8,  addr + 256*8, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*9,  addr + 256*9, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*10, addr + 256*10, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*11, addr + 256*11, 256);	

    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*12, addr + 256*12, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*13, addr + 256*13, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096))+ 256*14, addr + 256*14, 256);
    flash_object.w25q256_writeBuffer(&flash_object,(buff+(i*4096)) + 256*15, addr + 256*15, 256);	
  }

  return Stat;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (BYTE pdrv,BYTE cmd,void *buff)
{
  /* USER CODE BEGIN IOCTL */
  DRESULT res = RES_ERROR;
  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    *(DWORD*)buff = 1024*8;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    *(WORD*)buff = 4096;
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    *(WORD*)buff = 1;
      res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }
  return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
