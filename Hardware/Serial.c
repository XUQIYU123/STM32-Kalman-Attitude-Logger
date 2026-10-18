#include "stm32f10x.h"                  // Device header
#include "GPIO_SEX.h"


volatile uint8_t usart_rx_data;
volatile uint8_t usart_rx_flag = 0;

#define PACK_LEN 4
volatile uint8_t RX_PACKET[PACK_LEN]; // ⭐ 加 volatile


//volatile`告诉编译器：变量会被中断修改，不要优化，防止主循环读不到更新后的标志位。uint8_t usart_rx_data;


void Serial_Init(void)
{
	//gpio初始化结构体 
	//APB2外设时钟使能函数,开启时钟
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin =STM_TX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	
	//配置
	GPIO_Init(STM_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin =STM_RX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(STM_PORT,&GPIO_InitStructure);
	
	
	//usart1初始化
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600; //波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None ;
	USART_InitStructure.USART_Mode =  USART_Mode_Rx  |  USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No ; //校验位 奇偶校验
	USART_InitStructure.USART_StopBits =  USART_StopBits_1  ;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	
	
	USART_Init(USART1, &USART_InitStructure);
	// USART1接收中断（RXNE中断）
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	//配置NVIC
	NVIC_InitTypeDef	NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;   //中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd  = ENABLE;	//使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 3;   //抢占优先级	父优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;				//响应优先级	子优先级
	NVIC_Init(&NVIC_InitStructure);
	USART_Cmd(USART1,ENABLE);
}



void Serial_SendByte(uint8_t Data)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	// 再写入数据
	USART_SendData(USART1, Data);
}

//发送数字,将adc值输出到串口上
void Serial_SendNum(uint16_t num)
{
	uint8_t buf[5];  // uint16_t最大65535，5位足够
	uint8_t i = 0;

	if(num == 0)
	{
		Serial_SendByte('0');
		return;
	}

	while(num > 0)
	{
		buf[i] = num % 10 + '0';
		num = num / 10;  // ✅ 加上这行，去掉个位，循环才能退出
		i++;
	}

	while(i > 0)
	{
		i--;
		Serial_SendByte(buf[i]);
	}
	
	
}




//发送字符串
void Serial_SendString(char *str) 
{ 	
	while(*str != '\0') 	
	{ 		
		Serial_SendByte(*str); 		
		str++; 	
	} 
}



//中断处理hex数据包
// 中断处理hex数据包


void USART1_IRQHandler(void)
{
    static uint8_t RX_STATE = 0;
    static uint16_t rx_ptr = 0;
    
    // ⭐ 处理溢出错误，防止串口卡死（极其重要）
    if(USART_GetITStatus(USART1, USART_IT_ORE) != RESET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_ORE);
    }
    
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        usart_rx_data = USART_ReceiveData(USART1);
        
        if(usart_rx_data == 0xFF) // 遇到包头，无条件重置状态
        {
            RX_STATE = 1;
            rx_ptr = 0;
        }
        else if(RX_STATE == 1)
        {
            if(rx_ptr < PACK_LEN) // 防越界
            {
                RX_PACKET[rx_ptr] = usart_rx_data;
                rx_ptr++;
            }
            
            if(rx_ptr >= PACK_LEN)
            {
                RX_STATE = 2; // 载荷收满，准备收包尾
            }
        }
        else if(RX_STATE == 2)
        {
            if(usart_rx_data == 0xFE)
            {
                usart_rx_flag = 1; // ✅ 收到合法帧
            }
            // 无论包尾对不对，本帧结束，回到空闲态
            RX_STATE = 0;
            rx_ptr = 0;
        }
        else
        {
            // 空闲态收到乱码，重置
            RX_STATE = 0;
            rx_ptr = 0;
        }
    }
}



// 以十六进制打印1字节（两位，补前导0）
void Serial_SendHex(uint8_t dat)
{
    uint8_t high = dat >> 4;
    uint8_t low  = dat & 0x0F;
    
    if(high < 10) Serial_SendByte('0' + high);
    else Serial_SendByte('A' + high - 10);
    
    if(low < 10) Serial_SendByte('0' + low);
    else Serial_SendByte('A' + low - 10);
}
