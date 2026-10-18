#ifndef __UART_COMMANDS_H
#define __UART_COMMANDS_H

#include "stm32f10x.h"
//指令串口
#define CHIP_ERASE_SEC  8       // 整片擦除典型总时间，可根据实测调整
extern uint8_t  chip_erase_busy ;   // 整片擦除进行中标志：1=正在擦，0=空闲/完成
extern uint16_t erase_sec_cnt ;      // 擦除计时（秒）
void UART_Process_Command(void);

#endif