#include "FreeRTOS.h"
#include "task.h"
#include "uorb.h"
#include "task_topics.h"
#include "bsp.hpp"
#include "My_Vofa.hpp"

void Task_UorbDemo(void *argument)
{
    uorb_handle_t handle = uorb_subscribe("motor_state");  // 订阅motor_state话题

    motor_state_t data;

    while (1)
    {
        uorb_copy(handle, &data);// ← 接收数据
        Vofa_SendFireWater(Vofa_huart, data, MOTOR_STATE_NUM, 0);//发给Vofa
    }
}
