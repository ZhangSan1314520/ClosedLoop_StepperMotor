
/* FreeRTOS includes */
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "StepperMotor_Loop.hpp"
#include "MC_Serial.hpp"


/* Task Handle */
TaskHandle_t SystemInitHandle;
TaskHandle_t TestHandle;
TaskHandle_t Task_VofaRxHandle;

/* Task Function */
extern void Task_SystemInit(void *argument);
extern void Task_Test(void *argument);
extern void Task_VofaRx(void *argument);
extern void Task_MsgQueueDemo(void *argument);
extern void Task_UorbDemo(void *argument);
BaseType_t retA,retB,retC,retD,retE;
void MC_TaskStart(void)
{
    /* Task 创建 */
    retA = xTaskCreate(Task_SystemInit, "SystemInit", 256, NULL, osPriorityHigh, &SystemInitHandle); //系统初始化任务
    retB = xTaskCreate(Task_Test, "Test", 256, NULL, osPriorityNormal, &TestHandle); //测试任务
    retC = xTaskCreate(Task_VofaRx, "Task_VofaRx", 256, NULL, osPriorityNormal, &Task_VofaRxHandle); //Vofa解析任务
    retE = xTaskCreate(Task_UorbDemo, "UorbDemo", 256, NULL, osPriorityNormal, NULL);//uORB订阅演示任务
    if(retA != pdPASS || retB != pdPASS || retC != pdPASS || retE != pdPASS){
        return;
    }
}

extern "C" void MC_ScheduleStart(void)
{
    MC_TaskStart();//开始任务
}


