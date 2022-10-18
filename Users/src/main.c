#include "../../BSP/system.h"

ae800_objectTypeDef 	*ae800_device;
AD5171_objectTypeDef	ad5171_object;

//定義短路偵測的壓降，當量測電壓在短時間內小於這個數值將會判定短路
//並關閉馬達輸出
#define SHORT_VOLTAGE_SENSING 	2.5
#define MOTOR_SENSING_SWITCH	0.45

const uint8_t current_version=020;
const uint8_t updae_YY=22;
const uint8_t updae_MM=6;
const uint8_t updae_DD=21;

void AppTaskStart(void *argument);
void AppTaskWorking(void *argument);

const osThreadAttr_t ThreadStart_Attr = 
{
	.name = "ThreadStart",
	.attr_bits = osThreadDetached, 
	.priority = osPriorityNormal4,
	.stack_size = 2048,
};

const osThreadAttr_t ThreadWorking_Attr = 
{
	.name = "WorkingThread",
	.attr_bits = osThreadDetached, 
	.priority = osPriorityNormal4,
	.stack_size = 4096,
};

osThreadId_t ThreadIdStart = NULL;
osThreadId_t ThreadIdTaskWorking = NULL;

int main (void) 
{	
	system_init();

	HAL_SuspendTick();
	osKernelInitialize();                                  
	ThreadIdStart = osThreadNew(AppTaskStart, NULL, &ThreadStart_Attr);  
	
    osKernelStart();
	
	while(1);
}

void AppTaskStart(void *argument)
{
	const uint16_t usFrequency = 1; 
	uint32_t tick;

	HAL_ResumeTick();

	BSP_Init();

	ThreadIdTaskWorking = osThreadNew(AppTaskWorking, NULL, &ThreadWorking_Attr);  

	tick=osKernelGetTickCount(); 	
    while(1)
    {
		tick += usFrequency;                          
		osDelayUntil(tick);
    }
}

void AppTaskWorking(void *argument)
{
	uint32_t 	count=0;
	uint8_t 	led_count=0;
	uint8_t		system_para[36]={0};
	uint8_t		dir_flag=0;
	// uint16_t 	current_gain=0;
	// uint16_t    gain_x100=0;
	double		gain_value=0.0;
	uint8_t		current_step=0;
	uint16_t	short_sensing_delay_start=0;
	uint8_t		pc_state=0;
	uint16_t	power_on_off_delay_count=0;
	uint8_t		power_on_off_flag=0;

	usb_device_init(&system_para,36);
	ae800_device=ae800_init(&system_para);
	AD5171_init(&ad5171_object,AD5171_100K);
	gain_value=1+(200000/(double)ad5171_object.current_resistance);
	system_para[30]=ad5171_object.current_step;

	SENSE_500MA(1);
	SENSE_5A(0);

	while(1)
	{
		count++;
		if(count%200==0)
		{
			//1s執行一次
			if(system_para[27]==0)
			{
				LED_TOGGLE;
			}
		}
		else if(count%20==0)
		{
			//100ms執行一次
			if(system_para[27]!=0)
			{
				LED_TOGGLE;
			}

		}

		//2:轉向切換方式-0:3檔式(STOP/CW/CCW) 1:遠端
		//3:轉向設定(方式須設為遠端才會作用)-0:stop 1:CW 2:CCW
		if(system_para[2]==1)
		{
			if(system_para[3]==0)
			{
				M_SLEEP(0);
				M_DIR(0);
				M_PWM(0);
				M_BRAKE(0);
			}
			else if(system_para[3]==1)
			{
				M_SLEEP(1);
				M_DIR(1);
				M_PWM(1);
				M_BRAKE(1);
			}
			else if(system_para[3]==2)
			{
				M_SLEEP(1);
				M_DIR(0);
				M_PWM(1);
				M_BRAKE(1);
			}
		}

		//4:馬達種類-0:BLDC 1:BDC
		if(system_para[4]==0)
		{
			M_TYPE(0);
		}
		else if(system_para[4]==1)
		{
			M_TYPE(1);
		}

		//5:檢測模式-0:CHA-馬達電流 CHB-馬達電流 1:CHA-馬達電流 CHB-HB 2:CHA-HA CHB-HB
		if(system_para[5]==0)
		{
			CSD_PHD_1(0);
			CSD_PHD_2(0);
		}
		else if(system_para[5]==1)
		{
			CSD_PHD_1(0);
			CSD_PHD_2(1);
		}
		else if(system_para[5]==2)
		{
			CSD_PHD_1(1);
			CSD_PHD_2(1);
		}
		
		//6:初始化狀態
		if(system_para[6]==0)
		{
			//讀取500mA檔位的初始值
			system_para[25]=0;
			SENSE_500MA(1);
			SENSE_5A(0);
		}
		else if (system_para[6]==1)
		{
			//讀取5A檔位的初始值
			system_para[25]=1;
			SENSE_500MA(0);
			SENSE_5A(1);
		}
		else if(system_para[6]==2)
		{
			//完成讀取2個檔位的初始值
			//依輸出電流切換電流採樣檔
			if(ae800_device->output_current>MOTOR_SENSING_SWITCH)
			{
				system_para[25]=1;
				SENSE_500MA(0);
				SENSE_5A(1);
			}
			else
			{
				system_para[25]=0;
				SENSE_500MA(1);
				SENSE_5A(0);
			}
		}
		
		//取得模組運作狀態
		//11:皆正常 10:馬達驅動異常 1:編碼器電源異常 0:皆異常
		system_para[23]=ENCODER_PWR*10+M_FAULT;

		//取得目前電腦狀態
		if(LPA_S0==1&&LPA_S3==0&&LPA_S4==0)
		{
			//上電但未開機

			//當系統由睡眠或開機進入到關機狀態後
			if(pc_state!=0)
			{
				power_on_off_flag=1;
				pc_state=0;
				power_on_off_delay_count=0;
			}

			if(power_on_off_flag==1)
			{
				//電腦關閉5秒後關閉電源
				power_on_off_delay_count++;
				if(power_on_off_delay_count>1000)
				{
					FAN_ON(0);
					AE800_PWRON(0);
					ae800_device->is_power_on=0;
					ae800_device->ae800_device_turned_off(ae800_device);
					power_on_off_flag=0;
					power_on_off_delay_count=0;
				}
			}
			system_para[26]=0;
			BTN_LED_SIG_W(0);
			BTN_LED_SIG_R(1);
		}
		else if(LPA_S0==1&&LPA_S3==1&&LPA_S4==1)
		{
			//開機

			//當系統由睡眠或開機進入到關機狀態後
			if(pc_state!=1)
			{
				power_on_off_flag=1;
				pc_state=1;
				power_on_off_delay_count=0;
			}

			if(power_on_off_flag==1)
			{
				//電腦開啟5秒後開啟電源
				power_on_off_delay_count++;
				if(power_on_off_delay_count>1000)
				{
					FAN_ON(1);
					AE800_PWRON(1);
					ae800_device->is_power_on=1;
					power_on_off_flag=0;
					power_on_off_delay_count=0;
				}
			}

			system_para[26]=1;
			BTN_LED_SIG_W(1);
			BTN_LED_SIG_R(0);
		}
		else if(LPA_S0==1&&LPA_S3==0&&LPA_S4==1)
		{
			//進入睡眠
			system_para[26]=2;
			led_count++;
			if(led_count>200)
			{
				BTN_LED_W_TOGGLE;
				BTN_LED_SIG_R(0);
				led_count=0;
			}
		}
		
		if(system_para[13]==1)
		{
			system_para[27]=0;
			system_para[13]=0;
		}

		system_para[31]=current_version;
		system_para[32]=updae_YY;
		system_para[33]=updae_MM;
		system_para[34]=updae_DD;
		system_para[35]=0xff;

		//依照Gain設定值調整電阻
		// gain_x100=(uint16_t)(system_para[12]<<8)+system_para[13];
		if(system_para[12]!=current_step)
		{
			// uint16_t res_value=0;
			// gain_value=(double)gain_x100/100;
			// res_value=200000/(gain_value-1);
			// ad5171_object.ad5171_set_target_resistance(&ad5171_object,res_value);
			ad5171_object.ad5171_set_step(&ad5171_object,system_para[12]);
			gain_value=1+(200000/(double)ad5171_object.current_resistance);
			system_para[30]=ad5171_object.current_step;
			current_step=system_para[12];
		}

		system_para[28]=(uint8_t)((uint16_t)(gain_value*100)>>8);;
		system_para[29]=(uint8_t)((uint16_t)(gain_value*100));;

		//短路偵測(AE800輸出時才會進行偵測)
		if(ae800_device->on_off_state==1)
		{
			short_sensing_delay_start++;
			if(ae800_device->set_updated==1)
			{
				short_sensing_delay_start=0;
				ae800_device->set_updated=0;
			}

			if(short_sensing_delay_start>250)
			{
				//開啟輸出約1.2秒後才開始短路偵測(因設定輸出到開啟輸出這段時間延遲)
				if(((double)ae800_device->v_set_x100/100)-ae800_device->output_voltage>SHORT_VOLTAGE_SENSING)
				{
					if(system_para[27]==0)
					{
						//關閉馬達輸出，並將Fault flag設為1
						//需由通訊清除Flag後才可再次輸出
						M_SLEEP(0);
						M_DIR(0);
						M_PWM(0);
						M_BRAKE(0);
						system_para[24]=0;
						system_para[27]=1;
					}
					else if(system_para[27]==1)
					{
						//關閉馬達輸出後電源輸出仍為短路狀態則關閉電源輸出
						system_para[11]=0;
						system_para[27]=2;
						ae800_device->short_detected=1;
					}
				}
			}
		}
		else
		{
			short_sensing_delay_start=0;
		}
		
		//取得目前控制線設定輸出方向
		if(system_para[27]==0)
		{
			dir_flag=DIR_CW*10+DIR_CCW;
			if(dir_flag==0)
			{
				//STOP
				M_SLEEP(0);
				M_DIR(0);
				M_PWM(0);
				M_BRAKE(0);
				system_para[24]=0;
			}
			else if(dir_flag==10)
			{
				//CW
				M_SLEEP(1);
				M_DIR(1);
				M_PWM(1);
				M_BRAKE(1);
				system_para[24]=1;
			}
			else if(dir_flag==1)
			{
				//CCW
				M_SLEEP(1);
				M_DIR(0);
				M_PWM(1);
				M_BRAKE(1);
				system_para[24]=2;
			}
		}
		
		osDelay(5);
	}
}
