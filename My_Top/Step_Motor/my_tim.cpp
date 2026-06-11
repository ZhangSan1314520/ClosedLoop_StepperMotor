#include "my_tim.hpp"



/**
 * @brief 定时器回调函数，用于执行FOC（磁场定向控制）相关任务
 * @param htim 定时器句柄指针，包含定时器相关信息
 */
void My_Tim_Callback(TIM_HandleTypeDef *htim)
{
    static uint16_t count = 0;  // 静态计数器，用于周期性任务调度
    static uint32_t start_time = 0, end_time = 0; // 计时变量，用于性能测量（已注释）
    if (M1.control_init_flag == false || M2.control_init_flag == false) return; // 初始化未完成时，不执行
    // if ((htim->Instance->CR1 & 0x10) == 0x10U) // 向下计数时，请求编码器位置信息
    if ((htim->Instance->CR1 & 0x10) == 0x00U) // 向上计数时，请求编码器位置信息
    {
        if (count % 4 == 1)// 每4个周期执行一次（4k频率）
        {
            // start_time = DWT->CYCCNT; //记录开始时间 每 1 个 CPU 周期自增 1
            // foc1.Update_Dian_Theta_PLL(&voltage_motor1);//更新速度角度 耗时31us
            M1.Update_Speed_Angle_LPFAndPLL();//更新速度角度 耗时24us 一起37us
            
            // end_time = DWT->CYCCNT; // 记录结束时间
            // foc1.run_time_us = (float)(end_time - start_time) / 168.0f; // 计算运行时间（单位：微秒）

        }
        if (count % Stepper_VELOCITY_LOOP_FREQ_DIV == 1 && M1.work_mode == speed)                        // 每4个周期执行一次（4k频率速度环）
        {
            // start_time = DWT->CYCCNT; //记录开始时间
            M1.Speed_Loop();//速度环控制4k 耗时8.2us 
            // end_time = DWT->CYCCNT; // 记录结束时间
            // M1.run_time_us = (float)(end_time - start_time) / 168.0f; // 计算运行时间（单位：微秒）
        }
        if (count % Stepper_POSITION_LOOP_FREQ_DIV == 1 && M1.work_mode == position)                        // 每16个周期执行一次（1k频率位置环） 
        {
            // start_time = DWT->CYCCNT; //记录开始时间
            M1.Position_Loop();//位置环控制4k  耗时14.33us
            // end_time = DWT->CYCCNT; // 记录结束时间
            // M1.run_time_us = (float)(end_time - start_time) / 168.0f; // 计算运行时间（单位：微秒）
        }
        count++;
    }
}


void My_Tim_OC_Callback(TIM_HandleTypeDef *htim) // TIM1_CH5 的输出比较中断
{
    static uint16_t count = 0;
    static uint32_t start_time = 0, end_time = 0; // 计时变量
    if (M1.control_init_flag == false || M2.control_init_flag == false) return; // 初始化未完成时，不执行


    
    count++;
}

/**
 * @brief 初始化定时器并设置回调函数
 * @param htim 定时器句柄指针，指向定时器配置结构体
 */

 uint16_t ADC_Value[100]; // ADC采集值数组，长度为2
void My_Tim_Init(TIM_HandleTypeDef* htim,ADC_HandleTypeDef *hadc)
{
    HAL_ADC_Start_DMA(hadc,(uint32_t *)ADC_Value,100); //电池电压采集
    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (uint32_t)(100 * 0.5f)); // 设置占空比为50% 控制My_Tim_OC_Callback回调函数的触发时机
    HAL_TIM_Base_Start_IT(htim); //启动定时器周期中断
    HAL_TIM_OC_Start_IT(htim, TIM_CHANNEL_1); //启动定时器比较中断
    htim->PeriodElapsedCallback = My_Tim_Callback;  // 设置定时器周期结束回调函数
    htim->PWM_PulseFinishedCallback = My_Tim_OC_Callback;// PWM的模式的比较匹配回调函数
}





