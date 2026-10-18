#include "stm32f10x.h"                  // Device header
#include "GPIO_SEX.h"
#include "Delay.h"

// 非阻塞式需要的变量
uint32_t buzzer_tick = 0;   
uint8_t buzzer_state = 0;   // 0=空闲，1=正在发声，2=周期鸣叫中

/* ================== 初始化 ================== */
void BUZZER_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(BUZZER_CLOCK, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
	GPIO_SetBits(BUZZER_PORT, BUZZER_PIN); // 默认拉高，关闭蜂鸣器
}

/* ================== 阻塞式（仅用于开机测试，千万别放主循环） ================== */
void Buzzer_BeepCycle(uint16_t time1, uint16_t time2, uint8_t num)
{
	for(uint8_t i = 0; i < num; i++)
	{
		Delay_ms(time1);
		BUZZER_ON;
		Delay_ms(time2);
		BUZZER_OFF;
	}
}

/* ================== 核心：单次非阻塞短鸣（按键专用） ================== */
// 调用一次，响 50ms，然后自动关。不卡死主循环。
void Buzzer_Beep(void)
{
    BUZZER_ON;
    buzzer_state = 1;  // 标记正在响
    buzzer_tick = 0;   // 重新开始计时
}

/* ================== 非阻塞周期鸣叫 ================== */
// 必须在 1ms 定时器中断或者主循环 1ms 任务里调用
void Buzzer_Beep_NoBlock(uint16_t interval_ms, uint16_t sound_ms)
{
    // 如果蜂鸣器当前是关闭的，且没有在报警
    if (buzzer_state == 0) {
        buzzer_tick++;
        if (buzzer_tick >= interval_ms) {
            buzzer_tick = 0;
            buzzer_state = 2; // 进入报警状态
            BUZZER_ON;
        }
    }
    // 如果正在报警
    else if (buzzer_state == 2) {
        buzzer_tick++;
        if (buzzer_tick >= sound_ms) {
            buzzer_tick = 0;
            buzzer_state = 0; // 回到空闲
            BUZZER_OFF;
        }
    }
}

/* ================== 供主循环调用的 1ms 滴答函数 ================== */
// 不管主循环多忙，只要保证 1ms 调一次这个函数，按键提示音就能自动关闭
void Buzzer_Tick(void)
{
    if (buzzer_state == 1) { // 如果是单次短鸣
        buzzer_tick++;
        if (buzzer_tick >= 50) { // 响 50ms
            BUZZER_OFF;
            buzzer_state = 0;    // 自动关闭
            buzzer_tick = 0;
        }
    }
}