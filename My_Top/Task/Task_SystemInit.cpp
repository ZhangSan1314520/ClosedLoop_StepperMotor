
#include "FreeRTOS.h"
#include "task.h"
#include "ex_math.hpp"
#include "HAL_System.hpp"
#include "MC_Serial.hpp"
#include "bsp.hpp"
#include "Step.hpp"
#include "StepperMotor_Loop.hpp"
#include "my_tim.hpp"

void Task_SystemInit(void *argument)
{
    HAL_System::init(); //初始化滴答计时器
    MC_Serial::init(); //初始化串口 
    My_Tim_Init(TIM2_htim,VM_hadc); //初始化定时器
    Motor_M1.Motor_Init(); //初始化步进电机M1
    Motor_M2.Motor_Init(); //初始化步进电机M2

    M1.My_Stepper_Init(); //初始化步进电机控制参数
    M2.My_Stepper_Init(); //初始化步进电机控制参数

    vTaskDelete(NULL); //删除任务
}



