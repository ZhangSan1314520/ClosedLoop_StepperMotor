#pragma once

#include <stdbool.h>
#include "spi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.hpp"

#define KTH7111_PRECISION (65535.0f) // 2^16-1，编码器分辨率
#define RAW_TO_RAD  (6.28318530717f / KTH7111_PRECISION)// 编码器原始值转弧度系数：2π / (编码器满量程)
#define RAD_TO_DEG  (180.0f / 3.14159265358979323846f)// 弧度转角度系数：180 / π ≈ 57.29578
#include "ex_math.hpp" 


class KTH7111
{
public:
    uint16_t single_turn; // 单圈原始值16位
    float theta;          // 转换后的弧度角 (0 ~ 2π)
    float angle;          // 转换后的角度 (0 ~ 360)
    bool  encoder_dir; // 编码器方向
    KTH7111(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
    : _hspi(hspi), _cs_port(cs_port), _cs_pin(cs_pin),
      single_turn(0), theta(0.0f), angle(0.0f) {}

    bool KTH7111_Init(bool fx); // 初始化编码器参数，设置编码器旋转方向
    void KTH_CS_Enble(bool status); // 控制片选，true拉高，false拉低
    void SPI_Set_MOSI_Output();
    void SPI_Set_MOSI_Input();
    uint8_t CRC8_ITU(const uint8_t *data, uint8_t len); // CRC校验
    void KTH7111_Send_JIAO_CMD(); //发送指令读取角度
    uint8_t KTH7111_ReadReg(uint8_t addr); //读取寄存器
    bool KTH7111_WriteReg(uint8_t addr, uint8_t data); //写入寄存器
    bool KTH7111_RotationDirection(bool RD) ; //控制编码器输出角度方向
    bool KTH7111_SetNowZero(void); //设置当前角度为编码器零点
    float Get_KTH7111_Radian(); //读取弧度 0-2π
    float Get_KTH7111_Degree(); //读取角度 0-360
    HAL_StatusTypeDef KTH7111_BurnMTP(void);//保存当前参数到MTP
    void KTH7111_Read_MoreRegs(uint8_t *add, uint8_t *data_rx, uint8_t len); //读取多个寄存器
    void KTH7111_Write_MoreRegs(uint8_t *add, uint8_t *data_tx, uint8_t len,bool save); //写入多个寄存器
    bool KTH7111_ANLC_Calibration(uint32_t timeout_ms); //自动线性度校准（方式1：匀速旋转触发）
    uint8_t KTH7111_Get_ANLC_Status(void); //读取校准状态 0=未完成 1=完成 2=失败
    void SetZeroAngle_RuanJian(float zero); //设置软件零点

private:
    SPI_HandleTypeDef *_hspi;
    GPIO_TypeDef *_cs_port;
    uint16_t    _cs_pin;   

};



int KTH7111_ReadReg_tp(uint8_t addr, uint8_t *value_out);
