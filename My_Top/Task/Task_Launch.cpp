
/* FreeRTOS includes */
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Task Handle */
TaskHandle_t SystemInitHandle;
TaskHandle_t TestHandle;
TaskHandle_t Task_CLIHandle;

/* Task Function */
extern void Task_SystemInit(void *argument);
extern void Task_Test(void *argument);
extern void Task_CLI(void *argument);


void MC_TaskStart(void)
{
    /* Task 创建 */
    xTaskCreate(Task_SystemInit, "SystemInit", 512, NULL, osPriorityHigh, &SystemInitHandle); //系统初始化任务
    xTaskCreate(Task_Test, "Test", 512, NULL, osPriorityNormal, &TestHandle); //测试任务
    xTaskCreate(Task_CLI, "Task_CLI", 512, NULL, osPriorityNormal, &Task_CLIHandle); //测试任务
}


extern "C" void MC_ScheduleStart(void)
{
    MC_TaskStart();//开始任务
}


