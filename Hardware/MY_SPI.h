#ifndef __MY_SPI_H
#define __MY_SPI_H
#include "stm32f10x.h"                  // Device header

void MY_SPI_INIT(void);



//stm32 片选 选择从机
void SPI_W_SS(uint8_t bit);

//初始化
void MYSPI_INIT(void);

//时序
//起始条件
void MY_SPI_START(void);

//停止条件
void MY_SPI_STOP(void);

//交换字节时序 这种可以保留原始参数send_byte,可以在里面继续使用
uint8_t MY_SPI_SwapByte(uint8_t send_byte);

//交换字节时序2 效率更高 但是send_byte会被变成0x00,执行完循环后,内部就不能继续使用send_byte原始参数了
uint8_t MY_SPI_SwapByte2_Mode0(uint8_t send_byte);

//模式1 模式2和模式3只需翻转sck就行
uint8_t MY_SPI_SwapByte_Mode1(uint8_t send_byte);




#endif
