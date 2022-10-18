#ifndef AD5171_H
#define AD5171_H

#include "../../Hardware/inc/i2c.h"
#include "stm32h7xx.h"

#define     AD0         0
#define     RW          60

#if (AD0==0)
    #define AD5171_ADDR 0x58
#else
    #define AD5171_ADDR 0x5A
#endif

enum AD5171_RES
{
    AD5171_5K,
    AD5171_10K,
    AD5171_50K,
    AD5171_100K
};

typedef struct AD5171_structDef
{   
    i2c_objectTypeDef   i2c_object;
    uint32_t            resistance;
    uint32_t            current_resistance;
    uint8_t             current_step;
    uint8_t             state;

    //AD5171 可將阻值分為64個step，依照想設定的阻值尋找最接近的step
    void (*ad5171_set_target_resistance)    (struct i2c_objectStructDef* object,uint32_t target_res);
    //直接設定step，數值範圍為0-63
    void (*ad5171_set_step)                 (struct i2c_objectStructDef* object,uint8_t step);
}AD5171_objectTypeDef;  

void AD5171_init(AD5171_objectTypeDef *object,uint32_t resistance);

#endif