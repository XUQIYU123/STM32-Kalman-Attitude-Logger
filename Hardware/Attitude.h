#ifndef __ATTITUDE_H
#define __ATTITUDE_H

typedef struct {
    float pitch;   // 俯仰角（度）
    float roll;    // 横滚角（度）
    float yaw;     // 偏航角（度），会漂移
} Attitude_t;

extern Attitude_t attitude;

void  Attitude_Init(void);              // 初始化：卡尔曼参数 + 陀螺零偏校准
void  Attitude_CalibrateGyro(void);     // 陀螺仪零偏校准（开机静止调用）
void  Attitude_Update(float dt);        // 每次更新姿态，dt = 采样间隔(秒)

#endif