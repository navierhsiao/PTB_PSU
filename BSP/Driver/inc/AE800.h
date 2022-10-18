#ifndef __AE800_H
#define __AE800_H

//每10ms計數一次，即2s內沒接收到回應則判定斷線
#define AE800_TIMEOUT_CNT 200
typedef struct ae800_structDef
{
    usart_objectTypeDef ae800_uart;

    __IO uint8_t    link_state;
    //0:初始狀態/:指令正確/2:指令錯誤/3:參數錯誤
    __IO uint8_t    cmd_state;
    //0:正常/1:工作異常/2:通訊異常
    __IO uint8_t    state;
    //比較用數據
    uint16_t        v_set_x100;
    uint16_t        i_set_x100;
    //與AE800用通訊數據
    double          set_voltage;
    double          set_current;
    double          output_voltage;
    double          output_current;
    double          current_temperature;
    __IO uint8_t    on_off_state;
    __IO uint8_t    device_STUS0;
    //0xFF:初始化完成
    __IO uint8_t    init_state;
    uint8_t         last_query_command;
    uint16_t        time_out;
    uint8_t         short_detected;
    uint8_t         is_power_on;
    uint8_t         set_updated;
    uint8_t         diff_value_init_flag;
    uint16_t        current_base_value;
/*
*   Bit-0 -> OVP Shutdown.
*   Bit-1 -> OLP Shutdown.
*   Bit-2 -> OTP Shutdown.
*   Bit-3 -> FAN Failure.
*   Bit-4 -> AUX or SMPS Fail.
*   Bit-5 -> HI-TEMP Alarm.
*   Bit-6 -> AC input power down.
*   Bit-7 -> AC input failure.
*/
    __IO uint8_t FAIL_STATE[8];

    void (*ae800_set_on_off)                (struct ae800_structDef *object,uint8_t cmd);
    void (*ae800_set_ADDS)                  (struct ae800_structDef *object,uint8_t address);
    void (*ae800_set_remote_mode)           (struct ae800_structDef *object,uint8_t mode);
    void (*ae800_set_output_voltage)        (struct ae800_structDef *object,double voltage);
    void (*ae800_set_output_current)        (struct ae800_structDef *object,double current);
    void (*ae800_voltage_setting_query)     (struct ae800_structDef *object);
    void (*ae800_current_setting_query)     (struct ae800_structDef *object);
    void (*ae800_output_voltage_query)      (struct ae800_structDef *object);
    void (*ae800_output_current_query)      (struct ae800_structDef *object);
    void (*ae800_temperature_query)         (struct ae800_structDef *object);
    void (*ae800_device_status_query)       (struct ae800_structDef *object,uint8_t type);
    void (*ae800_device_turned_off)         (struct ae800_structDef *object);
}ae800_objectTypeDef;

enum ae800_query
{
    AE800_SETTING_CMD   =1,
    AE800_PWR_SET       =2,
    AE800_VSET          =3,
    AE800_ISET          =4,
    AE800_VOUT          =5,
    AE800_IOUT          =6,
    AE800_TEMP          =7,
    AE800_STUS0         =8,
    AE800_STUS1         =9
};

ae800_objectTypeDef *ae800_init(uint8_t *sys_para);
#endif