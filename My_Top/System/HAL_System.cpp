#include "HAL_System.hpp"
#include "FreeRTOS.h"
#include "task.h"

// ============================================================
// 静态成员变量定义
// STM32 的 TIM 计数器是 32 位的，最大值 0xFFFFFFFF
// 当计数器溢出（从 0xFFFFFFFF 回到 0）时，需要累加高 32 位
// ============================================================
uint64_t HAL_System::tick_us = 0;     // 64 位微秒计时器（高 32 位 + TIM 计数器）
uint32_t HAL_System::tick_ms = 0;     // 32 毫秒计时器（由 tick_us 计算得到）

// ============================================================
// 初始化函数
// ============================================================
void HAL_System::init(void)
{
    // 启动定时器中断（计数器开始计数）
    HAL_TIM_Base_Start_IT(System_htim);

    // 注册 PeriodElapsed 回调函数
    // 作用：当 TIM 计数器溢出时，自动调用 period_elapsed_callback()
    // 这样可以累加高 32 位，实现 64 位精度的长时间计时stm32g4xx_hal_tim
    HAL_TIM_RegisterCallback(System_htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, period_elapsed_callback);
}


// ============================================================
// 获取当前微秒数（64 位，精度由 TIM 决定）
// ============================================================
uint64_t HAL_System::get_tick_us(void)
{
    // 读取当前 TIM 计数器的低 32 位
    uint32_t current_cnt = __HAL_TIM_GET_COUNTER(System_htim);

    // 拼装 64 位计时器：
    // tick_us 的高 32 位 = 之前的累加值（elapse_cnt × 2^32）
    // tick_us 的低 32 位 = 当前 TIM 计数器值
    // 先把当前的低 32 位清零（& ~0xFFFFFFFF），再或上新的计数器值
    tick_us = (tick_us & (uint64_t)0xFFFFFFFF00000000) | current_cnt;

    return tick_us;
}

// ============================================================
// 获取当前毫秒数（32 位，由微秒计算得出）
// ============================================================
uint32_t HAL_System::get_tick_ms(void)
{
    // 微秒转毫秒（除以 1000）
    tick_ms = get_tick_us() / 1000;
    return tick_ms;
}

// ============================================================
// 延时函数（使用 FreeRTOS 的任务延时，释放 CPU）
// ============================================================
void HAL_System::delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// ============================================================
// 定时器溢出回调函数（每当 TIM 计数器从 0xFFFFFFFF 回到 0 时触发）
// ============================================================
// 注意：此函数运行在中断上下文中！
void HAL_System::period_elapsed_callback(TIM_HandleTypeDef *htim)
{
    // TIM 溢出一次 = 增加了 2^32 个计数单位
    // 1 << 32 = 0x100000000 = 4294967296
    // tick_us 高 32 位自动加 1（因为累加到这个数）
    tick_us += (uint64_t)1 << 32;
}


