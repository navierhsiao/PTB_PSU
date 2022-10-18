#ifndef __W25Q256_H
#define __W25Q256_H

#define QSPI_FLASH_SIZE     25                      /* Flash大小，2^25 = 32MB*/
#define QSPI_SECTOR_SIZE    (4 * 1024)              /* 扇?大小，4KB */
#define QSPI_PAGE_SIZE      256        				/* 页大小，256字? */
#define QSPI_END_ADDR    	(1 << QSPI_FLASH_SIZE)  /* 末尾地址 */
#define QSPI_FLASH_SIZES    32*1024*1024            /* Flash大小，2^25 = 32MB*/

#define WRITE_ENABLE_CMD      0x06         /* ?使能指令 */  
#define READ_ID_CMD2          0x9F         /* ?取ID命令 */  
#define READ_STATUS_REG_CMD   0x05         /* ?取??命令 */ 
#define BULK_ERASE_CMD        0xC7 
#define SUBSECTOR_ERASE_4_BYTE_ADDR_CMD      0x21    /* 32bit地址扇?擦除指令, 4KB */
#define QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD    0x34    /* 32bit地址的4?快速?入命令 */
#define QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xEC    /* 32bit地址的4?快速?取命令 */

#define RESET_ENABLE    0x66
#define RESET_CMD       0x99

typedef struct w25q256_structDef
{
    QSPI_objectTypeDef *qspi_object;
    uint32_t device_ID;
    void (*w25q256_eraseSector) (struct w25q256_structDef *object,uint32_t address);
    void (*w25q256_eraseChip)   (struct w25q256_structDef *object,double *progress);
    void (*w25q256_writeBuffer) (struct w25q256_structDef *object,uint8_t *buf,uint32_t address,uint16_t size);
    void (*w25q256_readBuffer)  (struct w25q256_structDef *object,uint8_t *buf,uint32_t address,uint32_t size);
    void (*w25q256_readID)      (struct w25q256_structDef *object);
    void (*w25q256_resetDevice) (struct w25q256_structDef *object);
}w25q256_objectTypeDef;

void W25Q256_Init(w25q256_objectTypeDef *object);
w25q256_objectTypeDef *get_w25q256_object(void);

#endif