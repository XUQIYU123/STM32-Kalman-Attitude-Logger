#ifndef __KALMAN_H
#define __KALMAN_H

#include "stm32f10x.h"  // STM32标准头文件

// 圆周率宏，用于弧度/角度转换
#define PI  3.1415926535f

/****************************
 * 单轴卡尔曼滤波结构体
 * 一套结构体专门算一个角度（Roll 或 Pitch）
 * 两个轴就定义两个结构体变量
 ****************************/
typedef struct {
    float theta;      // 【输出】滤波后的最终姿态角，单位：弧度rad
    float bias;       // 【内部变量】估算出来的陀螺仪零偏，单位：rad/s
    float P[2][2];    // 【内部变量】协方差矩阵2x2，记录状态估计的不确定性

    // 【可调参数】三个噪声系数，初始化时传入
    float Q_angle;    // 角度过程噪声：值越大，越不信任陀螺仪预测
    float Q_bias;     // 零偏过程噪声：值越大，零偏修正速度越快
    float R_measure;  // 观测噪声：值越大，越不信任加速度计
} Kalman_t;


// 卡尔曼初始化：上电调用1次，传入三个调参
void Kalman_Init(Kalman_t *kf, float Q_angle, float Q_bias, float R_measure);

// 卡尔曼迭代：每采集一次传感器调用1次，返回滤波后的角度（rad）
// u：陀螺仪角速度（rad/s）  z：加速度计算的观测角度（rad）  dt：采样间隔（秒）
float Kalman_Update(Kalman_t *kf, float u, float z, float dt);

#endif
