#pragma once

#include "bsp.hpp"


class Step_Motor
{

public:
    
    bool motor_direction = true; //电机方向，true:正转，false:反转
    uint16_t motor_frequency = 0; //电机频率
    float motor_duty_cycle = 0.5; //占空比
    
    Step_Motor(TIM_HandleTypeDef* htim, uint32_t channel,
               TIM_TypeDef* HTIM, uint32_t step_ch_ccer,
               GPIO_TypeDef* DIR_PORT, uint16_t DIR_PIN,
               GPIO_TypeDef* EN_PORT,  uint16_t EN_PIN):
                htim_(htim), channel_(channel),
                _step_TIM(HTIM), _step_ch_ccer(step_ch_ccer), 
                DIR_PORT_(DIR_PORT), DIR_PIN_(DIR_PIN),
                EN_PORT_(EN_PORT), EN_PIN_(EN_PIN)
                {}
                
    bool Set_Motor_Frequency(float Fre);
    void Set_Motor_DutyCycle(float DutyCycle);
    void Set_Motor_DTR(bool dtr);
    void Set_Motor_EN(bool en);
    void Motor_Init(void);

        
private:
    TIM_HandleTypeDef* htim_; //定时器句柄
    uint32_t channel_;//定时器通道
    GPIO_TypeDef* DIR_PORT_;  
    uint16_t DIR_PIN_;
    GPIO_TypeDef* EN_PORT_;   
    uint16_t EN_PIN_;

    bool init_flag = false; //初始化标志位
    uint32_t timer_clock_freq_; //定时器时钟频率

    TIM_TypeDef  *_step_TIM;       // 操作 CCER 用的定时器指针
    uint32_t      _step_ch_ccer;   // 通道掩码 (TIM_CCER_CC1E / TIM_CCER_CC2E)
    

    int16_t Fre_last = 0xff;   // 每个对象独立的上次频率 不能用 static
};


extern Step_Motor Motor_M1,Motor_M2;
