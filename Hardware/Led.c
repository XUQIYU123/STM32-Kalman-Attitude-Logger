#include "stm32f10x.h"
#include "GPIO_SEX.h"
#include "Buzzer.h"
void LED_Init(void)
{	
	
	//gpio初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =	LED_1 | LED_2 | LED_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	//APB2外设时钟使能函数,开启时钟
	RCC_APB2PeriphClockCmd(LED_CLOCK,ENABLE);
	//配置
	GPIO_Init(LED_PORT,&GPIO_InitStructure);
	//设置高电平
	GPIO_SetBits(LED_PORT,LED_1| LED_2 | LED_3);
	
	
};

void Led_On_1(void)
{
	GPIO_ResetBits(LED_PORT,LED_1);

};

void Led_Off_1(void)
{
	GPIO_SetBits(LED_PORT,LED_1);
	
}

void Led_On_2(void)
{
	GPIO_ResetBits(LED_PORT,LED_2);

};


void Led_Off_2(void)
{
	GPIO_SetBits(LED_PORT,LED_2);
	
}


void Led_On_3(void)
{
	GPIO_ResetBits(LED_PORT,LED_3);

};


void Led_Off_3(void)
{
	GPIO_SetBits(LED_PORT,LED_3);
	
}

//控制灯开关
void Led_Control(uint16_t GPIO_Pin,uint8_t state)
{
	if( state == 0 )
	{
		GPIO_WriteBit(LED_PORT,GPIO_Pin,Bit_RESET);
	}
	else 
	{	
		GPIO_WriteBit(LED_PORT,GPIO_Pin,Bit_SET);
	}
}

//翻转电平
void Led_turn_1(void)
{
	if (GPIO_ReadOutputDataBit(LED_PORT,LED_1)==0)
	{
		GPIO_SetBits(LED_PORT,LED_1);
	}
	else
	{
		GPIO_ResetBits(LED_PORT,LED_1);
	}
}

void Led_turn_2(void)
{
	if (GPIO_ReadOutputDataBit(LED_PORT,LED_2)==0)
	{
		GPIO_SetBits(LED_PORT,LED_2);
	}
	else
	{
		GPIO_ResetBits(LED_PORT,LED_2);
	}
}

void Led_turn_3(void)
{
	if (GPIO_ReadOutputDataBit(LED_PORT,LED_3)==0)
	{
		GPIO_SetBits(LED_PORT,LED_3);
	}
	else
	{
		GPIO_ResetBits(LED_PORT,LED_3);
	}
}	

static uint8_t color_state = 0;   // 默认开机绿灯

/* ---------- 温度颜色判断（带迟滞） ---------- */
void Update_LED_Color(float temp)
{
    switch (color_state)
    {
        /* ===== 当前是绿色：要变黄，必须高于35.5 ===== */
        case 0:
            if (temp >= 35.5f)
            {
                color_state = 1;   // 切黄
            }
            // 低于35.5，保持绿，啥也不做
            break;

        /* ===== 当前是黄色：要变绿或变红，各有阈值 ===== */
        case 1:
            if (temp < 34.5f)
            {
                color_state = 0;   // 回绿
            }
            else if (temp >= 40.5f)
            {
                color_state = 2;   // 变红
            }
            // 29.5 ~ 35.5 之间，保持黄
            break;

        /* ===== 当前是红色：要变黄，必须低于39.5 ===== */
        case 2:
            if (temp < 39.5f)
            {
                color_state = 1;   // 回黄
            }
            break;
    }

    /* ===== 根据 color_state 控制实际 LED ===== */
    switch (color_state)
    {
        case 0:   // 绿
            Led_On_1();
            Led_Off_2();
			Led_Off_3();
			BUZZER_OFF;
            break;
        case 1:   // 黄
            Led_On_2();
            Led_Off_3();
			Led_Off_1();
			BUZZER_ON;
            break;
        case 2:   // 红
            Led_On_3();
            Led_Off_1();
			Led_Off_2();
			BUZZER_ON;
            break;
    }
}