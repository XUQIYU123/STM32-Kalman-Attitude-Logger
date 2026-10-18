#ifndef __W25Q64_H
#define __W25Q64_H
#include "stm32f10x.h"                  // Device header


//写入数据前,先进行写使能 每次写操作结束后芯片会进入忙状态 读操作不会
void W25Q64_Init(void);


void W25Q64_ID(uint8_t *vendor_ID ,uint16_t *device_ID);//厂商ID 设备ID 一个8位一个16位,组合就是W25Q64地址 24位


//写使能
void W25Q64_Write_ENABLE(void);

//读状态寄存器1   Flash 的工作状态寄存器
void W25Q64_Read_Status_Register1(uint8_t *Status);
//判断芯片是否忙状态
void W25Q64_Wait_Busy(void);


//页编程 地址 数据以数组形式发出 发送多少位 页编程最多256字节 count <= 256
void W25Q64_PageProgram(uint32_t Address,uint8_t *data,uint16_t Count);


//扇区擦除 一次性擦除4kb的数据
void W25Q64_Sector_Erase(uint32_t Address);


//读取数据 起始地址 数据数组 字节数量 读取的字节不限量 写入的时候最多256字节
uint32_t W25Q64_Read_Data(uint32_t Address,uint8_t *data,uint32_t Count);

//连续多页写入数据
void W25Q64_Write_Data(uint32_t Address,uint8_t *data,uint32_t Count);

void W25Q64_Sector_Erase_Start(uint32_t Address);


// 只发页编程命令，不等待忙
void W25Q64_PageProgram_Start(uint32_t Address, uint8_t *data, uint16_t Count);

//整块擦除非阻塞
void W25Q64_ChipErase_Start(void);


// 查询Flash是否空闲 1=空闲 0=忙
uint8_t W25Q64_Is_Idle(void);


#endif
