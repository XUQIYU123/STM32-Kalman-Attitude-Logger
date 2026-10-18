#include "stm32f10x.h"                  // Device header

volatile uint8_t TIM_Flag = 0;  //全局中断标志位  加vo防止编译器优化
volatile uint8_t TIM_Flag_10ms = 0;
volatile uint8_t TIM_Flag_100ms = 0;
volatile uint8_t TIM_Flag_200ms = 0;
volatile uint8_t TIM_Flag_500ms = 0;
volatile uint8_t TIM_Flag_1000ms = 0; 
void MY_TIM_Init(void)
{
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	//开启TIM2时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	
	//配置时基单元
	TIM_TimeBaseInitStructure.TIM_ClockDivision =  TIM_CKD_DIV1;           //时钟分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数模式，向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 9999; //ARR自动重装值
	TIM_TimeBaseInitStructure.TIM_Prescaler =  71;  ////PSC预分频器

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	//开启更新中断
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //打开TIM2更新中断
	
	
	
	
	
	
	//配置NVIC
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;   //中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd  = ENABLE;	//使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 2;   //抢占优先级	父优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;				//响应优先级	子优先级
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2,ENABLE);
}



void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET) //读取更新中断事件标志位，判断是否成功
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清理标志位
		TIM_Flag++;
		TIM_Flag_10ms = 1;
		if(TIM_Flag % 10 ==0)
		{
			TIM_Flag_100ms = 1;
		}
		if(TIM_Flag % 20 ==0)
		{
			TIM_Flag_200ms = 1;
		}
		if(TIM_Flag % 50 ==0)
		{
			TIM_Flag_500ms = 1;
		}
		if(TIM_Flag % 100 ==0)
		{
			TIM_Flag_1000ms = 1;
			TIM_Flag = 0;
		}
		
		
		
	}
	
	
	
}