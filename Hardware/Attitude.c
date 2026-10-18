#include "stm32f10x.h"
#include "MPU6050_REG.h"
#include "Kalman.h"
#include "Attitude.h"
#include "Delay.h"
#include "MPU6050.h"
#include <math.h>

#define RAD_TO_DEG   57.29578f
#define DEG_TO_RAD   0.01745329f

/* ---------- 灵敏度（根据你的配置改） ---------- */
// 陀螺仪：±250°/s  →  131.0    （±2000°/s 用 16.4）
#define GYRO_SENS    131.0f
// 加速度：±2g      →  16384.0  （±16g 用 2048.0）
#define ACCEL_SENS   16384.0f

/* ---------- 两个独立卡尔曼滤波器 ---------- */
static Kalman_t kf_pitch;
static Kalman_t kf_roll;

/* ---------- 全局姿态 ---------- */
Attitude_t attitude = {0, 0, 0};

/* ---------- 陀螺仪零偏 ---------- */
static float gyro_offset_x = 0.0f;
static float gyro_offset_y = 0.0f;
static float gyro_offset_z = 0.0f;




/* ============================================================
 * 卡尔曼初始化 + 陀螺仪零偏校准
 * ============================================================ */
void Attitude_Init(void)
{
    /* 参数说明：
       Q_angle  ：角度过程噪声，越大越相信加速度计（跟得快但抖）
       Q_bias   ：零偏过程噪声，越大零偏修正越快
       R_measure：观测噪声，越大越相信陀螺仪（平滑但漂）
       这三个数没有"标准答案"，一般经验值如下          */
    Kalman_Init(&kf_pitch, 0.001f, 0.003f, 0.03f);
    Kalman_Init(&kf_roll,  0.001f, 0.003f, 0.03f);

    Attitude_CalibrateGyro();   // 开机静止校准
}


/* ============================================================
 * 陀螺仪零偏校准：板子必须完全静止
 * ============================================================ */
void Attitude_CalibrateGyro(void)
{
    MPU6050_Data_t data;
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;

    for (int i = 0; i < 500; i++) {
        MPU6050_GETDATA(&data);
        sum_x += data.gyrox;
        sum_y += data.gyroy;
        sum_z += data.gyroz;
        // 如果你有 delay_ms，就用；没有就空转一点点
       // for (volatile int k = 0; k < 4000; k++);   // 约2ms @72MHz
		Delay_ms(2);
    }

    gyro_offset_x = sum_x / 500.0f;
    gyro_offset_y = sum_y / 500.0f;
    gyro_offset_z = sum_z / 500.0f;
}




/* ============================================================
 * 姿态更新：建议每 10ms 调用一次（dt = 0.01f）
 * ============================================================ */
void Attitude_Update(float dt)
{
    MPU6050_Data_t data;
    MPU6050_GETDATA(&data);

    /* ---------- 1. 转物理量 ---------- */
    float ax_g = data.accx / ACCEL_SENS;   // 单位 g
    float ay_g = data.accy / ACCEL_SENS;
    float az_g = data.accz / ACCEL_SENS;

    // 陀螺仪转 rad/s，并减去零偏
    float gx = (data.gyrox - gyro_offset_x) / GYRO_SENS * DEG_TO_RAD;
    float gy = (data.gyroy - gyro_offset_y) / GYRO_SENS * DEG_TO_RAD;
    float gz = (data.gyroz - gyro_offset_z) / GYRO_SENS * DEG_TO_RAD;

    /* ---------- 2. 加速度计计算绝对角度（rad） ---------- */
    // Pitch：绕Y轴，前后倾
    float pitch_acc = atan2f(-ax_g, sqrtf(ay_g * ay_g + az_g * az_g));
    // Roll ：绕X轴，左右倾
    float roll_acc  = atan2f(ay_g, az_g);

    /* ---------- 3. 卡尔曼滤波（输入rad、输出rad） ---------- */
    float pitch_rad = Kalman_Update(&kf_pitch, gy, pitch_acc, dt);
    float roll_rad  = Kalman_Update(&kf_roll,  gx, roll_acc,  dt);

    /* ---------- 4. 转成角度存入全局 ---------- */
    attitude.pitch = pitch_rad * RAD_TO_DEG;
    attitude.roll  = roll_rad  * RAD_TO_DEG;

        /* ---------- 5. Yaw 积分 + 静止零偏跟踪 ---------- */
    // 判断是否静止（加速度模长接近1g）
    float acc_mag = sqrtf(ax_g*ax_g + ay_g*ay_g + az_g*az_g);
    uint8_t is_static = (fabsf(acc_mag - 1.0f) < 0.05f) ? 1 : 0;
    
    // 静止时缓慢修正陀螺仪零偏，跟踪温度漂移
    if(is_static)
    {
        // 跟踪系数0.001，越小越平滑、响应越慢
        gyro_offset_z += (data.gyroz - gyro_offset_z) * 0.001f;
    }
    
    // 重新计算z轴角速度
    gz = (data.gyroz - gyro_offset_z) / GYRO_SENS * DEG_TO_RAD;
    attitude.yaw += gz * RAD_TO_DEG * dt;
    
    // 可选：角度归一化到 ±180°
    while(attitude.yaw > 180.0f) attitude.yaw -= 360.0f;
    while(attitude.yaw < -180.0f) attitude.yaw += 360.0f;

}