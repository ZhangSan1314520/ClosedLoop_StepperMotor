#pragma once  // 保证头文件只被编译一次，防止头文件被重复引用

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <cstdlib>
#include "CLI_Module.hpp"
#include "MC_Serial.hpp"
#include "Step.hpp"


void start(EmbeddedCli *cli, char *args, void *context); //启动电机
void stop(EmbeddedCli *cli, char *args, void *context); //停止电机
void reset(EmbeddedCli *cli, char *args, void *context); //复位电机

void set_dir(EmbeddedCli *cli, char *args, void *context); //设置电机正反转方向
void set_fre(EmbeddedCli *cli, char *args, void *context); //设置电机频率
void set_duty(EmbeddedCli *cli, char *args, void *context); //设置电机占空比0-1














