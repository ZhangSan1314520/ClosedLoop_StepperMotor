#include "My_CLI.hpp"

void start(EmbeddedCli *cli, char *args, void *context) //启动电机
{
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;

    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);

    // Motor_M1.Set_Motor_EN(1); //启动电机
    printf("启动电机\r\n");

}

void stop(EmbeddedCli *cli, char *args, void *context) //停止电机
{
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;

    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    Motor_M1.Set_Motor_EN(0); //停止电机
    printf("停止电机\r\n");
}


void reset(EmbeddedCli *cli, char *args, void *context) //驱动板重启
{
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;

    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);

    HAL_NVIC_SystemReset();//复位芯片
    printf("驱动板重启\r\n");

}

void set_dir(EmbeddedCli *cli, char *args, void *context) //设置电机正反转方向
{
    char *end;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    if (argc <= 0) { //argc是除了定义头后 字符串的个数
        printf("参数个数错误\r\n");
        return;
    }
    uint8_t temp = strtof(arg_list[1], &end); //将字符串转换为浮点数
    if(end == arg_list[1] || (temp != 0 && temp != 1)) 
    {
        printf("参数格式错误\r\n");
        return;
    }
    Motor_M1.Set_Motor_DTR(temp); //设置电机正反转方向
    printf("电机正反转方向设置%d\r\n",temp);
}


void set_fre(EmbeddedCli *cli, char *args, void *context) //设置电机频率 单位 Hz
{
    char *end;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    if (argc <= 0) { //argc是除了定义头后 字符串的个数
        printf("参数个数错误\r\n");
        return;
    }
    float temp = strtof(arg_list[1], &end); //将字符串转换为浮点数
    if(end == arg_list[1]) 
    {
        printf("参数格式错误\r\n");
        return;
    }
    Motor_M1.Set_Motor_Frequency(temp); //设置电机频率
    printf("电机频率设置%.2fhz\r\n",temp);
}

void set_duty(EmbeddedCli *cli, char *args, void *context) //设置电机占空比0-1
{
    char *end;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    if (argc <= 0) { //argc是除了定义头后 字符串的个数
        printf("参数个数错误\r\n");
        return;
    }
    float temp = strtof(arg_list[1], &end); //将字符串转换为浮点数
    if(end == arg_list[1]) 
    {
        printf("参数格式错误\r\n");
        return;
    }
    // Motor_M1.Set_Motor_DutyCycle(temp); //设置电机占空比
    printf("电机占空比设置%.2f\r\n",temp);
}

