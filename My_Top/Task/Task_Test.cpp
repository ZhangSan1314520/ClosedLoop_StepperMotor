
#include "FreeRTOS.h"
#include "task.h"
#include "MC_Serial.hpp"

#include "My_Vofa.hpp"


void Task_Test(void *argument)
{
    //程序RAM占用10752/1024 = 10.5KB的RAM，剩余RAM 36KB-10.5KB  = 25.5KB 可分配给操作系统FreeRTOS
    while (1)
    {
        // printf("hellow\r\n");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}



