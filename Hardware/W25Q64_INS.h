#ifndef __W25Q54_INS_H
#define __W25Q54_INS_H
//W25Q64指令码

#define Write_Enable								0x06   	//开启使能
#define Write_Disable								0x04   	//关闭使能
#define W25Q64_READ_STATUS_REGISTER_1   			0x05    //读状态寄存器1
#define W25Q64_READ_STATUS_REGISTER_2   			0x35    //读状态寄存器2
#define W25Q64_WRITE_STATUS_REGISTER    			0x01    //写状态寄存器
#define W25Q64_PAGE_PROGRAM            				0x02    //页编程(页写入)
#define W25Q64_QUAD_PAGE_PROGRAM        			0x32    //四线页编程
#define W25Q64_BLOCK_ERASE_64KB         			0xD8    //64KB块擦除
#define W25Q64_BLOCK_ERASE_32KB         			0x52    //32KB块擦除
#define W25Q64_SECTOR_ERASE_4KB         			0x20    //4KB扇区擦除
#define W25Q64_CHIP_ERASE               			0xC7    //整片擦除
#define W25Q64_ERASE_SUSPEND            			0x75    //擦除暂停
#define W25Q64_ERASE_RESUME             			0x7A    //擦除恢复
#define W25Q64_POWER_DOWN               			0xB9    //掉电模式
#define W25Q64_HIGH_PERFORMANCE_MODE    			0xA3    //高性能模式
#define W25Q64_CONTINUOUS_READ_MODE_RESET 			0xFF  //连续读模式复位
#define W25Q64_RELEASE_POWER_DOWN_HPM_DEVICE_ID 	0xAB //退出掉电/读设备ID
#define W25Q64_MANUFACTURER_DEVICE_ID   			0x90    //读厂商与设备ID
#define W25Q64_READ_UNIQUE_ID           			0x4B    //读唯一ID
#define W25Q64_JEDEC_ID                				0x9F    //读JEDEC ID(厂商+设备ID)
#define W25Q64_READ_DATA               			 	0x03    //标准读数据
#define W25Q64_FAST_READ                			0x0B    //快速读
#define W25Q64_FAST_READ_DUAL_OUTPUT    			0x3B    //双线输出快速读
#define W25Q64_FAST_READ_DUAL_IO        			0xBB    //双线IO快速读
#define W25Q64_FAST_READ_QUAD_OUTPUT    			0x6B    //四线输出快速读
#define W25Q64_FAST_READ_QUAD_IO        			0xEB    //四线IO快速读
#define W25Q64_OCTAL_WORD_READ_QUAD_IO  			0xE3    //四线IO八字读

#define W25Q64_DUMMY_BYTE               			0xFF    //SPI空读填充字节




#endif
