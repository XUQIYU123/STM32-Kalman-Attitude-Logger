#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"

void LED_Init(void);

//开启关闭灯1-3
void Led_On_1(void);

void Led_Off_1(void);

void Led_On_2(void);



void Led_Off_2(void);



void Led_On_3(void);



void Led_Off_3(void);


//控制灯开关
void Led_Control(uint16_t GPIO_Pin,uint8_t state);


//翻转电平
void Led_turn_1(void);


void Led_turn_2(void);


void Led_turn_3(void);

void Update_LED_Color(float temp);

#endif
