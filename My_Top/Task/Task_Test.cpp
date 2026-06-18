
#include "FreeRTOS.h"
#include "task.h"
#include "MC_Serial.hpp"
#include "My_Vofa.hpp"

#include "StepperMotor_Loop.hpp"

BL24C16F EEPROM1(EEPROM_hi2c);

uint8_t test1,test2 = 0;
uint8_t bufff_tx[10] = {0x01, 0x02, 0xa3, 0xb4, 0x05, 0x06, 0xc7, 0xf8, 0x09, 0x0a};
uint8_t bufff_rx[10];



void lajio()
{
    uint16_t x = 60;
    uint16_t y = 60;
    HAL_TIM_PWM_Start(M1_STEP_htim, M1_STEP_CHANNEL);
    HAL_TIM_PWM_Start(M2_STEP_htim, M2_STEP_CHANNEL);
    HAL_GPIO_WritePin(M1_EN_PORT,M1_EN_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M2_EN_PORT,M2_EN_PIN,GPIO_PIN_RESET);
    __HAL_TIM_SET_AUTORELOAD(M1_STEP_htim,x);//设置周期
    __HAL_TIM_SET_AUTORELOAD(M2_STEP_htim, y);//设置周期
    __HAL_TIM_SET_COMPARE(M1_STEP_htim, M1_STEP_CHANNEL, x/2);//设置占空比
    __HAL_TIM_SET_COMPARE(M2_STEP_htim, M2_STEP_CHANNEL, y/2);//设置占空比
}


void Task_Test(void *argument)
{
    // 程序RAM占用10752/1024 = 10.5KB的RAM，剩余RAM 36KB-10.5KB  = 25.5KB 可分配给操作系统FreeRTOS

    // lajio();
    while (1)
    {
        // printf("laji1:%d laji2:%d \r\n",M1.laji,M2.laji);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}




