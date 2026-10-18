#include "stm32f10x.h"
#include "GPIO_SEX.h"
#include "MPU6050_REG.h"
#include "MPU6050.h"
#include "Kalman.h"
#include <math.h>

#define MPU6050_ADDRESS 0xD0   // 7位地址0x68，左移后0xD0



/* ============================================================
 *                      软件 I2C 底层函数
 * 直接使用宏定义：MPU6050_PORT / MPU6050_SCL / MPU6050_SDA
 * ============================================================ */

// 简单延时，用于软件I2C时序
static void I2C_Delay(void)
{
    for (volatile int i = 0; i < 20; i++);   // 根据主频调整，约几微秒
}

// SCL 输出高/低
#define SCL_H()   GPIO_SetBits(MPU6050_PORT, MPU6050_SCL)
#define SCL_L()   GPIO_ResetBits(MPU6050_PORT, MPU6050_SCL)

// SDA 输出高/低
#define SDA_H()   GPIO_SetBits(MPU6050_PORT, MPU6050_SDA)
#define SDA_L()   GPIO_ResetBits(MPU6050_PORT, MPU6050_SDA)

// 读取 SDA 电平
#define SDA_READ() GPIO_ReadInputDataBit(MPU6050_PORT, MPU6050_SDA)


// 起始条件：SCL高时，SDA由高变低
static void I2C_Start(void)
{
    SDA_H();
    SCL_H();
    I2C_Delay();
    SDA_L();
    I2C_Delay();
    SCL_L();
    I2C_Delay();
}

// 停止条件：SCL高时，SDA由低变高
static void I2C_Stop(void)
{
    SDA_L();
    SCL_H();
    I2C_Delay();
    SDA_H();
    I2C_Delay();
}

// 发送一个字节（高位先发）
static void I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & 0x80)
            SDA_H();
        else
            SDA_L();
        byte <<= 1;

        I2C_Delay();
        SCL_H();
        I2C_Delay();
        SCL_L();
        I2C_Delay();
    }
}

// 接收一个字节
static uint8_t I2C_ReceiveByte(void)
{
    uint8_t byte = 0;
    SDA_H();               // 释放SDA，让从机控制

    for (uint8_t i = 0; i < 8; i++)
    {
        byte <<= 1;
        SCL_H();
        I2C_Delay();
        if (SDA_READ())
            byte |= 0x01;
        SCL_L();
        I2C_Delay();
    }
    return byte;
}

// 发送应答：ack=0 应答，ack=1 非应答
static void I2C_SendAck(uint8_t ack)
{
    if (ack)
        SDA_H();
    else
        SDA_L();

    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SCL_L();
    I2C_Delay();
}

// 接收应答
static uint8_t I2C_ReceiveAck(void)
{
    uint8_t ack;
    SDA_H();               // 释放SDA
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    ack = SDA_READ();      // 读从机应答
    SCL_L();
    I2C_Delay();
    return ack;
}


/* ============================================================
 *                      MPU6050 寄存器读写
 * ============================================================ */

void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    I2C_Start();
    I2C_SendByte(MPU6050_ADDRESS);   // 写地址
    I2C_ReceiveAck();
    I2C_SendByte(RegAddress);
    I2C_ReceiveAck();
    I2C_SendByte(Data);
    I2C_ReceiveAck();
    I2C_Stop();
}

uint8_t MPU6050_ReceiveReg(uint8_t RegAddress)
{
    uint8_t Data;
    I2C_Start();
    I2C_SendByte(MPU6050_ADDRESS);
    I2C_ReceiveAck();
    I2C_SendByte(RegAddress);
    I2C_ReceiveAck();

    I2C_Start();                     // 重复起始
    I2C_SendByte(MPU6050_ADDRESS | 0x01);  // 读地址
    I2C_ReceiveAck();
    Data = I2C_ReceiveByte();
    I2C_SendAck(1);                  // 非应答
    I2C_Stop();
    return Data;
}

void MPU6050_ReceiveRegs(uint8_t RegAddress, uint8_t *buf, uint8_t len)
{
    I2C_Start();
    I2C_SendByte(MPU6050_ADDRESS);
    I2C_ReceiveAck();
    I2C_SendByte(RegAddress);
    I2C_ReceiveAck();

    I2C_Start();
    I2C_SendByte(MPU6050_ADDRESS | 0x01);
    I2C_ReceiveAck();

    for (uint8_t i = 0; i < len; i++)
    {
        buf[i] = I2C_ReceiveByte();
        if (i == len - 1)
            I2C_SendAck(1);   // 最后一个字节NACK
        else
            I2C_SendAck(0);   // 其余ACK
    }
    I2C_Stop();
}


/* ============================================================
 *                      MPU6050 初始化
 * ============================================================ */

void MPU6050_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能 GPIOB 时钟
    RCC_APB2PeriphClockCmd(MPU6050_CLOCK, ENABLE);

    // 配置 SCL/SDA 为通用开漏输出
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin   = MPU6050_SCL | MPU6050_SDA;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MPU6050_PORT, &GPIO_InitStructure);

    // 空闲时拉高
    GPIO_SetBits(MPU6050_PORT, MPU6050_SCL | MPU6050_SDA);

    // 配置 MPU6050 寄存器（兼容芯片寄存器地址完全一致）
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1,   0x01);
    MPU6050_WriteReg(MPU6050_PWR_MGMT_2,   0x00);
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV,   0x09);
    MPU6050_WriteReg(MPU6050_CONFIG,       0x06);
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG,  0x00);
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x00);
}


/* ============================================================
 *                      读取6轴数据
 * ============================================================ */

void MPU6050_GETDATA(MPU6050_Data_t *data)
{
    uint8_t buf[14];
    MPU6050_ReceiveRegs(MPU6050_ACCEL_XOUT_H, buf, 14);

    data->accx  = (int16_t)((buf[0]  << 8) | buf[1]);
    data->accy  = (int16_t)((buf[2]  << 8) | buf[3]);
    data->accz  = (int16_t)((buf[4]  << 8) | buf[5]);
    data->temp  = (int16_t)((buf[6]  << 8) | buf[7]);
    data->gyrox = (int16_t)((buf[8]  << 8) | buf[9]);
    data->gyroy = (int16_t)((buf[10] << 8) | buf[11]);
    data->gyroz = (int16_t)((buf[12] << 8) | buf[13]);
}