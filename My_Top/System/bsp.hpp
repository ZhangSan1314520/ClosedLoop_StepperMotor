// #include "bsp.hpp"


#pragma once

#include "usart.h"
#include "tim.h"
#include "gpio.h"
#include "i2c.h"  
#include "spi.h"  
#include "adc.h"  


#define Stepper_VELOCITY_LOOP_FREQ_DIV 8  //速度环 
#define Stepper_POSITION_LOOP_FREQ_DIV 8 //位置环
#define Stepper_MAIN_LOOP_FREQ_HZ 16000.0
#define Stepper_VELOCITY_LOOP_FREQ_HZ (Stepper_MAIN_LOOP_FREQ_HZ / Stepper_VELOCITY_LOOP_FREQ_DIV)
#define Stepper_POSITION_LOOP_FREQ_HZ (Stepper_MAIN_LOOP_FREQ_HZ / Stepper_POSITION_LOOP_FREQ_DIV)

#define Speed_FV_MAX 2900.0f //
#define Position_FV_MAX 2000.0f //最大频率

#define M1_STEP_htim  &htim1  //M1电机 脉冲信号PWM定时器
#define M1_STEP_TIM  TIM1  //M1电机 脉冲信号PWM定时器
#define M1_STEP_CHANNEL  TIM_CHANNEL_1   //M1电机 脉冲信号PWM定时器通道
#define M1_STEP_CCER  TIM_CCER_CC1E 

#define M2_STEP_htim  &htim2  //M2电机 脉冲信号PWM定时器
#define M2_STEP_TIM  TIM2  //M2电机 脉冲信号PWM定时器
#define M2_STEP_CHANNEL  TIM_CHANNEL_2   //M2电机 脉冲信号PWM定时器通道
#define M2_STEP_CCER  TIM_CCER_CC2E  


// #define M2_STEP_htim  &htim1  //M2电机 脉冲信号PWM定时器
// #define M2_STEP_TIM  TIM1  //M2电机 脉冲信号PWM定时器
// #define M2_STEP_CHANNEL  TIM_CHANNEL_2   //M2电机 脉冲信号PWM定时器通道
// #define M2_STEP_CCER  TIM_CCER_CC2E  




#define System_htim &htim15    //计时定时器

#define Vofa_huart &huart2    //Vofa串口
#define CLI_huart &huart3     //CLI串口和打印输出

// ==================== 电机1 ====================
#define M1_DIR_PORT     M1_DIR_GPIO_Port //电机方向脚
#define M1_DIR_PIN      M1_DIR_Pin       

#define M1_EN_PORT      M1_EN_GPIO_Port  //电机使能脚
#define M1_EN_PIN       M1_EN_Pin

#define M1_MS1_PORT     M1_MS1_GPIO_Port //电机微步选择1 
#define M1_MS1_PIN      M1_MS1_Pin

#define M1_MS2_PORT     M1_MS1_GPIO_Port //电机微步选择2 （硬件接地）
#define M1_MS2_PIN      M1_MS1_Pin

#define ENC1_CAL_PORT     ENC1_CAL_GPIO_Port //编码器校准脚
#define ENC1_CAL_PIN      ENC1_CAL_Pin

#define SP1_CS_PORT     SP1_CS_GPIO_Port //编码器SPI片选脚
#define SP1_CS_PIN      SP1_CS_Pin

#define Arr_Min 33 //极限3k的时候ARR的最小值


// ==================== 电机2 ====================
#define M2_DIR_PORT     M2_DIR_GPIO_Port
#define M2_DIR_PIN      M2_DIR_Pin

#define M2_EN_PORT      M2_EN_GPIO_Port
#define M2_EN_PIN       M2_EN_Pin

#define M2_MS1_PORT     M2_MS1_GPIO_Port
#define M2_MS1_PIN      M2_MS1_Pin

#define M2_MS2_PORT     M2_MS1_GPIO_Port
#define M2_MS2_PIN      M2_MS1_Pin

#define ENC2_CAL_PORT     ENC2_CAL_GPIO_Port
#define ENC2_CAL_PIN      ENC2_CAL_Pin

#define SP2_CS_PORT     SPI2_CS_GPIO_Port
#define SP2_CS_PIN      SPI2_CS_Pin

// ==================== 外设句柄 ====================

#define EEPROM_hi2c       &hi2c2      // BL24C16F EEPROM I2C句柄
#define KTH7111_hspi1     &hspi1      // KTH7111编码器 SPI句柄
#define KTH7111_hspi2     &hspi2      // KTH7111编码器 SPI句柄
#define TIM3_htim         &htim3    //定时器3 闭环控制定时器
#define VM_hadc           &hadc1    //电压检测ADC句柄