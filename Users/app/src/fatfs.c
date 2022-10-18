
#include "fatfs.h"
#include "../../BSP/system.h"
#include <string.h>

#define BUF_SIZE				(4*1024)

/*
**************************************************************************************************************
*	儲存的項目分為3個檔案
*	(1)目前工作參數->為當次耐久機種的運作資料，命名為該機種名稱，換機種時舊有的資料就會被清除
*	(2)模組的相關資訊->模組系統相關，需儲存的資訊
*	(3)工作記錄資料->工作記錄參數，每1min會寫入一次
**************************************************************************************************************
*/

char		rootPath[4];  
FATFS		fs;    
FIL			file;      
uint8_t		work[4096];

//文件內所使用的功能
FRESULT get_files_in_Dir(root_attribute *attr,const char *dir_path);
FRESULT create_new_file(const char *current_dir,const char *fileName);
FRESULT create_new_dir(const char *current_dir,const char *dirName);
FRESULT delete_file(const char *current_dir,const char *fileName);
FRESULT delete_dir(const char *current_dir,const char *dirName);
FRESULT read_file_data(char *buf,const char *current_dir,const char *fileName,uint32_t buf_size);
FRESULT write_data_to_file(char *buf,const char *current_dir,const char *fileName,uint32_t file_len);

//提供給外部的function
void FATFS_Init(void)
{
	FRESULT 		result=FR_OK;
	uint8_t			is_system_file_exist=0;
	root_attribute 	root_attr={0};

	result = FATFS_LinkDriver(&USER_Driver, rootPath);

	get_files_in_Dir(&root_attr,rootPath);

}

void fileSystem_read_data(const char *fileName,uint8_t *buf,uint32_t buf_size)
{
	read_file_data(buf,rootPath,fileName,buf_size);
}

void fileSystem_write_data(const char *fileName,uint8_t *buf,uint32_t buf_size)
{
	write_data_to_file(buf,rootPath,fileName,buf_size);
}

void fileSystem_get_diretory(root_attribute *attr)
{	
	get_files_in_Dir(attr,rootPath);
}

void fileSystem_format()
{
	FRESULT result;

	result = f_mount(&fs, rootPath, 0);	
	if (result != FR_OK)
	{
		return;
	}	

	result = f_mkfs("", FM_ANY, 0, work, sizeof(work));
	if (result != FR_OK)
	{
		return;
	}

	result  = f_mount(NULL, rootPath, 0);
	if (result != FR_OK)
	{
		return;
	}
}

void fileSystem_deleteFile(const char *fileName)
{
	delete_file(rootPath,fileName);
}
//內部操作function
FRESULT get_files_in_Dir(root_attribute *attr,const char *dir_path)
{
	FRESULT 	result=FR_OK;
	FILINFO 	info={0};
	DIR 		dir={0};
	uint8_t 	type=0;

	result = f_mount(&fs, rootPath, 0);	/* Mount a logical drive */
	if (result != FR_OK)
	{
		return result;
	}

	result = f_opendir(&dir, dir_path); /* 如果不?参?，?从?前目??始 */
	if (result != FR_OK)
	{
		return result;
	}

	while(1)
	{
		result = f_readdir(&dir, &info);

		if (result != FR_OK || info.fname[0] == 0)
		{
			break;
		}

		if (info.fattrib & AM_DIR)
		{
			type=1;
		}
		else
		{
			type=2;
		}

		f_stat(info.fname, &info);

		if(type==1)
		{
			//目錄
			sprintf(attr->dir_names[attr->dir_cnt],"%s",(char*)info.fname);
			attr->dir_cnt++;
		}
		else if(type==2)
		{
			//文件
			sprintf(attr->file_names[attr->file_cnt],"%s",(char*)info.fname);
			attr->file_cnt++;
		}
	}

	f_mount(NULL, rootPath, 0);

	return result;
}

FRESULT create_new_file(const char *current_dir,const char *fileName)
{
	FRESULT 	result;
	uint32_t 	bw;
	uint8_t 	err;
	char 		file_path[32];

	result = f_mount(&fs, rootPath, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		return result;
	}

	sprintf(file_path,"%s%s",current_dir,fileName);
	result = f_open(&file, file_path, FA_CREATE_ALWAYS | FA_WRITE);
	if (result != FR_OK)
	{
		return result;
	}

	f_close(&file);
	f_mount(NULL, rootPath, 0);

	return result;
}

FRESULT create_new_dir(const char *current_dir,const char *dirName)
{
	FRESULT 	result;
	uint8_t 	err;
	char 		dir_path[32];

	result = f_mount(&fs, rootPath, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		return result;
	}

	sprintf(dir_path,"%s%s",current_dir,dirName);
	result = f_mkdir(dir_path);
	if (result != FR_OK)
	{
		return result;
	}

	f_mount(NULL, rootPath, 0);
	return result;
}

FRESULT delete_file(const char *current_dir,const char *fileName)
{
	FRESULT 	result;
	uint8_t 	err;
	char 		file_path[32];

	result = f_mount(&fs, rootPath, 0);	
	sprintf(file_path,"%s%s",current_dir,fileName);
	result = f_unlink(file_path);
	if (result != FR_OK)
	{
		return result;
	}

	f_mount(NULL, rootPath, 0);
	return result;
}

FRESULT delete_dir(const char *current_dir,const char *dirName)
{
	FRESULT 	result;
	uint8_t 	err;
	char 		dir_path[32];

	result = f_mount(&fs, rootPath, 0);	
	sprintf(dir_path,"%s%s",current_dir,dirName);
	result = f_unlink(dir_path);

	if (result != FR_OK)
	{
		return result;
	}

	return result;
}

FRESULT read_file_data(char *buf,const char *current_dir,const char *fileName,uint32_t buf_size)
{
	FRESULT 	result;
	char 		file_path[32];
	uint32_t 	br;
	uint8_t 	err;

	result = f_mount(&fs, rootPath, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		return result;
	}

	sprintf(file_path,"%s%s",current_dir,fileName);
	result = f_open(&file, file_path, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		return result;
	}

	result = f_read(&file, buf, buf_size, &br);

	f_close(&file);
	f_mount(NULL, rootPath, 0);
	return result;
}

FRESULT write_data_to_file(char *buf,const char *current_dir,const char *fileName,uint32_t file_len)
{
	FRESULT 	result;
	char 		file_path[32];
	uint32_t 	bw;
	uint8_t 	err;

	result = f_mount(&fs, rootPath, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		return result;
	}

	sprintf(file_path,"%s%s",current_dir,fileName);
	result = f_open(&file, file_path, FA_CREATE_ALWAYS | FA_WRITE );

	result=f_lseek(&file,f_size(&file));

	result = f_write(&file, buf, strlen(buf), &bw);
	if(result!=FR_OK)
	{
		return result;
	}

	f_close(&file);	
	f_mount(NULL, rootPath, 0);
	return result;
}

DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}
