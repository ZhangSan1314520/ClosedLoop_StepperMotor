#pragma once

#include "ex_math.hpp"  // deg2rad

#define MOTOR_STATE_NUM 10

// uORB Topic 数据索引（发布端和订阅端共享）
enum MotorStateIdx {
    kRegFinal = 0,          // M1 最终输出角度 (弧度)
    kTargetLocation,        // M1 目标位置 (已转弧度)
    kAngularVelocity,       // M1 最终角速度
    kTargetSpeed,           // M1 目标速度
    kMotorFreApplied,       // M1 实际输出频率
    kTargetErrorLocation,   // M1 误差位置
    kM2RegFinal,            // M2 最终输出角度 (弧度)
    kM2TargetLocation,      // M2 目标位置 (已转弧度)
    kM2AngularVelocity,     // M2 最终角速度
    kM2TargetSpeed,         // M2 目标速度
    
};

typedef float motor_state_t[MOTOR_STATE_NUM]; //创建一个新的类型名称 叫motor_state_t 代表float[10] 的数组类型。
//枚举值就是索引号 


