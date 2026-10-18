#include "stm32f10x.h"                  // Device header
#include "Serial.h"
#include "Led.h"
#include "Ring_Buffer.h"
#include "W25Q64.h"

uint8_t  chip_erase_busy = 0;   // 整片擦除进行中标志：1=正在擦，0=空闲/完成
uint16_t erase_sec_cnt = 0;      // 擦除计时（秒）
void UART_Process_Command(void)
{
	switch(RX_PACKET[0])
    {
        case 0x01:
            Led_On_3(); //指令1和2,,因为和三级预警灯泡警示的代码冲突,所以灯3只会亮一下就熄灭,同样,下面只会灭一会又重新亮
		break;			//写这个指令只是为了调试串口发送与接收是否正常
            
        case 0x02:
            Led_Off_3(); 
            break;
            
        case 0x03:
            flash_addr = 0; //清空地址,从0x000000开始写,覆盖旧代码
            break;
         
		case 0x04: //根据后三个字节的地址来擦除指定扇区
			
			uint32_t s_addr = ((uint32_t)RX_PACKET[1] << 16) | ((uint32_t)RX_PACKET[2] << 8) | RX_PACKET[3];
			if(s_addr > 0x7FFFFF) break; // 非法地址直接忽略
			W25Q64_Sector_Erase(s_addr);
		
		
		
		case 0x05:
			// 用指令的后3字节拼成24位地址  来查看w25里存储的数据
			uint32_t addr = ((uint32_t)RX_PACKET[1] << 16) | ((uint32_t)RX_PACKET[2] << 8) | RX_PACKET[3];
			uint8_t buf[18];
			W25Q64_Read_Data(addr, buf, 18);
			
			Serial_SendString("\r\nAddr:");
			Serial_SendNum(addr);
			Serial_SendString("\r\n");
			for(uint8_t i=0; i<18; i++)
			{
				Serial_SendHex(buf[i]);
				Serial_SendByte(' ');
			}
			Serial_SendString("\r\n");
			break;

		case 0x06: //根据后三个字节的地址来擦除指定扇区
			
			
			if(s_addr > 0x7FFFFF) break; // 非法地址直接忽略
			W25Q64_ChipErase_Start();
			chip_erase_busy = 1;       // 标记：进入擦除状态
			erase_sec_cnt = 0;         // 计时器清零
			flash_addr = 0;             // 同步重置写入地址指针
			break;
        default:
			Serial_SendString("CMD_ERR\r\n");
            // 非法指令，可加串口错误应答，或直接忽略
            break;
    }
	
}



















