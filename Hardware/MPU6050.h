#ifndef __MPU6050_H
#define	__MPU6050_H
#include "MPU6050_REG.h"
#include "stm32f10x.h" 


//mpu6050因为没有磁力计,所以yaw角会随着时间不断增加偏差是吗
typedef struct {
    int16_t accx;
    int16_t accy;
    int16_t accz;
    int16_t temp;
    int16_t gyrox;
    int16_t gyroy;
    int16_t gyroz;
} MPU6050_Data_t;


//超时等待
void MPU6050_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);

//初始化
void MPU6050_Init(void);

//发送一个数据
void MPU6050_WriteReg(uint8_t ADDRESS,uint8_t data);
//接受一个数据
uint8_t  MPU6050_ReceiveReg(uint8_t ADDRESS);

//接受多个数据
void MPU6050_ReceiveRegs(uint8_t ADDRESS , uint8_t *buf,uint8_t len);

//获取传感器数据
void MPU6050_GETDATA(MPU6050_Data_t *data);



#endif
