#pragma once
#include "pid_Increment.hpp"
#include "ADRC.hpp"
#include "pll.hpp"
#include "kth_read.hpp"
#include "ex_math.hpp"
#include "Step.hpp" 

enum Work_Mode
{
    speed = 0,//速度模式
    position = 1,//位置模式
    None_Mode = 2,// 无模式
};


class StepperMotor
{
public:
    StepperMotor(Step_Motor *motor, KTH7111 *encoder,
                float skp, float ski, float skd,
                float pkp, float pki, float pkd, bool dir)
        : _motor(motor), _encoder(encoder),
        pid_speed(skp, ski, skd, 1.0/Stepper_VELOCITY_LOOP_FREQ_HZ, 200.0f, -200.0f),
        pid_location(pkp, pki, pkd, 1.0/Stepper_POSITION_LOOP_FREQ_HZ, 6.0f, -6.0f),
        motor_encoder_dir(dir)
        {}


    Work_Mode work_mode = None_Mode;
    bool control_init_flag = false;//初始化标志位
    bool LPFAndPLL = false; // false: 低通滤波角度 true: PLL锁相环角度
    float theta_m = 0.0f; //机械角度
    float theta_m_last = 0.0f; //上一次的机械角度
    float theta_m_offic = 0.0f; //机械角度的误差
    float theta_m_offic_filtered = 0.0f; //滤波后的机械角度误差
    float theta_m_speed = 0.0f; //机械角速度
    float theta_av_speed = 0.0f; //机械角速度的平均值
    float filtered_speed = 0.0f; //滤波后的机械角速度
    
    float _pll_reg_out = 0.0f; //PLL锁相环的输出角度
    float _pll_Angular_velocity = 0.0f; //PLL锁相环的输出角速度

    float reg_final = 0.0f; //最终输出的角度 (弧度)
    float theta_deg_final = 0.0f; //最终输出的角度（360度）
    float Angular_velocity_final = 0.0f; //最终输出的角速度  
    
    
    int16_t motor_fre = 0; //电机频率
    float _error_speed = 0.0f; //速度误差
    float _target_speed = 0.0f; //目标速度
    float _target_location1 = 0.0f; //目标位置 弧度
    float _target_error_location = 0.0f; //误差位置 


    void My_Stepper_Init(); //电机初始化
    void Update_Speed_Angle_LPFAndPLL(); //更新低通滤波角度或锁相环角度
    void Speed_Loop(void);//速度环控制
    void Position_Loop(void);//位置环控制
    void setFre(void);//设置电机频率

private:
    
    LowpassFilter speed_lpf;     // 速度低通滤波器
    LowpassFilter error_lpf;     // 误差低通滤波器
    AvgFilter speed_avg;     // 速度平均值滤波器

    Step_Motor *_motor; //步进电机
    KTH7111   *_encoder; //编码器
    PID_Increment pid_speed; //速度环PID
    PID_Increment pid_location; //位置环PID
    bool motor_encoder_dir; //电机编码器方向

    // ADRC adrc; //线性自抗扰控制器

};

extern StepperMotor M1, M2;



