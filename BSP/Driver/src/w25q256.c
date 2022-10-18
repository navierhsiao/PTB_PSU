#include "../../system.h"

QSPI_objectAttr qspi_attr={
    .Instance               = QUADSPI,
    .ClockPrescaler         = 1,
    .FifoThreshold          = 32,
    .SampleShifting         = QSPI_SAMPLE_SHIFTING_HALFCYCLE,
    .FlashSize              = QSPI_FLASH_SIZE,
    .ChipSelectHighTime     = QSPI_CS_HIGH_TIME_1_CYCLE, /* Min 50ns for nonRead */
    .ClockMode              = QSPI_CLOCK_MODE_0,
    .DualFlash              = QSPI_DUALFLASH_DISABLE,
    .FlashID                = QSPI_FLASH_ID_1
};

void W25Q256_eraseSector(w25q256_objectTypeDef *object,uint32_t address);
void W25Q256_eraseChip(w25q256_objectTypeDef *object,double *progress);
void W25Q256_writeBuffer(w25q256_objectTypeDef *object,uint8_t *buf,uint32_t address,uint16_t size);
void W25Q256_readBuffer(w25q256_objectTypeDef *object,uint8_t *buf,uint32_t address,uint32_t size);
void W25Q256_readID(w25q256_objectTypeDef *object);
void W25Q256_resetDevice(w25q256_objectTypeDef *object);

void W25Q256_writeEnable(w25q256_objectTypeDef *object);
void W25Q256_autoPollingMemReady(w25q256_objectTypeDef *object);

void W25Q256_Init(w25q256_objectTypeDef *object)
{
    object->qspi_object=QSPI_object_Init(qspi_attr);
    object->w25q256_eraseSector=W25Q256_eraseSector;
	object->w25q256_eraseChip=W25Q256_eraseChip;
    object->w25q256_writeBuffer=W25Q256_writeBuffer;
    object->w25q256_readBuffer=W25Q256_readBuffer;
    object->w25q256_readID=W25Q256_readID;
	object->w25q256_resetDevice=W25Q256_resetDevice;

	object->w25q256_readID(object);
}

void W25Q256_eraseSector(w25q256_objectTypeDef *object,uint32_t address)
{
    QSPI_CommandTypeDef command={0};

    object->qspi_object->cmdCplt=0;

    W25Q256_writeEnable(object);

    /* 基本配置 */
	command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1?方式发送指令 */
	command.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32位地址 */
	command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字? */
	command.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，?据输出延? */
	command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* 每次?输都发指令 */	
	
	/* 擦除配置 */
	command.Instruction = SUBSECTOR_ERASE_4_BYTE_ADDR_CMD;   /* 32bit地址方式的扇?擦除命令，扇?大小4KB*/       
	command.AddressMode = QSPI_ADDRESS_1_LINE;  /* 地址发送是1?方式 */       
	command.Address     = address;              /* 扇?首地址，保?是4KB整?倍 */    
	command.DataMode    = QSPI_DATA_NONE;       /* 无需发送?据 */  
	command.DummyCycles = 0;                    /* 无需空周期 */  


	object->qspi_object->qspi_writeCmd_IT(object->qspi_object,&command);
	
	/* 等待命令发送完? */
	while(object->qspi_object->cmdCplt == 0);
	object->qspi_object->cmdCplt = 0;
	
	/* 等待?程?束 */
	object->qspi_object->statusMatch = 0;

    W25Q256_autoPollingMemReady(object);

	while(object->qspi_object->statusMatch == 0);
	object->qspi_object->statusMatch = 0;
}

void W25Q256_eraseChip(w25q256_objectTypeDef *object,double *progress)
{
	uint32_t addr=0;
	for(int i=0;i<16*512;i++)
	{
		W25Q256_eraseSector(object,addr);
		addr+=4096;
	}
}

void W25Q256_writeBuffer(w25q256_objectTypeDef *object,uint8_t *buf,uint32_t address,uint16_t size)
{
	QSPI_CommandTypeDef command={0};
	
    object->qspi_object->txCplt=0;

    W25Q256_writeEnable(object);
	
	command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    
	command.AddressSize       = QSPI_ADDRESS_32_BITS;     
	command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 
	command.DdrMode           = QSPI_DDR_MODE_DISABLE;   
	command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 
	command.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;	
	
	command.Instruction = QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD; 
	command.DummyCycles = 0;                  
	command.AddressMode = QSPI_ADDRESS_1_LINE; 
	command.DataMode    = QSPI_DATA_4_LINES;  
	command.NbData      = size;        
	command.Address     = address;       

    object->qspi_object->qspi_writeCmd(object->qspi_object,&command);
	
	/* 启动MDMA?输 */
    object->qspi_object->qspi_writeData_dma(object->qspi_object,buf);
	
	/* 等待?据发送完? */
	while(object->qspi_object->txCplt == 0);
	object->qspi_object->txCplt = 0;
	
	/* 等待Flash页?程完? */
	object->qspi_object->statusMatch = 0;

	W25Q256_autoPollingMemReady(object);

	while(object->qspi_object->statusMatch == 0);
	object->qspi_object->statusMatch = 0;	
}

void W25Q256_readBuffer(w25q256_objectTypeDef *object,uint8_t *buf,uint32_t address,uint32_t size)
{
	QSPI_CommandTypeDef command = {0};
	
	/* 用于等待接收完成?志 */
	object->qspi_object->rxCplt = 0;
	
	/* 基本配置 */
	command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    	/* 1?方式发送指令 */
	command.AddressSize       = QSPI_ADDRESS_32_BITS;      	/* 32位地址 */
	command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  	/* 无交替字? */
	command.DdrMode           = QSPI_DDR_MODE_DISABLE;      	/* W25Q256JV不支持DDR */
	command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  	/* DDR模式，?据输出延? */
	command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;		/* 每次?输要发指令 */	
 
	/* ?取?据 */
	command.Instruction = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD; /* 32bit地址的4?快速?取命令 */
	command.DummyCycles = 6;                    /* 空周期 */
	command.AddressMode = QSPI_ADDRESS_4_LINES; /* 4?地址 */
	command.DataMode    = QSPI_DATA_4_LINES;    /* 4??据 */ 
	command.NbData      = size;              /* ?取的?据大小 */ 
	command.Address     = address;          /* ?取?据的起始地址 */ 
	
    object->qspi_object->qspi_writeCmd(object->qspi_object,&command);

	/* MDMA方式?取 */
    object->qspi_object->qspi_readData_dma(object->qspi_object,buf);
	
	/* 等接受完? */
	while(object->qspi_object->rxCplt == 0);
	object->qspi_object->rxCplt = 0;
}

void W25Q256_readID(w25q256_objectTypeDef *object)
{
	QSPI_CommandTypeDef command = {0};
	uint8_t buf[3];

	/* 基本配置 */
	command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1?方式发送指令 */
	command.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32位地址 */
	command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字? */
	command.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，?据输出延? */
	command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	  /* 每次?输都发指令 */
	
	/* ?取JEDEC ID */
	command.Instruction = READ_ID_CMD2;         /* ?取ID命令 */
	command.AddressMode = QSPI_ADDRESS_NONE;    /* 1?地址 */
	command.DataMode = QSPI_DATA_1_LINE;        /* 1?地址 */
	command.DummyCycles = 0;                    /* 无空周期 */
	command.NbData = 3;                         /* ?取三个?据 */

    object->qspi_object->qspi_writeCmd_IT(object->qspi_object,&command);

    HAL_QSPI_Receive(&object->qspi_object->hqspi,buf,HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

	object->device_ID = (buf[0] << 16) | (buf[1] << 8 ) | buf[2];
}

void W25Q256_writeEnable(w25q256_objectTypeDef *object)
{
    QSPI_CommandTypeDef command = {0};

    /* 基本配置 */
    command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1?方式发送指令 */
    command.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32位地址 */
    command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字? */
    command.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
    command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，?据输出延? */
    command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* 每次?输都发指令 */

    /* ?使能 */
    command.Instruction       = WRITE_ENABLE_CMD;  /* ?使能指令 */
    command.AddressMode       = QSPI_ADDRESS_NONE; /* 无需地址 */
    command.DataMode          = QSPI_DATA_NONE;    /* 无需?据 */
    command.DummyCycles       = 0;                 /* 空周期  */

    object->qspi_object->qspi_writeCmd_IT(object->qspi_object,&command);
}

void W25Q256_resetDevice(w25q256_objectTypeDef *object)
{
    QSPI_CommandTypeDef command = {0};

    command.InstructionMode   = QSPI_INSTRUCTION_1_LINE; 
    command.AddressSize       = QSPI_ADDRESS_32_BITS;      
    command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 
    command.DdrMode           = QSPI_DDR_MODE_DISABLE;      
    command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	

    command.Instruction       = RESET_ENABLE;  
    command.AddressMode       = QSPI_ADDRESS_NONE;
    command.DataMode          = QSPI_DATA_NONE;    
    command.DummyCycles       = 0;              

    object->qspi_object->qspi_writeCmd(object->qspi_object,&command);

    command.Instruction       = RESET_CMD;  
    command.AddressMode       = QSPI_ADDRESS_NONE;
    command.DataMode          = QSPI_DATA_NONE;    
    command.DummyCycles       = 0;              

    object->qspi_object->qspi_writeCmd(object->qspi_object,&command);
}

void W25Q256_autoPollingMemReady(w25q256_objectTypeDef *object)
{
	QSPI_CommandTypeDef     command = {0};
	QSPI_AutoPollingTypeDef config = {0};

	/* 基本配置 */
	command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1?方式发送指令 */
	command.AddressSize       = QSPI_ADDRESS_32_BITS;       /* 32位地址 */
	command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字? */
	command.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q256JV不支持DDR */
	command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，?据输出延? */
	command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 /* 每次?输都发指令 */
	
	/* ?取??*/
	command.Instruction       = READ_STATUS_REG_CMD; /* ?取??命令 */
	command.AddressMode       = QSPI_ADDRESS_NONE;   /* 无需地址 */
	command.DataMode          = QSPI_DATA_1_LINE;    /* 1??据 */
	command.DummyCycles       = 0;                   /* 无需空周期 */

	/* 屏蔽位设置的bit0，匹配位等待bit0?0，即不?查询??寄存器bit0，等待其?0 */
	config.Mask            = 0x01;
	config.Match           = 0x00;
	config.MatchMode       = QSPI_MATCH_MODE_AND;
	config.StatusBytesSize = 1;
	config.Interval        = 0x10;
	config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    object->qspi_object->qspi_autoPolling_IT(object->qspi_object,&command,&config);
}