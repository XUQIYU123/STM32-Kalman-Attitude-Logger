#include "stm32f10x.h"                  // Device header
#include "GPIO_SEX.h"
#include "MY_SPI.h"
#include "W25Q64_INS.h" //驱动模块
//写入数据前,先进行写使能 每次写操作结束后芯片会进入忙状态 读操作不会
void W25Q64_Init(void)
{
	
	MY_SPI_INIT();
	
	
	
}


void W25Q64_ID(uint8_t *vendor_ID ,uint16_t *device_ID)//厂商ID 设备ID 一个8位一个16位,组合就是W25Q64地址 24位
{
	MY_SPI_START();
	MY_SPI_SwapByte(W25Q64_JEDEC_ID);
	*vendor_ID = MY_SPI_SwapByte(W25Q64_DUMMY_BYTE); //得带指针,不带指针改动的是这个数据的地址,带了才是数据
	*device_ID <<= 8;
	*device_ID |= MY_SPI_SwapByte(W25Q64_DUMMY_BYTE);
	MY_SPI_STOP();
}

//写使能
void W25Q64_Write_ENABLE(void)
{
	MY_SPI_START();
	MY_SPI_SwapByte(Write_Enable);
	MY_SPI_STOP();
}

//读状态寄存器1   Flash 的工作状态寄存器
void W25Q64_Read_Status_Register1(uint8_t *Status)
{
	MY_SPI_START();
	MY_SPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
	*Status = MY_SPI_SwapByte(W25Q64_DUMMY_BYTE);
	MY_SPI_STOP();
	
}

//判断芯片是否忙状态
void W25Q64_Wait_Busy(void)
{	
	uint32_t timeout = 100000;
	MY_SPI_START();
	MY_SPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
	while((MY_SPI_SwapByte(W25Q64_DUMMY_BYTE) &0x01) == 0x01)
	{
		timeout--;
		if(timeout == 0)
		{
			break;
		}
	}
	MY_SPI_STOP();
}

//页编程 地址 数据以数组形式发出 发送多少位 页编程最多256字节 count <= 256
void W25Q64_PageProgram(uint32_t Address,uint8_t *data,uint16_t Count)
{
	W25Q64_Write_ENABLE();
	MY_SPI_START();
	MY_SPI_SwapByte(W25Q64_PAGE_PROGRAM );
	MY_SPI_SwapByte(Address >> 16);
	MY_SPI_SwapByte(Address >> 8 );
	MY_SPI_SwapByte(Address);
	for(uint16_t i = 0; i < Count;i++)
	{
		MY_SPI_SwapByte(data[i]);
	}
	MY_SPI_STOP();
	//W25Q64_Wait_Busy();//事后等待 安全性高  事前等待 效率高
}

//扇区擦除 一次性擦除4kb的数据
void W25Q64_Sector_Erase(uint32_t Address)
{
	W25Q64_Write_ENABLE();
	MY_SPI_START();
	MY_SPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);
	MY_SPI_SwapByte(Address >> 16);
	MY_SPI_SwapByte(Address >> 8 );
	MY_SPI_SwapByte(Address);
	MY_SPI_STOP();
	//W25Q64_Wait_Busy();

}

//读取数据 起始地址 数据数组 字节数量 读取的字节不限量 写入的时候最多256字节
uint32_t W25Q64_Read_Data(uint32_t Address,uint8_t *data,uint32_t Count)
{
	MY_SPI_START();
	MY_SPI_SwapByte(W25Q64_READ_DATA);
	MY_SPI_SwapByte(Address >> 16);
	MY_SPI_SwapByte(Address >> 8 );
	MY_SPI_SwapByte(Address);
	for(uint32_t i=0;i < Count;i++)
	{
		data[i] =MY_SPI_SwapByte(W25Q64_DUMMY_BYTE);
	}
	MY_SPI_STOP();
}

//连续多页写入数据
void W25Q64_Write_Data(uint32_t Address,uint8_t *data,uint32_t Count)
{
    uint32_t page_remain; // 当前这一页，还剩下多少空位可以写
    uint32_t write_len;   // 这一轮PageProgram要写入多少字节

    // Count代表还剩多少字节没写完。只要还有数据没写完，就继续循环
    while(Count > 0)
    {
        // ========= 1. 计算当前页剩余空间 =========
        // Address % 256：拿到当前地址在【本页内的偏移量】0~255
        // 一页总共256字节，256减去偏移 = 这一页还能写多少字节
        page_remain = 256 - (Address % 256);

        // ========= 2. 确定本次写多少字节 =========
        // 两种情况：
        // 情况A：当前页剩余空间，比剩下待写字节少 → 写满这一页
        if(page_remain < Count)
        {
            write_len = page_remain;
        }
        // 情况B：当前页空间足够放下剩余全部数据 → 一次性写完剩下所有
        else
        {
            write_len = Count;
        }
        // 执行到这里 write_len 的范围一定是：1 ~ 256，满足PageProgram限制

        // =========3.调用底层页编程函数写入数据=========
        W25Q64_PageProgram(Address, data, (uint16_t)write_len); //因为传参是uint16_t Count 所以要转换

        // =========4.更新三个变量，准备下一轮循环=========
        Address += write_len;   // Flash地址向后移动write_len个字节
        data    += write_len;   // 内存数据指针往后跳write_len个字节
        Count   -= write_len;   // 剩余待写字节，减掉本次已经写完的数量
    }
}
void W25Q64_Sector_Erase_Start(uint32_t Address)
{
    W25Q64_Write_ENABLE();
    MY_SPI_START();
    MY_SPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);
    MY_SPI_SwapByte(Address >> 16);
    MY_SPI_SwapByte(Address >> 8);
    MY_SPI_SwapByte(Address);
    MY_SPI_STOP();
}

// 只发页编程命令，不等待忙
void W25Q64_PageProgram_Start(uint32_t Address, uint8_t *data, uint16_t Count)
{
    W25Q64_Write_ENABLE();
    MY_SPI_START();
    MY_SPI_SwapByte(W25Q64_PAGE_PROGRAM);
    MY_SPI_SwapByte(Address >> 16);
    MY_SPI_SwapByte(Address >> 8);
    MY_SPI_SwapByte(Address);
    for(uint16_t i = 0; i < Count; i++)
    {
        MY_SPI_SwapByte(data[i]);
    }
    MY_SPI_STOP();
}

// 查询Flash是否空闲 1=空闲 0=忙
uint8_t W25Q64_Is_Idle(void)
{
    uint8_t status;
    MY_SPI_START();
    MY_SPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
    status = MY_SPI_SwapByte(W25Q64_DUMMY_BYTE);
    MY_SPI_STOP();
    return (status & 0x01) ? 0 : 1;
}

// 发起整片擦除命令，不等待，立刻返回
void W25Q64_ChipErase_Start(void)
{
    W25Q64_Write_ENABLE();
    MY_SPI_START();
    MY_SPI_SwapByte(0xC7);  // W25Q64 标准整片擦除指令 Chip Erase
    MY_SPI_STOP();
}
