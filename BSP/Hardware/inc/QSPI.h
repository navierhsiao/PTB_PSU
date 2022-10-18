#ifndef __QSPI_H
#define __QSPI_H

typedef struct
{
    QUADSPI_TypeDef     *Instance;
    uint32_t            ClockPrescaler;    
    uint32_t            FifoThreshold;    
    uint32_t            SampleShifting;  
    uint32_t            FlashSize;        
    uint32_t            ChipSelectHighTime;
    uint32_t            ClockMode;         
    uint32_t            DualFlash;          
    uint32_t            FlashID;
}QSPI_objectAttr;

typedef struct QSPI_objectStructDef
{
    QSPI_HandleTypeDef  hqspi;
    QSPI_objectAttr     object_attr;
    __IO uint8_t txCplt;
    __IO uint8_t rxCplt;
    __IO uint8_t cmdCplt;
    __IO uint8_t statusMatch;

    void (*qspi_init)           (struct QSPI_objectStructDef*);
    void (*qspi_writeCmd)       (struct QSPI_objectStructDef*,QSPI_CommandTypeDef *);
    void (*qspi_writeCmd_IT)    (struct QSPI_objectStructDef*,QSPI_CommandTypeDef *);
    void (*qspi_writeData_dma)  (struct QSPI_objectStructDef*,uint8_t *);
    void (*qspi_readData_dma)   (struct QSPI_objectStructDef*,uint8_t *);
    void (*qspi_autoPolling) (struct QSPI_objectStructDef*,QSPI_CommandTypeDef *,QSPI_AutoPollingTypeDef *);
    void (*qspi_autoPolling_IT) (struct QSPI_objectStructDef*,QSPI_CommandTypeDef *,QSPI_AutoPollingTypeDef *);
    void (*qspi_memoryMapped)   (struct QSPI_objectStructDef*,QSPI_CommandTypeDef *,QSPI_MemoryMappedTypeDef *);
}QSPI_objectTypeDef;

#endif