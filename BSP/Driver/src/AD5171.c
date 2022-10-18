#include "../../system.h"

const i2c_objectAttr i2c_object_attr={
    .Instance           =I2C1,
    .Timing             =0x10C0ECFF,
    .OwnAddress1        =0,   
    .AddressingMode     =I2C_ADDRESSINGMODE_7BIT,   
    .DualAddressMode    =I2C_DUALADDRESS_DISABLE,   
    .OwnAddress2        =0,      
    .OwnAddress2Masks   =I2C_OA2_NOMASK,  
    .GeneralCallMode    =I2C_GENERALCALL_DISABLE,   
    .NoStretchMode      =I2C_NOSTRETCH_DISABLE   
};

void AD5171_set_target_resistance(AD5171_objectTypeDef *object,uint32_t target_resistance);
void AD5171_set_step(AD5171_objectTypeDef *object,uint8_t step);
void AD5171_get_state(AD5171_objectTypeDef *object);

void AD5171_init(AD5171_objectTypeDef *object,uint32_t resistance)
{
    I2C_Object_Init(&object->i2c_object,i2c_object_attr);

    object->ad5171_set_target_resistance=AD5171_set_target_resistance;
    object->ad5171_set_step=AD5171_set_step;

    if(resistance==AD5171_5K)
    {
        object->resistance=5000;
    }
    else if(resistance==AD5171_10K)
    {
        object->resistance=10000;   
    }
    else if(resistance==AD5171_50K)
    {
        object->resistance=50000;
    }
    else if(resistance==AD5171_100K)
    {
        object->resistance=100000;
    }

    AD5171_get_state(object);
}

//step 0阻值最大->63阻值最小
void AD5171_set_target_resistance(AD5171_objectTypeDef *object,uint32_t target_resistance)
{
    if(object->resistance!=0)
    {
        if(target_resistance<object->resistance)
        {
            uint8_t data[2]={0,0};
            uint8_t set_step=0;

            set_step=63-((63*target_resistance-63*RW)/object->resistance);
            data[1]=set_step;
            object->i2c_object.i2c_write(&object->i2c_object,AD5171_ADDR,&data,2);

            object->state=1;
            while(object->state!=0x00)
            {
                AD5171_get_state(object);
            }
        }
    }
}

void AD5171_set_step(AD5171_objectTypeDef *object,uint8_t step)
{
    if(step<64)
    {
        uint8_t data[2]={0,0};
        data[1]=step;
        object->i2c_object.i2c_write(&object->i2c_object,AD5171_ADDR,&data,2);
        object->state=1;
        while(object->state!=0x00)
        {
            AD5171_get_state(object);
        }
    }
}

void AD5171_get_state(AD5171_objectTypeDef *object)
{
    uint8_t data=0;
    uint8_t read_step=0;
    double  para=0.0;
    object->i2c_object.i2c_read(&object->i2c_object,AD5171_ADDR,&data,1);

    /*
        讀回data的前2位為狀態碼，後5位為目前設定的step
        E1 E0 D5 D4 D3 D2 D1 D0
        狀態碼：E1  E0   說明
                0   0   Ready
                0   1   For factory use
                1   0   Error
                1   1   Success
    */

    read_step=data&0x3F;
    object->current_step=read_step;
    para=(63.0-(double)object->current_step)/63.0;
    object->current_resistance=para*(double)object->resistance+RW;
    object->state=data>>6;
}