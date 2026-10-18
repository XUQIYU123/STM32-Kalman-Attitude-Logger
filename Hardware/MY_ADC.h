#ifndef __MY_ADC_H
#define	__MY_ADC_H

#include "stm32f10x.h" 
//热敏传感器

void MY_ADC_Init(void);



//多次采样求取平均值
uint16_t Read_ADC_VAL(uint8_t ch,uint8_t times);

//采样指定通道ADC
uint16_t Read_ADC(uint8_t ch);

//一阶滞后滤波
uint16_t Read_ADC_Filtered(uint8_t ch);
//获取温度
float NTC_GetTemp(uint16_t adc_val);


// GL5528光敏，上拉10k电阻，LDR接GND,获取光照
float LDR_GetLux(uint16_t adc_val);


#endif
