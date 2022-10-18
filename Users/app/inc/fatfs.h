/**
  ******************************************************************************
  * @file   fatfs.h
  * @brief  Header for fatfs applications
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __fatfs_H
#define __fatfs_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "ff.h"
#include "ff_gen_drv.h"
#include "user_diskio.h" /* defines USER_Driver as external */

typedef struct
{
  char      file_names[20][20];
  uint8_t   file_cnt;
  char      dir_names[20][20];
  uint8_t   dir_cnt;
  int       file_size[20];
  int       dir_size[20];
}root_attribute;

extern uint8_t retUSER; /* Return value for USER */
extern char USERPath[4]; /* USER logical drive path */
extern FATFS USERFatFS; /* File system object for USER logical drive */
extern FIL USERFile; /* File object for USER */

void FATFS_Init(void);

void fileSystem_read_data(const char *fileName,uint8_t *buf,uint32_t buf_size);
void fileSystem_write_data(const char *fileName,uint8_t *buf,uint32_t buf_size);
void fileSystem_get_diretory(root_attribute *attr);
void fileSystem_format();
void fileSystem_deleteFile(const char *fileName);
#ifdef __cplusplus
}
#endif
#endif /*__fatfs_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
