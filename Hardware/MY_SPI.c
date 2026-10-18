#include "stm32f10x.h"                  // Device header
#include "GPIO_SEX.h"

//stm32 片选 选择从机
void SPI_W_SS(uint8_t bit)
{
	GPIO_WriteBit(MYSPI_PORT,MYSPI_SS,(BitAction)bit);
}




void MY_SPI_INIT(void)
{
	//gpio初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = MYSPI_SS ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//APB2外设时钟使能函数,开启时钟
	RCC_APB2PeriphClockCmd(MYSPI_CLOCK,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	//配置
	
	GPIO_Init(MYSPI_PORT,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = MYSPI_MISO;
	GPIO_Init(MYSPI_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = MYSPI_MOSI| MYSPI_SCK;
	GPIO_Init(MYSPI_PORT,&GPIO_InitStructure);
	//SPI初始化
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//指定预分频器的值 256
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge ;    //这两个是配置时序模式
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CRCPolynomial =  7;     //CRC校验位 目前我们不用,所以选择默认参数7
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //数据字节大小
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //全双工
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB ; //高位先行
	SPI_InitStructure.SPI_Mode =  SPI_Mode_Master ;//选主从模式
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft ;//指定NSS是由软件还是硬件管理
	SPI_Init(SPI1,&SPI_InitStructure);
	SPI_Cmd(SPI1,ENABLE);
	SPI_W_SS(1);
}



//时序
//起始条件

void MY_SPI_START(void)
{
	SPI_W_SS(0);
}


//停止条件
void MY_SPI_STOP(void)
{
	SPI_W_SS(1);
}

//交换字节时序 这种可以保留原始参数send_byte,可以在里面继续使用
uint8_t MY_SPI_SwapByte(uint8_t send_byte)
{
	//不需要手动清除标志位,设备会自动清理 前提是得调用数据发送函数与读取函数
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET ); //set高电平 等待TXE为1 数据寄存器为空才可以写入数据
	SPI_I2S_SendData(SPI1, send_byte); //先把数据传入到数据寄存器中
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) != SET ); //set高电平 因为同步时序,所以等收入字节
	return SPI_I2S_ReceiveData(SPI1);
}




