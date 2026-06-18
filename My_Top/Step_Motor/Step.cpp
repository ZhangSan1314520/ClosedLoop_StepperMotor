#include "Step.hpp"
#include "FreeRTOS.h"
#include "task.h"


Step_Motor Motor_M1(M1_STEP_htim, M1_STEP_CHANNEL, M1_STEP_TIM, M1_STEP_CCER,
                    M1_DIR_PORT, M1_DIR_PIN, M1_EN_PORT, M1_EN_PIN);
Step_Motor Motor_M2(M2_STEP_htim, M2_STEP_CHANNEL, M2_STEP_TIM, M2_STEP_CCER,
                    M2_DIR_PORT, M2_DIR_PIN, M2_EN_PORT, M2_EN_PIN);



void Step_Motor::Set_Motor_DTR(bool dtr) //1:正转 0:反转
{
    // HAL_TIM_PWM_Stop(htim_, channel_);       // 停PWM ??? 
    HAL_GPIO_WritePin(DIR_PORT_, DIR_PIN_, dtr ? GPIO_PIN_RESET : GPIO_PIN_SET);//拉低正转，拉高反转
    for(int i=0;i<200;i++){__asm("nop");} //短暂延时，确保方向信号稳定
    // HAL_TIM_PWM_Start(htim_, channel_);      // 重开PWM ???
}


void Step_Motor::Set_Motor_EN(bool en) // true:使能电机 false:关闭电机
{
    if(en)
        _step_TIM->CCER |= (_step_ch_ccer); // 开 CH1 + CH1N
    else
        _step_TIM->CCER &= ~(_step_ch_ccer); // 关 CH1 + CH1N
    HAL_GPIO_WritePin(EN_PORT_, EN_PIN_, en ? GPIO_PIN_RESET : GPIO_PIN_SET);//拉低使能
    for(int i=0;i<200;i++){ __asm("nop"); } //短暂延时，确保使能信号稳定 1 nop = 1/64MHz = 15.6 纳秒
    
}


void Step_Motor::Set_Motor_DutyCycle(float DutyCycle)   //设置占空比
{
    uint32_t Motor_ARR,CCR_VAL = 0;
    if(DutyCycle>1.0f || DutyCycle<0.0f) DutyCycle = 0.5f; 
    Motor_ARR = __HAL_TIM_GET_AUTORELOAD(htim_);
    CCR_VAL = (uint32_t)(Motor_ARR*DutyCycle);
    __HAL_TIM_SET_COMPARE(htim_, channel_, CCR_VAL);
}



#include "MC_Serial.hpp"

bool Step_Motor::Set_Motor_Frequency(float Fre) //返回 true:频率改变 false:频率未改变
{
    bool new_dir = false;
    uint32_t freq;
    uint32_t arr = 0;
    int16_t Fre_temp = (int16_t)Fre;//转整数计算
    
    // 0. 频率改变时才重新计算
    if (Fre_temp == Fre_last) return false; 
    Fre_last = Fre_temp;

    // 1. 0关电机
    if (Fre_temp == 0) 
    {
        Set_Motor_EN(false);
        return true;
    }else
    {
        Set_Motor_EN(true);
    }

    // 2. 判断方向
    if (Fre_temp > 0)
    {
        new_dir = true;
        freq    = (uint32_t)Fre_temp;
    }else if (Fre_temp < 0)
    {
        new_dir = false;
        freq    = (uint32_t)(-Fre_temp);
    }

    // 3. 只在方向改变时切 DIR
    if (new_dir != motor_direction)
    {
        Set_Motor_DTR(new_dir);
        motor_direction = new_dir;
    }

    // 4. 算 ARR
    arr = timer_clock_freq_ / freq;

    // 5. 16位定时器保护: ARR 最大 65535，最小 1
    if (arr >= 65535) arr = 65535;

    if (arr == 0) arr = 1;
    htim_->Instance->CNT = 0; // 清零计数器
    __HAL_TIM_SET_AUTORELOAD(htim_, (uint16_t)(arr - 1));//设置周期 
    __HAL_TIM_SET_COMPARE(htim_, channel_, (uint16_t)(arr/2));//设置占空比 
    return true;
}


extern void M1_Tim_Callback(TIM_HandleTypeDef *htim);
extern void M2_Tim_Callback(TIM_HandleTypeDef *htim);

void Step_Motor::Motor_Init()
{
    uint32_t sysclk = HAL_RCC_GetSysClockFreq();  // 返回 64000000 ???
    timer_clock_freq_ = sysclk / (htim_->Instance->PSC + 1);  //获取定时器时钟频率
    Set_Motor_Frequency(motor_frequency); //默认0Hz频率
    __HAL_TIM_SET_COMPARE(htim_, channel_, 0);//默认占空比为0
    HAL_TIM_PWM_Start(htim_, channel_); //启动PWM输出和中断
    Set_Motor_EN(false);//默认关闭电机
    HAL_TIM_Base_Start_IT(htim_); //启动定时器周期中断 
    // 根据实例绑定不同的定时器周期结束回调 
    if (this == &Motor_M1) htim_->PeriodElapsedCallback = M1_Tim_Callback;
    if (this == &Motor_M2)  htim_->PeriodElapsedCallback = M2_Tim_Callback;
        
    init_flag = true; //设置电机初始化完成标志位
}

