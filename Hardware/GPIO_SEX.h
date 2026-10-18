#ifndef __GPIO_SEX_H
#define __GPIO_SEX_H
//引脚配置
#include "stm32f10x.h"                  // Device header


//传感器ADC
#define		NTC_ADC		  	GPIO_Pin_0 	 	//热敏传感器ADC
#define		LDR_ADC			GPIO_Pin_1 		//光敏传感器ADC
#define   	NTC_CH			ADC_Channel_0	//通道0
#define		LDR_CH			ADC_Channel_1	//通道1
#define 	ADC_PORT		GPIOA
#define 	ADC_CLOCK		RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1


//按键
#define		KEY_1			GPIO_Pin_2
#define		KEY_2			GPIO_Pin_3
#define		KEY_PORT		GPIOA
#define 	KEY_CLOCK		RCC_APB2Periph_GPIOA


//W25Q64存储器
#define 	MYSPI_SS   		GPIO_Pin_4		//A
#define		MYSPI_SCK		GPIO_Pin_5 		//A
#define 	MYSPI_MISO		GPIO_Pin_6 		//A
#define		MYSPI_MOSI		GPIO_Pin_7		//就是stm32的MOSI,MISO
#define		MYSPI_PORT		GPIOA
#define 	MYSPI_CLOCK		RCC_APB2Periph_GPIOA


//USB	串口
#define		STM_TX		GPIO_Pin_9		//A
#define		STM_RX		GPIO_Pin_10		//A
#define		STM_PORT		GPIOA
#define 	STM_CLOCK	RCC_APB2Periph_GPIOA


//OLED
#define		OLED_SCL  		GPIO_Pin_8 		//GPIOB	
#define 	OLED_SDA    	GPIO_Pin_9		//B
#define		OLED_PORT		GPIOB
#define 	OLED_CLOCK		RCC_APB2Periph_GPIOB


//LED
#define		LED_1			GPIO_Pin_5    //绿
#define		LED_2			GPIO_Pin_6	  //黄
#define		LED_3			GPIO_Pin_7	   //红
#define		LED_PORT		GPIOB
#define 	LED_CLOCK		RCC_APB2Periph_GPIOB


//蜂鸣器
#define 	BUZZER_PIN     	GPIO_Pin_1
#define 	BUZZER_PORT    	GPIOB
#define 	BUZZER_ON   	GPIO_ResetBits(BUZZER_PORT,BUZZER_PIN)  //输出低，响
#define 	BUZZER_OFF  	GPIO_SetBits(BUZZER_PORT,BUZZER_PIN)    //输出高，关闭
#define 	BUZZER_CLOCK	RCC_APB2Periph_GPIOB

//MPU6050	陀螺仪传感器
#define		MPU6050_SCL		GPIO_Pin_10 	//B
#define  	MPU6050_SDA 	GPIO_Pin_11		//B
#define		MPU6050_PORT	GPIOB
#define 	MPU6050_CLOCK	RCC_APB2Periph_GPIOB


#endif
