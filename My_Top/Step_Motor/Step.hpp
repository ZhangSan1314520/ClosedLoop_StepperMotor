#pragma once

#include "bsp.hpp"


class Step_Motor
{

public:
    
    uint32_t motor_step_cnt = 0; //步进电机步数
    bool motor_direction = true; //电机方向，true:正转，false:反转
    uint16_t motor_frequency = 1000; //电机频率
    float motor_duty_cycle = 0.5; //占空比
    uint8_t step_mode = 0; //步进模式，0: 电机2400步反转
    
    Step_Motor(TIM_HandleTypeDef* htim, uint32_t channel,
               GPIO_TypeDef* DIR_PORT, uint16_t DIR_PIN,
               GPIO_TypeDef* EN_PORT,  uint16_t EN_PIN):
                htim_(htim), channel_(channel),
                DIR_PORT_(DIR_PORT), DIR_PIN_(DIR_PIN),
                EN_PORT_(EN_PORT), EN_PIN_(EN_PIN)
                {}
                
    void Set_Motor_Frequency(int16_t Fre);
    void Set_Motor_DutyCycle(float DutyCycle);
    void Set_Motor_DTR(bool dtr);
    void Set_Motor_EN(bool en);
    void Motor_Init(void);
    void Step_Handler(void);


private:
    TIM_HandleTypeDef* htim_;
    uint32_t channel_;
    GPIO_TypeDef* DIR_PORT_;  
    uint16_t DIR_PIN_;
    GPIO_TypeDef* EN_PORT_;   
    uint16_t EN_PIN_;

    bool init_flag = false; //初始化标志位
    uint32_t timer_clock_freq_; //定时器时钟频率
    
};


extern Step_Motor Motor_M1,Motor_M2;
