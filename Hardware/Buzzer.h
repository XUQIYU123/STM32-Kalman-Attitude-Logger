#ifndef __BUZZER_H
#define	__BUZZER_H

#include "stm32f10x.h"  



void BUZZER_Init(void);

void Buzzer_Beep(void);
//每隔多久响一次,一次持续多久,响多少次 单位ms 阻塞式
void Buzzer_BeepCycle(uint16_t time1,uint16_t time2,uint8_t num);

//非阻塞 interval_ms 间隔时间	sound_ms 持续时间 在主函数里,得在这个外面调用delay_ms(1)才行
void Buzzer_Beep_NoBlock(uint16_t interval_ms,uint16_t sound_ms);


void Buzzer_Tick(void);
	

//翻转电平
void Buzzer_Turn(void);






#endif
