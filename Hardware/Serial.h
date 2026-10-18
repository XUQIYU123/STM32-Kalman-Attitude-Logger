#ifndef __SERIAL_H
#define	__SERIAL_H

#include "stm32f10x.h" 
extern volatile uint8_t RX_PACKET[];

void Serial_Init(void);


void Serial_SendByte(uint8_t Data);

//发送数字,将adc值输出到串口上
void Serial_SendNum(uint16_t num);

//发送字符串
void Serial_SendString(char *str);

void Serial_SendHex(uint8_t dat);








#endif
