#include "StepperMotor_Loop.hpp"

PLL_Parameter pll_conf(FOC_PLL_KP, FOC_PLL_KI);

KTH7111 kth7111_A(KTH7111_hspi1, SP1_CS_PORT, SP1_CS_PIN); //编码器对象，使用SPI通信，指定片选引脚
KTH7111 kth7111_B(KTH7111_hspi2, SP2_CS_PORT, SP2_CS_PIN); //编码器对象，使用SPI通信，指定片选引脚


StepperMotor M1(&Motor_M1, &kth7111_A, 1.0f,0.5f,0.1f,  1.0f,0.0f,0.0f,true,M1_MS1_GPIO_Port, M1_MS1_Pin);
StepperMotor M2(&Motor_M2, &kth7111_B, 1.0f,0.5f,0.1f,  1.0f,0.0f,0.0f,true,M2_MS1_GPIO_Port, M2_MS1_Pin);

// StepperMotor M1(&Motor_M2, &kth7111_B, 1.0f,0.5f,0.1f,  1.0f,0.0f,0.0f,true,M2_MS1_GPIO_Port, M2_MS1_Pin);
// StepperMotor M2(&Motor_M1, &kth7111_A, 1.0f,0.5f,0.1f,  1.0f,0.0f,0.0f,true,M1_MS1_GPIO_Port, M1_MS1_Pin);



void StepperMotor:: My_Stepper_Init()
{
    
    LPFAndPLL = false;//默认使用低通滤波角度
    speed_lpf.init(0.7); //初始化速度低通滤波器
    error_lpf.init(0.7); //初始化误差低通滤波器
    speed_avg.init(5); //初始化速度平均值滤波器
    _encoder->KTH7111_Init(motor_encoder_dir); //初始化编码器 
    _motor->Motor_Init(); //初始化电机
    HAL_GPIO_WritePin(MS_FULL_PORT_, MS_FULL_PIN_, MS_FULL ? GPIO_PIN_RESET : GPIO_PIN_SET);
    motor_fre = 0;
    control_init_flag = true;
}

/**
 * @brief 更新低通滤波角度或锁相环角度
 * 该函数用于根据配置选择使用低通滤波或锁相环(PLL)方式来更新电机角度和速度信息
 */
void StepperMotor::Update_Speed_Angle_LPFAndPLL() //更新低通滤波角度或锁相环角度
{
    // 静态变量，用于临时存储角度值和上一次的角度值
    static float fudu_test = 0.0f;
    static float theta_m_offic_temp = 0.0;

    _encoder->KTH7111_Send_JIAO_CMD();//发送角度指令
    fudu_test = _encoder->Get_KTH7111_Radian();//获取编码器角度

    theta_m = fudu_test; // 获取原始机械角度 弧度
    
    if (LPFAndPLL == false) //低通滤波角度
    {
      theta_m = wrap_to_PI(theta_m);
      //方式1 低通滤波 更新机械角度速度
      theta_m_offic_temp = theta_m - theta_m_last;
      theta_m_last = theta_m; // 更新上一次的机械角度
      theta_m_offic = wrap_to_PI(theta_m_offic_temp);// 将差值规整到 [-PI, PI) 区间

      theta_m_offic_filtered = error_lpf.filter(theta_m_offic);//对误差角度低通滤波
      theta_m_speed = theta_m_offic_filtered*Stepper_VELOCITY_LOOP_FREQ_HZ;//speed=路程/t 
      theta_av_speed = speed_avg.filter(theta_m_speed);//对速度进行平均值滤波
      filtered_speed = speed_lpf.filter(theta_av_speed);//对速度进行低通滤波
      reg_final = theta_m; //将机械角度赋值给最终角度
      theta_deg_final = rad2deg(reg_final); //将弧度转换为360度
      Angular_velocity_final = filtered_speed;  //将最终速度赋值给最终角速度
      
    }else if (LPFAndPLL == true)
    {
      // 方式2PLL锁相环 更新PLL锁相环的输出角度和角速度
      foc_pll_run(theta_m,PLL_FREQ_Dt,&_pll_reg_out,&_pll_Angular_velocity,&pll_conf); //更新PLL锁相环的输出角度和角速度
      reg_final = _pll_reg_out; //将PLL锁相环的输出角度赋值给最终角度
      theta_deg_final = rad2deg(reg_final); //将弧度转换为360度
      Angular_velocity_final = _pll_Angular_velocity; //将最终速度赋值给最终角速度
      
    }

}


void StepperMotor::setFre()//设置电机频率
{
    if(MS_FULL_Last != (uint16_t)MS_FULL)
    {
        HAL_GPIO_WritePin(MS_FULL_PORT_, MS_FULL_PIN_, (uint16_t)MS_FULL ? GPIO_PIN_RESET : GPIO_PIN_SET);
        MS_FULL_Last = (uint16_t)MS_FULL;
    }
    if (_motor->Set_Motor_Frequency(motor_fre))  // 频率变了清零走的步数 重新开始计步
    {
        motor_step_cnt = 0; 
    }
}



void StepperMotor::Speed_Loop(void)
{
    float _delta_fre = 0.0f;
    _error_speed = _target_speed - Angular_velocity_final; //计算速度误差
    _delta_fre = pid_speed.update(_error_speed); //计算PID增量
    motor_fre += _delta_fre; //更新电机频率
    if(motor_fre > 3000) motor_fre = 3000;
    if(motor_fre < -3000) motor_fre = -3000;
    setFre(); //设置电机频率
}


void StepperMotor::Position_Loop(void)
{
    float _delta_fre = 0.0f;
    _target_error_location = _target_location1 - reg_final; //计算位置误差
    _delta_fre = pid_location.update(_target_error_location); //计算PID增量
    _target_speed +=_delta_fre; //更新目标速度
    if(_target_speed > 60.0f) _target_speed = 60.0f;
    if(_target_speed < -60.0f) _target_speed = -60.0f;
    Speed_Loop(); //位置环内部调用速度环控制
}



// ADRC adrc(20,100,0.1,1.0/Stepper_POSITION_LOOP_FREQ_HZ); //线性自抗扰控制器

// void StepperMotor::Position_Loop(void)
// {
//     motor_fre = (int16_t)adrc.update(_target_location1, reg_final);
//     if(motor_fre >  2000) motor_fre =  2000;
//     if(motor_fre < -2000) motor_fre = -2000;
//     setFre();
// }



void StepperMotor::Step_Handler()//电机计步计数控制
{
    if (!control_init_flag) return;

    if ((uint32_t)motor_step_max != motor_step_max_last_) //步数上限变化 → 清零计步
    {
        motor_step_cnt = 0;
        motor_step_max_last_ = (uint32_t)motor_step_max;
        _motor->Set_Motor_EN(true);//电机使能
    }

    // 已到上限 → 跳过
    if ((uint32_t)motor_step_max != 0 && motor_step_cnt >= (uint32_t)motor_step_max) return;
    motor_step_cnt++;//计步
    // 刚到上限 → 关电机
    if ((uint32_t)motor_step_max != 0 && motor_step_cnt >= (uint32_t)motor_step_max)
    {
        _motor->Set_Motor_EN(false);
    }
}
