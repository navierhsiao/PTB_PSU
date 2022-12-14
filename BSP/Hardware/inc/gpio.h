#ifndef __GPIO_H
#define __GPIO_H

#define LED(n)        (n?HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET))
#define LED_TOGGLE    (HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_0))

//0:CSD 1:PHD
#define CSD_PHD_1(n)      (n?HAL_GPIO_WritePin(GPIOD,GPIO_PIN_10,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOD,GPIO_PIN_10,GPIO_PIN_RESET))
#define CSD_PHD_2(n)     (n?HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,GPIO_PIN_RESET))

//馬達控制相關
//SLEEP:0(睡眠狀態)/1(啟用狀態)
#define M_SLEEP(n)      (n?HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,GPIO_PIN_RESET))
//DIR:1(CW)/0(CCW)
#define M_DIR(n)        (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET))
#define M_PWM(n)        (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET))
//BRAKE:0(煞車開啟)/1(煞車關閉)
#define M_BRAKE(n)      (n?HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET))
//TYPE:0(無刷馬達)/1(有刷馬達)
#define M_TYPE(n)       (n?HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET))
#define M_FAULT         (HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_9))
//其它IO
#define AE800_PWRON(n)  (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET))
#define SENSE_500MA(n)  (n?HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_RESET))
#define SENSE_5A(n)     (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET))

//按鈕LED輸出
#define BTN_LED_SIG_W(n)  (n?HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET):HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET))
#define BTN_LED_SIG_R(n)  (n?HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET))

#define BTN_LED_W_TOGGLE  (HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_8))
#define BTN_LED_R_TOGGLE  (HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_6))

//風扇輸出
#define FAN_ON(n)      (n?HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET))

#define LPA_S0          (HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_11))
#define LPA_S3          (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15))
#define LPA_S4          (HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_15))
#define LPA_S5          (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14))

#define DIR_CW          (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7))
#define DIR_CCW         (HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_1))

//偵測編碼器5V電源 1:OK 0:fail
#define ENCODER_PWR     (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8))

void GPIO_Init(void);

#endif