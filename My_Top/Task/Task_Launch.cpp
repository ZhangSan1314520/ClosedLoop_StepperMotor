
/* FreeRTOS includes */
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Task Handle */
TaskHandle_t SystemInitHandle;
TaskHandle_t TestHandle;
TaskHandle_t Task_CLIHandle;
TaskHandle_t Task_VofaRxHandle;

/* Task Function */
extern void Task_SystemInit(void *argument);
extern void Task_Test(void *argument);
extern void Task_CLI(void *argument);
extern void Task_VofaRx(void *argument);

void MC_TaskStart(void)
{
    /* Task 创建 */
    xTaskCreate(Task_SystemInit, "SystemInit", 256, NULL, osPriorityHigh, &SystemInitHandle); //系统初始化任务
    xTaskCreate(Task_Test, "Test", 256, NULL, osPriorityNormal, &TestHandle); //测试任务
    xTaskCreate(Task_VofaRx, "Task_VofaRx", 256, NULL, osPriorityNormal, &Task_VofaRxHandle); //Vofa解析任务


}


extern "C" void MC_ScheduleStart(void)
{
    MC_TaskStart();//开始任务
}


