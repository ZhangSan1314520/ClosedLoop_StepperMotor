#include "Step.hpp"
#include "FreeRTOS.h"
#include "task.h"


Step_Motor Motor_M1(M1_STEP_htim, M1_STEP_CHANNEL, M1_DIR_PORT, M1_DIR_PIN, M1_EN_PORT, M1_EN_PIN);
Step_Motor Motor_M2(M2_STEP_htim, M2_STEP_CHANNEL, M2_DIR_PORT, M2_DIR_PIN, M2_EN_PORT, M2_EN_PIN);


void Step_Motor::Set_Motor_DTR(bool dtr) //1:正转 0:反转
{
    HAL_TIM_PWM_Stop(htim_, channel_);       // 停PWM ???
    HAL_GPIO_WritePin(DIR_PORT_, DIR_PIN_, dtr ? GPIO_PIN_RESET : GPIO_PIN_SET);//拉低正转，拉高反转
    for(int i=0;i<20;i++){} //短暂延时，确保方向信号稳定
    HAL_TIM_PWM_Start(htim_, channel_);      // 重开PWM ???
}


void Step_Motor::Set_Motor_EN(bool en) // true:使能电机 false:关闭电机
{
    if(en)
        HAL_TIM_PWM_Start(htim_, channel_);      // 开PWM
    else
        HAL_TIM_PWM_Stop(htim_, channel_);       // 停PWM
    HAL_GPIO_WritePin(EN_PORT_, EN_PIN_, en ? GPIO_PIN_RESET : GPIO_PIN_SET);//拉低使能
    vTaskDelay(pdMS_TO_TICKS(1));
}


void Step_Motor::Set_Motor_DutyCycle(float DutyCycle)   //设置占空比
{
    uint32_t Motor_ARR,CCR_VAL = 0;
    if(DutyCycle>1.0f || DutyCycle<0.0f) DutyCycle = 0.5f; 
    Motor_ARR = __HAL_TIM_GET_AUTORELOAD(htim_);
    CCR_VAL = (uint32_t)(Motor_ARR*DutyCycle);
    __HAL_TIM_SET_COMPARE(htim_, channel_, CCR_VAL);
}




void Step_Motor::Set_Motor_Frequency(int16_t Fre)
{
    bool new_dir = false;
    uint32_t freq;
    uint32_t arr = 0;

    // 1. 除零保护
    if (Fre == 0) return;
        
    // 2. 判断方向
    if (Fre > 0)
    {
        new_dir = true;
        freq    = (uint32_t)Fre;
    }else
    {
        new_dir = false;
        freq    = (uint32_t)(-Fre);
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
    if (arr > 65535)
        arr = 65535;
    if (arr == 0)
        arr = 1;

    __HAL_TIM_SET_AUTORELOAD(htim_, (uint16_t)(arr - 1));
    __HAL_TIM_SET_COMPARE(htim_, channel_, (uint16_t)(arr / 2));
}







// ── M1 的独立定时器周期结束回调 ──
static void M1_Tim_Callback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == M1_STEP_TIM) // 判断是否是当前定时器中断
    {
        Motor_M1.Step_Handler();
    }
}

// ── M2 的独立定时器周期结束回调 ──
static void M2_Tim_Callback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == M2_STEP_TIM) // 判断是否是当前定时器中断
    {
        Motor_M2.Step_Handler();
    }
}
void Step_Motor::Motor_Init()
{
    uint32_t sysclk = HAL_RCC_GetSysClockFreq();  // 返回 64000000 ???
    timer_clock_freq_ = sysclk / (htim_->Instance->PSC + 1);  //获取定时器时钟频率
    Set_Motor_Frequency(motor_frequency); //默认1000Hz频率
    __HAL_TIM_SET_COMPARE(htim_, channel_, 0);//默认占空比为0
    Set_Motor_EN(true);//使能电机
    HAL_TIM_Base_Start_IT(htim_); //启动定时器周期中断 
    // 根据实例绑定不同的定时器周期结束回调 
    if (this == &Motor_M1)
        htim_->PeriodElapsedCallback = M1_Tim_Callback;
    else if (this == &Motor_M2) 
        htim_->PeriodElapsedCallback = M2_Tim_Callback;
    init_flag = true; //设置电机初始化完成标志位
}


void Step_Motor::Step_Handler(void)
{
    // static uint16_t test_cnt = 0; //上次步进的时间戳
    // if(init_flag == true)
    // {
    //     motor_step_cnt ++;                     // 每来一次中断 = 走了一步，步数+1
    //     switch (step_mode)
    //     {
    //         case 0:                           // 模式0：2400步反转一次
    //             if (motor_step_cnt >= 1000) { //极限2300
    //                 HAL_GPIO_WritePin(EN_PORT_, EN_PIN_, GPIO_PIN_SET);//拉低使能
    //                 test_cnt++;
    //                 if(test_cnt>motor_frequency) 
    //                 {
    //                     motor_step_cnt = 0;       // 清零步数，重新计数
    //                     HAL_GPIO_WritePin(EN_PORT_, EN_PIN_,GPIO_PIN_RESET);//拉低使能
                        
    //                     test_cnt=0;
    //                     // motor_direction = !motor_direction;   // 反转方向
    //                     // Set_Motor_DTR(motor_direction);       // 写DIR引脚（内部会停PWM→设DIR→重开PWM）
    //                 }
                   
    //             }
    //             break;

    //         case 1: 
    //             break;
    //         default:
    //             break;
    //     }

    // }
   
}
