#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Led.h"
#include "Key.h"
#include "OLED.h"
#include "Buzzer.h"
#include "W25Q64.h"
#include "MPU6050.h"
#include "Serial.h"
#include "MY_ADC.h"
#include "MY_TIM.h"
#include "GPIO_SEX.h"
#include "Kalman.h"
#include "Attitude.h"
#include "Ring_Buffer.h"
#include "UART_Commands.h"



extern uint8_t TIM_Flag;  //定时全局中断标志位
extern uint8_t TIM_Flag_10ms;
extern uint8_t TIM_Flag_100ms;
extern uint8_t TIM_Flag_200ms;
extern uint8_t TIM_Flag_500ms;
extern uint8_t TIM_Flag_1000ms; 
MPU6050_Data_t data;
uint8_t KEYNUM =0;
uint16_t volatile adc_ldr;
float lux;
uint16_t volatile adc_ntc;
float temp;

static uint8_t s_w25 = 0; //0 不发 1 发送
static uint8_t key_clear = 0;
static uint8_t current_page = 0; // 0=温度页, 1=姿态页

//extern volatile uint8_t record_flag;  // 0=停止记录，1=开启记录

//extern uint32_t flash_addr;      // Flash 写入地址
//extern volatile uint16_t data_cnt;        // 环形缓冲区未处理字节数

// 串口通信 (Serial.c)
extern volatile uint8_t usart_rx_flag;





int main(void)

{	
	//优先级分组  系统初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	KEY_Init();
	OLED_Init();
	BUZZER_Init();
	W25Q64_Init();
	MPU6050_Init();
	Serial_Init();
	MY_TIM_Init();
	MY_ADC_Init();
	Attitude_Init();
	
	
	
	
	
	
	while (1)

	{
		if(TIM_Flag_10ms == 1)
		{
			//10ms任务 姿态解算
			TIM_Flag_10ms = 0; //清标志！
			Attitude_Update(0.01f);
		
			
		}
		if(TIM_Flag_100ms == 1)
		{
			//100ms任务 人机交互与通信
			TIM_Flag_100ms = 0;
			KEYNUM =KEY_GetNum();
			//按键1切换页面,按键2发送数据到w25
			if(KEYNUM == 1)
			{
				
				key_clear = 1;
				current_page = !current_page; // 0变1，1变0，完美切换

				
			}
			if(KEYNUM == 2)
			{
				
				record_flag = !record_flag; // 翻转记录状态
				Buzzer_Beep();  // 滴一声
				 
			}
			
			// --- 串口指令解析 ---
			if(usart_rx_flag == 1)
			{
				usart_rx_flag = 0; // 清除接收标志
				
				UART_Process_Command();
			}
			Task_100ms_Collect();//写入环形缓冲区
			
		}
		if(TIM_Flag_200ms == 1)
		{
			//200ms任务 环境传感器采样
			TIM_Flag_200ms = 0;
			adc_ldr = Read_ADC_Filtered(LDR_CH);
			adc_ntc = Read_ADC_Filtered(NTC_CH);
			temp = NTC_GetTemp(adc_ntc);
			lux = LDR_GetLux(adc_ldr);
			Update_LED_Color(temp);
		}
		if(TIM_Flag_500ms == 1)
		{
			//500ms任务 OLED 显示刷新
			TIM_Flag_500ms = 0;
			
			//page0显示温度,光照,id
			if(current_page == 0)
			{
				
				if(key_clear == 1)
				{
					OLED_Clear();
					key_clear = 0;
				}
				uint8_t who = MPU6050_ReceiveReg(0x75);  // 读WHO_AM_I
				//OLED_ShowString(1,1,"ID:");
				//OLED_ShowHexNum(1, 6, who, 2);           // 看ID
				OLED_ShowString(1,1,"temp:");
				OLED_ShowString(2,1,"ldr:");
				//OLED_ShowNum(4,6,adc_ntc,5);
				OLED_ShowFloat(1,6,temp,2,1);
				OLED_ShowFloat(2,6,lux,2,1);
				OLED_ShowString(3,1,"REC: ");
				OLED_ShowString(4,1,"Cnt: ");
				//OLED_ShowNum(3,6,adc_ldr,5);
				
				if(record_flag == 1) OLED_ShowString(3, 6, "ON ");
				else    OLED_ShowString(3, 6, "OFF");
				OLED_ShowNum(4, 6, data_cnt, 3);
			}
			
			
			//欧拉角 页1
			if(current_page == 1)
			{
				if(key_clear == 1)
				{
					OLED_Clear();
					key_clear = 0;
				}
				
				
				OLED_ShowString(1,1,"ya:");
				OLED_ShowString(2,1,"ro");
				OLED_ShowString(3,1,"pi");
				OLED_ShowFloat(1,5,attitude.yaw,2,2);
				OLED_ShowFloat(2,5,attitude.roll,2,2);
				OLED_ShowFloat(3,5,attitude.pitch,2,2);
				
			}
			
			
		}
		if(TIM_Flag_1000ms == 1)
		{
			//1000ms任务 数据上报与心跳
			// ========== 整片擦除进度查询（仅擦除中才执行） ==========
			if(chip_erase_busy)
			{
				erase_sec_cnt++;  // 秒数+1
				
				if(W25Q64_Is_Idle())
				{
					// ===== 擦除完成，只执行一次 =====
					chip_erase_busy = 0;       // 清除忙标志，下次不再进入
					flash_addr = 0;            // 重置写入地址
					Serial_SendString("Erase: 100% Done!\r\n");
				}
				else
				{
					// ===== 擦除中，计算估算进度 =====
					uint8_t percent = (erase_sec_cnt * 100) / CHIP_ERASE_SEC;
					if(percent > 99) percent = 99;
					
					Serial_SendString("Erase: ");
					Serial_SendNum(percent);
					Serial_SendString("%\r\n");
				}
			}
			TIM_Flag_1000ms = 0;
			Task_Flash_Write();
			Serial_SendString("Buf:");
			Serial_SendNum(data_cnt);
			Serial_SendString(" Addr:");
			Serial_SendNum((uint16_t)(flash_addr >> 8)); // 发送高位地址简化显示 显示的是页数 256字节一页
			Serial_SendString("\r\n");
		

			
		}
		
	
		

	}

}
