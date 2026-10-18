#include "stm32f10x.h"
#include "Delay.h"
#include "GPIO_SEX.h"
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = KEY_1 | KEY_2;
	//gpio输出速度,输入模式下没用
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	RCC_APB2PeriphClockCmd(KEY_CLOCK,ENABLE);
	GPIO_Init(KEY_PORT,&GPIO_InitStructure);
	GPIO_SetBits(KEY_PORT,KEY_1 | KEY_2);
}

void WaitKeyRelease(uint16_t pin, uint16_t timeout_ms)
{
    uint16_t t = 0;
    // 循环继续的【两个条件必须同时成立】
    // 条件1：按键仍然按住 == 低电平
    // 条件2：计时t还没超过超时时间
    while( GPIO_ReadInputDataBit(KEY_PORT,pin)==0 && t < timeout_ms )
    {      
		Delay_ms(1);
        t++;
    }
    // 退出场景二选一：
    // ①按键松开 → GPIO读到1 → 第一个条件不成立 → 直接跳出while
    // ②按键一直按着，t计满超时 → 第二个条件不成立 → 跳出while
}



uint8_t KEY_GetNum(void)
{
	uint8_t keynum = 0;
	if(GPIO_ReadInputDataBit(KEY_PORT,KEY_1) == 0)
	{
		Delay_ms(20);
		WaitKeyRelease(KEY_1,150);
		Delay_ms(20);
		keynum =1;
		
	}
	if(GPIO_ReadInputDataBit(KEY_PORT,KEY_2) == 0)
	{
		Delay_ms(20);
		WaitKeyRelease(KEY_2,150); //如果按键一直按下就卡在这里
		Delay_ms(20);
		keynum =2;
		
	}
	return keynum;
}
	