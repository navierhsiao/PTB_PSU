#ifndef I2C_H
#define I2C_H

typedef struct
{
    I2C_TypeDef     *Instance;
    //init structure
    uint32_t        Timing;    
    uint32_t        OwnAddress1;   
    uint32_t        AddressingMode;   
    uint32_t        DualAddressMode;   
    uint32_t        OwnAddress2;      
    uint32_t        OwnAddress2Masks;  
    uint32_t        GeneralCallMode;   
    uint32_t        NoStretchMode;   
}i2c_objectAttr;

typedef struct  i2c_objectStructDef
{
    I2C_HandleTypeDef hi2c;
    i2c_objectAttr object_attr;

    void (*i2c_init)        (struct i2c_objectStructDef*);
    void (*i2c_write)       (struct i2c_objectStructDef* object,uint16_t deviceAddr,uint8_t *data,uint16_t length);
    void (*i2c_read)        (struct i2c_objectStructDef* object,uint16_t deviceAddr,uint8_t *data,uint16_t length);
}i2c_objectTypeDef;

void I2C_Object_Init(i2c_objectTypeDef *object,i2c_objectAttr attr);

#endif