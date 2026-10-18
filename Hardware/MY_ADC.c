#include "stm32f10x.h"                  
#include "GPIO_SEX.h"
#include <math.h>
//热敏和光敏
void MY_ADC_Init(void)
{
	RCC_APB2PeriphClockCmd(ADC_CLOCK,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	//ADC专属模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = NTC_ADC | LDR_ADC	;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ADC_PORT,&GPIO_InitStructure);

	
	//ADC初始化
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //只用ADC1，独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;       //非扫描
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//单次转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //软件触发
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1; //非扫描，通道数量=1

	ADC_Init(ADC1,  &ADC_InitStructure);
	
	//开启ADC
	ADC_Cmd(ADC1,ENABLE);
	
	//复位校准
	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1) == SET); //等待复位校准完成
	ADC_StartCalibration(ADC1); 
    while(ADC_GetCalibrationStatus( ADC1) == SET);    //等待ADC校准完成
}

uint16_t Read_ADC(uint8_t ch)
{
	//配置要采集的通道  采样周期越短，速度越快，但精度也越差，越长则相反 Rank = 排队序号
	ADC_RegularChannelConfig(ADC1, ch,1, ADC_SampleTime_55Cycles5);
	//软件启动一次转换
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	//等待EOC转换完成
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	//读取ADC值  不需要清理标志位，硬件读取后自动清理，当然不放心也可以手动清理，不影响
	//adc_val = ADC_GetConversionValue(ADC1);  //第一步读
	//ADC_ClearFlag(ADC1, ADC_FLAG_EOC);       //第二步清标志
	return ADC_GetConversionValue(ADC1); //adc_val
}


//多次采样，求取滤波平均值
uint16_t Read_ADC_VAL(uint8_t ch,uint8_t times)
{
	uint32_t sum = 0;
	if(times == 0) 
	{
		times = 1;
	}
	for(uint8_t i=0;i < times ; i++)
	{
		sum += Read_ADC(ch);
	
	}
	
	
	return (sum + times / 2 )/times; //四舍五入
}

//一阶滞后滤波 上一次结果*0.8 + 这次结果*0.2 (系数可以自己设置 0.1 ~ 0.3)

uint16_t Read_ADC_Filtered(uint8_t ch)
{
	//先做十次平均,剔除高频毛刺
	  uint16_t this_adc = Read_ADC_VAL(ch, 10);
    
    // 用数组给通道 0 和 通道 1 分别建立独立的记忆
    // ADC_Channel_0 的值是 0，ADC_Channel_1 的值是 1
    static uint16_t last_adc[2] = {0, 0}; 
    static uint8_t is_first[2] = {1, 1};
    
    if(is_first[ch])
    {
        last_adc[ch] = this_adc;
        is_first[ch] = 0;
    }
    else
    {
        // 注意这里用的是 last_adc[ch]，只跟自己上一次的值混合
        last_adc[ch] = last_adc[ch] * 0.8f + this_adc * 0.2f;
    }
    
    return last_adc[ch];
}





float NTC_GetTemp(uint16_t adc_val)
{
	
      float Vadc = adc_val * 3.3f / 4095.0f;
    
    // 上拉电阻是 10K，不是 3.3K！
    float Rntc = 10000.0f * Vadc / (3.3f - Vadc);
    
    const float B = 3950.0f;
    const float R0 = 5000.0f;
    const float T0 = 298.15f;
    
    float invT = 1.0f / T0 + (1.0f / B) * logf(Rntc / R0);
    float Temp_K = 1.0f / invT;
    float Temp_C = Temp_K - 273.15f;
	
    return Temp_C;
}

// 光敏
float LDR_GetLux(uint16_t adc_val)
{
    // 除零保护
    if (adc_val < 10)   adc_val = 10;
    if (adc_val > 4085) adc_val = 4085;

    float Vadc = adc_val * 3.3f / 4095.0f;

    // ✅ 修正：10k上拉、LDR下拉
    // Vadc = 3.3 × Rldr / (Rldr + 10k)
    // 反推：Rldr = 10000 × Vadc / (3.3 - Vadc)
    float Rldr = 10000.0f * Vadc / (3.3f - Vadc);

    // lux 公式不变（GL5528）
    float lux = powf(50000.0f / Rldr, 1.0f / 0.7f);

    return lux;
}