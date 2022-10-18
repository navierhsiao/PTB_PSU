#include "../../system.h"
#include <string.h>

//電壓ADC採樣參數
#define VOLTAGE_CHANNEL_CALIBRATION         -990
#define VOLTAGE_COMP_TIMES                  1.12676
#define VOLTAGE_CHANNEL_FACTOR              0.000981

//電流ADC採樣參數
#define CURRENT_CHANNEL_CALIBRATION         -964
#define CURRENT_CHANNEL_FACTOR              0.0002309
//由查表法算出之因數，後方為加權值
#define CURRENT_CHANNEL_FACTOR_NEW          0.00021951*1.92

ae800_objectTypeDef ae800_object={
    .link_state=0,
    .cmd_state=0,
    .state=0,
    .v_set_x100=0,
    .i_set_x100=0,
    .set_voltage=0.0,
    .set_current=0.0,
    .output_voltage=0.0,
    .output_current=0.0,
    .current_temperature=0.0,
    .on_off_state=0,
    .device_STUS0=0,
    .init_state=0,
    .last_query_command=0,
    .time_out=0,
    .short_detected=0,
    .is_power_on=0
};

usart_objectAttr ae800_uart_object_attr={
  .Instance         = USART1,
  .BaudRate         = 4800,
  .WordLength       = UART_WORDLENGTH_8B,
  .StopBits         = UART_STOPBITS_1,
  .Parity           = UART_PARITY_NONE,
  .Mode             = UART_MODE_TX_RX,
  .HwFlowCtl        = UART_HWCONTROL_NONE,
  .OverSampling     = UART_OVERSAMPLING_16,
  .OneBitSampling   = UART_ONE_BIT_SAMPLE_DISABLE,
  .ClockPrescaler   = UART_PRESCALER_DIV1,
  .AdvFeatureInit   = UART_ADVFEATURE_NO_INIT
};

const osThreadAttr_t ThreadAE800_Attr = 
{
	.name = "AE800Thread",
	.attr_bits = osThreadDetached, 
	.priority = osPriorityNormal4,
	.stack_size = 1024,
};

void AppTaskAE800(void *argument);
osThreadId_t ThreadIdTaskAE800 = NULL;

void ae800_recieve_data_handle(uint8_t *rxBuf,int length);
ae800_objectTypeDef *ae800_init(uint8_t *sys_para);

void ae800_set_on_off(ae800_objectTypeDef *object,uint8_t cmd);
void ae800_set_ADDS(ae800_objectTypeDef *object,uint8_t address);
void ae800_set_remote_mode(ae800_objectTypeDef *object,uint8_t mode);
void ae800_set_output_voltage(ae800_objectTypeDef *object,double voltage);
void ae800_set_output_current(ae800_objectTypeDef *object,double current);
void ae800_voltage_setting_query(ae800_objectTypeDef *object);
void ae800_current_setting_query(ae800_objectTypeDef *object);
void ae800_output_voltage_query(ae800_objectTypeDef *object);
void ae800_output_current_query(ae800_objectTypeDef *object);
void ae800_temperature_query(ae800_objectTypeDef *object);
void ae800_device_status_query(ae800_objectTypeDef *object,uint8_t type);
void ae800_device_turned_off(ae800_objectTypeDef *object);

ae800_objectTypeDef *ae800_init(uint8_t *sys_para)
{
    USART_object_Init(&ae800_object.ae800_uart,ae800_uart_object_attr,ae800_recieve_data_handle);

    ae800_object.ae800_set_on_off               =ae800_set_on_off;
    ae800_object.ae800_set_ADDS                 =ae800_set_ADDS;
    ae800_object.ae800_set_remote_mode          =ae800_set_remote_mode;
    ae800_object.ae800_set_output_voltage       =ae800_set_output_voltage;
    ae800_object.ae800_set_output_current       =ae800_set_output_current;
    ae800_object.ae800_voltage_setting_query    =ae800_voltage_setting_query;
    ae800_object.ae800_current_setting_query    =ae800_current_setting_query;
    ae800_object.ae800_output_voltage_query     =ae800_output_voltage_query;
    ae800_object.ae800_output_current_query     =ae800_output_current_query;
    ae800_object.ae800_temperature_query        =ae800_temperature_query;
    ae800_object.ae800_device_status_query      =ae800_device_status_query;
    ae800_object.ae800_device_turned_off        =ae800_device_turned_off;
    ThreadIdTaskAE800=osThreadNew(AppTaskAE800,sys_para,&ThreadAE800_Attr);

    return &ae800_object;
}

void AppTaskAE800(void *argument)
{
    uint16_t    poll_state=0;
    uint16_t    temp_Vset=0;
    uint16_t    temp_Iset=0;
    uint8_t     temp_on_off=0;
    uint16_t    exec_count=0;
    uint8_t     diff_value_init_count=0;
    uint16_t    diff_add_value=0;
    //系統參數定義詳見system.h
    uint8_t *system_para_temp=(uint8_t*)argument;
    while(1)
    {
        //每100ms通訊一次
        if(exec_count>10)
        {
            if(ae800_object.is_power_on==1)
            {
                if(ae800_object.link_state==0)
                {
                    //未連結
                    switch (ae800_object.init_state)
                    {
                    case 0:
                        system_para_temp[20]=0;
                        ae800_object.ae800_set_ADDS(&ae800_object,1);
                        if(ae800_object.cmd_state==1)
                        {
                            ae800_object.init_state=1;
                            ae800_object.cmd_state=0;
                        }
                        break;
                    case 1:
                        ae800_object.ae800_set_remote_mode(&ae800_object,1);
                        if(ae800_object.cmd_state==1)
                        {
                            ae800_object.init_state=2;
                            ae800_object.cmd_state=0;
                        }
                        break;
                    case 2:
                        ae800_object.ae800_voltage_setting_query(&ae800_object);
                        if(ae800_object.cmd_state==1)
                        {
                            ae800_object.init_state=3;
                            ae800_object.cmd_state=0;
                        }
                        break;
                    case 3:
                        ae800_object.ae800_current_setting_query(&ae800_object);
                        if(ae800_object.cmd_state==1)
                        {
                            ae800_object.init_state=4;
                            ae800_object.cmd_state=0;
                        }
                        break;
                    case 4:
                        ae800_object.ae800_temperature_query(&ae800_object);
                        if(ae800_object.cmd_state==1)
                        {
                            ae800_object.init_state=5;
                            ae800_object.cmd_state=0;
                        }
                        break;
                    case 5:
                        ae800_object.ae800_temperature_query(&ae800_object);
                        if(ae800_object.cmd_state==1)
                        {
                            ae800_object.init_state=6;
                            ae800_object.cmd_state=0;
                        }
                        break;
                    case 6:
                        ae800_object.ae800_device_status_query(&ae800_object,0);
                        if(ae800_object.cmd_state==1)
                        {
                            ae800_object.init_state=0xFF;
                            ae800_object.cmd_state=0;
                            ae800_object.link_state=1;
                            system_para_temp[20]=1;
                        }
                        break;
                    }
                }
                else if(ae800_object.link_state==1)
                {
                    //已連接到AE800，並初始化完成
                    //初始化設定完成
                    switch (poll_state)
                    {
                    case 0:
                    //詢問狀態
                        ae800_object.ae800_device_status_query(&ae800_object,0);
                        if(ae800_object.cmd_state==1)
                        {
                            ae800_object.cmd_state=0;
                            poll_state=1;
                        }
                        break;
                    case 1:
                    //比較電壓設定值是否有變化，若有則設定新電壓值
                        if(temp_Vset!=ae800_object.v_set_x100)
                        {
                            ae800_object.set_updated=1;
                            ae800_object.set_voltage=((double)ae800_object.v_set_x100/100);
                            ae800_object.ae800_set_output_voltage(&ae800_object,ae800_object.set_voltage);
                            if(ae800_object.cmd_state==1)
                            {
                                temp_Vset=ae800_object.v_set_x100;
                                ae800_object.cmd_state=0;
                                poll_state=2;
                            }    
                        }
                        else
                        {
                            poll_state=2;
                        }
                        break;
                    case 2:
                    //比較電流設定值是否有變化，若有則設定新電流值
                        if(temp_Iset!=ae800_object.i_set_x100)
                        {
                            ae800_object.set_current=((double)ae800_object.i_set_x100/100);
                            ae800_object.ae800_set_output_current(&ae800_object,ae800_object.set_current);
                            if(ae800_object.cmd_state==1)
                            {
                                temp_Iset=ae800_object.i_set_x100;
                                ae800_object.cmd_state=0;
                                poll_state=3;
                            }    
                        }
                        else
                        {
                            poll_state=3;
                        }
                        break;
                    case 3:
                    //比較輸出設定是否有變化，若有則變更輸出狀態
                    //未初始化完成前直接跳過這一步
                        if(ae800_object.diff_value_init_flag==1)
                        {
                            if(ae800_object.on_off_state!=temp_on_off)
                            {
                                ae800_object.ae800_set_on_off(&ae800_object,ae800_object.on_off_state);
                                temp_on_off=ae800_object.on_off_state;

                                //電源關閉後則清除輸出短路flag
                                if(temp_on_off==0)
                                {
                                    ae800_object.short_detected=0;
                                }

                                if(ae800_object.cmd_state==1)
                                {
                                    ae800_object.cmd_state=0;
                                    poll_state=0;
                                }    
                            }
                            else
                            {
                                poll_state=0;
                            }
                        }
                        else
                        {
                            // temp_on_off=1;
                            // ae800_object.ae800_set_on_off(&ae800_object,1);
                            poll_state=0;
                        }
                        break;
                    }

                    if(ae800_object.last_query_command!=0)
                    {
                        ae800_object.time_out++;
                        if(ae800_object.time_out>AE800_TIMEOUT_CNT)
                        {
                            system_para_temp[20]=0;
                            ae800_object.link_state=0;
                            ae800_object.init_state=0;
                            ae800_object.time_out=0;
                        }
                    }
                    else
                    {
                        ae800_object.time_out=0;
                    }

                    if(ae800_object.diff_value_init_flag==0)
                    {
                        diff_value_init_count++;
                        diff_add_value+=get_ADC_smoothed_data(1);
                        if(diff_value_init_count>10)
                        {
                            ae800_object.current_base_value=diff_add_value/11;
                            ae800_object.diff_value_init_flag=1;
                        }
                    }
                }
            }

            exec_count=0;
        }

        //電壓設定(*100)
        ae800_object.v_set_x100=(uint16_t)(system_para_temp[7]<<8)+system_para_temp[8];
        //電流設定(*100)
        ae800_object.i_set_x100=(uint16_t)(system_para_temp[9]<<8)+system_para_temp[10];

        
        //輸出設定
        if(ae800_object.short_detected==0)
        {
            ae800_object.on_off_state=system_para_temp[11];
        }
        else 
        {
            ae800_object.on_off_state=0;
        }
        
        //由ADC取得輸出電壓、電流值
        //初始化完成後才開始量測電壓電流
        if(ae800_object.diff_value_init_flag==1)
        {
            ae800_object.output_voltage=((double)get_ADC_smoothed_data(0)+VOLTAGE_CHANNEL_CALIBRATION)*VOLTAGE_CHANNEL_FACTOR*VOLTAGE_COMP_TIMES;
            //因電路轉換計算不準確，改由查表法計算
            // ae800_object.output_current=((double)get_ADC_smoothed_data(1)+CURRENT_CHANNEL_CALIBRATION)*CURRENT_CHANNEL_FACTOR;
            ae800_object.output_current=(((double)get_ADC_smoothed_data(1)-ae800_object.current_base_value)*CURRENT_CHANNEL_FACTOR_NEW);
        }
        
        //將計算好的電壓、電流值放入參數陣列中
        //電壓(*100)上八位
        system_para_temp[15]=(uint8_t)((uint16_t)(ae800_object.output_voltage*100)>>8);
        //電壓(*100)下八位
        system_para_temp[16]=(uint8_t)((uint16_t)(ae800_object.output_voltage*100));
        //電流(*1000)上八位
        system_para_temp[17]=(uint8_t)(((uint16_t)(ae800_object.output_current*1000))>>8);
        //電流(*1000)下八位
        system_para_temp[18]=(uint8_t)(((uint16_t)(ae800_object.output_current*1000)));
        //模組溫度
        system_para_temp[19]=(uint8_t)(ae800_object.current_temperature);
        //指令狀態
        if(ae800_object.cmd_state>1)
        {
            system_para_temp[21]=ae800_object.cmd_state;
        }
        else
        {
            system_para_temp[21]=0;
        }
        //狀態碼
        system_para_temp[22]=ae800_object.device_STUS0;
        exec_count++;
        osDelay(10);
    }
}

/*
**************************************************************************
*       void ae800_recieve_data_handle(void)
*       處理從AE800接收到的回傳資料
**************************************************************************
*/
void ae800_recieve_data_handle(uint8_t *rxBuf,int length)
{
    const char CMD_Ok[]="=>\r\n";
    const char CMD_ERROR[]="?>\r\n";
    const char PARA_ERROR[]="!>\r\n";
    uint8_t ae800_echo_buff[30];

    if(length==0||length>30)
    {
        return;
    }

    memcpy(ae800_echo_buff,rxBuf,length);
   
    if(strncmp(ae800_echo_buff,CMD_Ok,4)==0)
    {
        //指令正確
        ae800_object.cmd_state=1;
    }
    else if(strncmp(ae800_echo_buff,CMD_ERROR,4)==0)
    {
        //指令錯誤
        ae800_object.cmd_state=2;
        ae800_object.last_query_command=0;
        ae800_object.state=2;
    }
    else if(strncmp(ae800_echo_buff,PARA_ERROR,4)==0)
    {
        //參數錯誤
        ae800_object.cmd_state=3;
        ae800_object.last_query_command=0;
        ae800_object.state=2;
    }    

    if(ae800_object.last_query_command==AE800_SETTING_CMD)
    { 
        ae800_object.last_query_command=0;
    }
    else if(ae800_object.last_query_command==AE800_PWR_SET)
    {
        //未使用
        ae800_object.cmd_state=1;
        ae800_object.last_query_command=0;
    }
    else if(ae800_object.last_query_command==AE800_VSET)
    {
        uint8_t len=strcspn(ae800_echo_buff,"V\r\n");
        char temp[5];
        strncpy(temp,ae800_echo_buff,len);
        ae800_object.set_voltage=atof(temp);
        ae800_object.v_set_x100=100*ae800_object.set_voltage;
        ae800_object.cmd_state=1;
        ae800_object.last_query_command=0;
    }
    else if(ae800_object.last_query_command==AE800_ISET)
    {
        uint8_t len=strcspn(ae800_echo_buff,"A\r\n");
        char temp[5];
        strncpy(temp,ae800_echo_buff,len);
        ae800_object.set_current=atof(temp);
        ae800_object.i_set_x100=100*ae800_object.set_current;
        ae800_object.cmd_state=1;
        ae800_object.last_query_command=0;
    }
    else if(ae800_object.last_query_command==AE800_VOUT)
    {
        uint8_t len=strcspn(ae800_echo_buff,"V\r\n");
        char temp[5];
        strncpy(temp,ae800_echo_buff,len);
        ae800_object.output_voltage=atof(temp);
        ae800_object.cmd_state=1;
        ae800_object.last_query_command=0;
    }
    else if(ae800_object.last_query_command==AE800_IOUT)
    {
        uint8_t len=strcspn(ae800_echo_buff,"V\r\n");
        char temp[5];
        strncpy(temp,ae800_echo_buff,len);
        ae800_object.output_current=atof(temp);
        ae800_object.cmd_state=1;
        ae800_object.last_query_command=0;
    }
    else if(ae800_object.last_query_command==AE800_TEMP)
    {
        uint8_t len=strcspn(ae800_echo_buff,"°C\r\n");
        char temp[5];
        strncpy(temp,ae800_echo_buff,len);
        ae800_object.current_temperature=atof(temp);
        ae800_object.cmd_state=1;
        ae800_object.last_query_command=0;
    }
    else if(ae800_object.last_query_command==AE800_STUS0)
    {
        char temp[2];
        strncpy(temp,ae800_echo_buff,2);
        ae800_object.device_STUS0=atoi(temp);
        
        if(ae800_object.device_STUS0==0)
        {
            ae800_object.cmd_state=1;
            ae800_object.last_query_command=0;
        }
        else
        {
            for(int i=0;i<8;i++)
            {
                if(ae800_object.device_STUS0&(1<<i))
                {
                    ae800_object.FAIL_STATE[i]=1;
                }
                else
                {
                    ae800_object.FAIL_STATE[i]=0;
                }
            }
            ae800_object.cmd_state=1;
            ae800_object.last_query_command=0;
        }

        //若FAIL_STATE的bit4、bit6被設為1，則判定power off
        if(ae800_object.FAIL_STATE[4]==1&&ae800_object.FAIL_STATE[6]==1)
        {
            ae800_object.link_state=0;
            ae800_object.init_state=0;
            ae800_object.cmd_state=0;
            for(int i=0;i<8;i++)
            {
                ae800_object.FAIL_STATE[i]=0;
            }
        }
    }
}

/*
**************************************************************************
*       void ae800_set_ADDS(uint8_t address)
*       設定AE800傳輸地址，主機地址依照機體上旋鈕的設定
*       address:    0-7
**************************************************************************
*/
void ae800_set_ADDS(ae800_objectTypeDef *object,uint8_t address)
{
    uint8_t str[10];
    const char cmd[]="ADDS ";
    sprintf(str,"%s%d\r\n",cmd,address);
    
    size_t length=strlen(str);

    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,str,length,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_SETTING_CMD;
}

/*
**************************************************************************
*       void ae800_set_on_off(uint8_t cmd)
*       設定電源on/off，只要傳輸該指令電源就會被切換到remote模式
*       cmd:    0(off)/1(on)
**************************************************************************
*/
void ae800_set_on_off(ae800_objectTypeDef *object,uint8_t state)
{
    uint8_t str[10];
    const char cmd[]="POWER ";
    sprintf(str,"%s%d\r\n",cmd,state);

    size_t length=strlen(str);

    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,str,length,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_PWR_SET;
}

/*
**************************************************************************
*       void ae800_set_remote_mode(uint8_t mode)
*       設定電源控制為主機控制或是本機控制
*       cmd:    0(local))/1(remote)
**************************************************************************
*/
void ae800_set_remote_mode(ae800_objectTypeDef *object,uint8_t mode)
{
    uint8_t str[10];
    const char cmd[]="REMS ";
    sprintf(str,"%s%d\r\n",cmd,mode);

    size_t length=strlen(str);

    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,str,length,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_SETTING_CMD;
}

/*
**************************************************************************
*       void ae800_set_output_voltage(double voltage)
*       設定輸出電壓
*       voltage:    0.00-60.00V
**************************************************************************
*/
void ae800_set_output_voltage(ae800_objectTypeDef *object,double voltage)
{
    uint8_t str[10];
    const char cmd[]="SV ";
    sprintf(str,"%s%.2f\r\n",cmd,voltage);

    size_t length=strlen(str);

    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,str,length,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_SETTING_CMD;
}

/*
**************************************************************************
*       void ae800_set_output_current(double current)
*       設定最大輸出電流
*       voltage:    0.00-13.40A
**************************************************************************
*/
void ae800_set_output_current(ae800_objectTypeDef *object,double current)
{
    uint8_t str[10];
    const char cmd[]="SI ";
    sprintf(str,"%s%.2f\r\n",cmd,current);

    size_t length=strlen(str);

    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,str,length,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_SETTING_CMD;
}

/*
**************************************************************************
*       void ae800_voltage_setting_query()
*       詢問設定電壓
**************************************************************************
*/
void ae800_voltage_setting_query(ae800_objectTypeDef *object)
{
    const char tx_str[5]="SV?\r\n";
    
    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,tx_str,5,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_VSET;
}

/*
**************************************************************************
*       void ae800_current_setting_query()
*       詢問設定電流
**************************************************************************
*/
void ae800_current_setting_query(ae800_objectTypeDef *object)
{
    const char tx_str[5]="SI?\r\n";
    
    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,tx_str,5,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_ISET;
}

/*
**************************************************************************
*       void ae800_output_voltage_query()
*       詢問輸出電壓
**************************************************************************
*/
void ae800_output_voltage_query(ae800_objectTypeDef *object)
{
    const char tx_str[5]="RV?\r\n";
    
    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,tx_str,5,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_VOUT;
}

/*
**************************************************************************
*       void ae800_output_current_query()
*       詢問輸出電流
**************************************************************************
*/
void ae800_output_current_query(ae800_objectTypeDef *object)
{
    const char tx_str[5]="RI?\r\n";
    
    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,tx_str,5,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_IOUT;
}

/*
**************************************************************************
*       void ae800_temperature_query()
*       詢問內部溫度
**************************************************************************
*/
void ae800_temperature_query(ae800_objectTypeDef *object)
{
    const char tx_str[5]="RT?\r\n";
    
    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,tx_str,5,USART_NO_SUFFIX);
    ae800_object.last_query_command=AE800_TEMP;
}

/*
**************************************************************************
*       void ae800_device_status_query(uint8_t type)
*       詢問裝置的運作狀態，1未使用到
*        <type> = 0 : 
*       Bit-0 -> OVP Shutdown.
*       Bit-1 -> OLP Shutdown.
*       Bit-2 -> OTP Shutdown.
*       Bit-3 -> FAN Failure.
*       Bit-4 -> AUX or SMPS Fail.
*       Bit-5 -> HI-TEMP Alarm.
*       Bit-6 -> AC input power down.
*       Bit-7 -> AC input failure.
*       <type> = 1 :
*       Bit-0 -> Inhibit by VCI / ACI or ENB.
*       Bit-1 -> Inhibit by Software Command.
*       Bit-2 -> (Not used.)
*       Bit-3 -> (Not used.)
*       Bit-4 -> (POWER)
*       Bit-5 -> (Not used.)
*       Bit-6 -> (Not used.)
*       Bit-7 -> (REMOTE)
**************************************************************************
*/
void ae800_device_status_query(ae800_objectTypeDef *object,uint8_t type)
{
    uint8_t str[10];
    const char cmd[]="STUS ";
    sprintf(str,"%s%d\r\n",cmd,type);

    size_t length=strlen(str);

    object->ae800_uart.uart_transmit_DMA(&object->ae800_uart,str,length,USART_NO_SUFFIX);
    if(type==0)
    {
        ae800_object.last_query_command=AE800_STUS0;
    }
    else if(type==1)
    {
        ae800_object.last_query_command=AE800_STUS1;
    }
}

//AE800電源關閉後重置所有參數
void ae800_device_turned_off(ae800_objectTypeDef *object)
{
    object->link_state=0;
    object->cmd_state=0;
    object->state=0;
    object->v_set_x100=0;
    object->i_set_x100=0;
    object->set_voltage=0.0;
    object->set_current=0.0;
    object->output_voltage=0.0;
    object->output_current=0.0;
    object->current_temperature=0.0;
    object->on_off_state=0;
    object->device_STUS0=0;
    object->init_state=0;
    object->last_query_command=0;
    object->time_out=0;
    object->short_detected=0;
}