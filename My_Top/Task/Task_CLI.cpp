#include "My_CLI.hpp"

CLI_Module cli_console;  // 全局CLI实例

// ============================================================
// 1. 命令处理函数声明
// ============================================================

// ============================================================
// 2. CLI 任务入口
// ============================================================
void Task_CLI(void *argument)
{
    cli_console.init("FOC_ZGJ");                       // CLI终端名称
    cli_console.bind_write_data(&sr_console);       // 绑定串口1

    // 注册命令
    CLI_Module::add_command_to_all("start", "start", true, NULL, start);//电机启动
    CLI_Module::add_command_to_all("stop", "stop", true, NULL, stop);//电机停止
    CLI_Module::add_command_to_all("reset", "reset", true, NULL, reset);//驱动板重启

    CLI_Module::add_command_to_all("set_dir", "set_dir [dir] 0-顺时针 1-逆时针", true, NULL, set_dir);//设置电机正反转方向
    CLI_Module::add_command_to_all("set_fre", "set_fre", true, NULL, set_fre);//设置电机频率
    CLI_Module::add_command_to_all("set_duty", "set_duty [duty] 范围0.0f-1.0f", true, NULL, set_duty);//设置电机占空比

    CLI_Module::start_all();  // 启动所有CLI

    vTaskDelete(NULL);
}

