
#include "FreeRTOS.h"
#include "task.h"
#include "ex_math.hpp"
#include "HAL_System.hpp"
#include "MC_Serial.hpp"
#include "bsp.hpp"
#include "Step.hpp"
#include "StepperMotor_Loop.hpp"
#include "my_tim.hpp"
#include "My_Vofa.hpp"
#include "uorb.h"

void Task_SystemInit(void *argument)
{
    uorb_init(); //初始化uORB消息中间件
    HAL_System::init(); //初始化计时器
    MC_Serial::init(); //初始化串口 
    Vofa_Init(Vofa_huart); //初始化Vofa
    My_Tim_Init(TIM3_htim,VM_hadc); //初始化定时器绑定中断

    M1.My_Stepper_Init(); //初始化步进电机控制参数
    M2.My_Stepper_Init(); //初始化步进电机控制参数

    vTaskDelete(NULL); //删除任务
}


