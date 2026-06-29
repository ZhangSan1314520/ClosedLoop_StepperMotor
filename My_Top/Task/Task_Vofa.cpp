
#include "FreeRTOS.h"
#include "task.h"
#include "My_Vofa.hpp"
#include "MC_Serial.hpp"
#include "StepperMotor_Loop.hpp"

struct Vafa_data
{ // 解析后的参数值
    uint8_t len;
    float val[VOFA_PARAM_MAX];
    float last_val[VOFA_PARAM_MAX]; // 记录上次的值
    bool changed[VOFA_PARAM_MAX];   // 标记哪个变了
};

/* 同步 last_val = val，用于初始化或写入后 */
void vofa_sync_last(Vafa_data *d)
{
    for (uint8_t i = 0; i < d->len; i++)
    {
        d->last_val[i] = d->val[i];
        d->changed[i] = false;
    }
}

const char *IA_Names[] = {"iq_p", "iq_i", "id_p", "id_i"};
const char *Speed_Names[] = {"speed_p", "speed_i", "speed_d"};
const char *Target_Names[] = {"Target_Iq", "Target_Id", "Target_speed", "Target_location1", "Target_location2"};
const char *Location_Names[] = {"location_p", "location_i", "location_d"};
const char *Button_Names[] = {"motor_selection", "motor_fre", "motor_MS", "step_max"}; // 电机选择12 / 电机方向 / 电机频率/最大动的步数
const char *CLI_Names[] = {"motor_ms"};

Vafa_data Vafa_IA;
Vafa_data Vafa_Speed;
Vafa_data Vafa_Target;
Vafa_data Vafa_Location;
Vafa_data Vafa_CLI;
Vafa_data Vafa_Button;

void vofa_data_init()
{
    Vafa_IA.len = sizeof(IA_Names) / sizeof(IA_Names[0]);
    Vafa_Speed.len = sizeof(Speed_Names) / sizeof(IA_Names[0]);
    Vafa_Target.len = sizeof(Target_Names) / sizeof(Target_Names[0]);
    Vafa_Location.len = sizeof(Location_Names) / sizeof(Location_Names[0]);
    Vafa_CLI.len = sizeof(CLI_Names) / sizeof(CLI_Names[0]);
    Vafa_Button.len = sizeof(Button_Names) / sizeof(Button_Names[0]);
    // 第一次读取后同步 last_val，避免误判 */
    vofa1.vofa_get_batch(&vofa1, IA_Names, Vafa_IA.val, Vafa_IA.len); // 读到的都是0
    vofa1.vofa_get_batch(&vofa1, Speed_Names, Vafa_Speed.val, Vafa_Speed.len);
    vofa1.vofa_get_batch(&vofa1, Target_Names, Vafa_Target.val, Vafa_Target.len);
    vofa1.vofa_get_batch(&vofa1, Location_Names, Vafa_Location.val, Vafa_Location.len);
    vofa1.vofa_get_batch(&vofa1, CLI_Names, Vafa_CLI.val, Vafa_CLI.len);
    vofa1.vofa_get_batch(&vofa1, Button_Names, Vafa_Button.val, Vafa_Button.len);

    vofa_sync_last(&Vafa_IA);
    vofa_sync_last(&Vafa_Speed);
    vofa_sync_last(&Vafa_Target);
    vofa_sync_last(&Vafa_Location);
    vofa_sync_last(&Vafa_CLI);
    vofa_sync_last(&Vafa_Button);
}

/* 模式切换专用函数 */
void vofa_update_mode(Work_Mode *target, Work_Mode default_val ,uint8_t res_pid) // 只在值变了的时候，写入目标到target
{
    static Work_Mode last_mode = default_val; // 默认模式 位置模式
    float val = vofa1.get("work_mode", (float)default_val);
    Work_Mode new_mode = (Work_Mode)((int)val);

    if (new_mode != last_mode)
    {
        // if(new_mode>=0 && new_mode<=5) M1.Start_FOC_Motor(FOC_Motor_HTIM);//
        // else M1.Stop_FOC_Motor(FOC_Motor_HTIM);
        

        *target = new_mode;
        last_mode = new_mode;
        switch (res_pid) // 是否复位PID
        {
        case 0:
            
            break;
        case 1:
            M1.motor_fre = 0; //切模式时先停电机
            M1.pid_speed.reset();
            M1.pid_location.reset();
            break;
        case 2:
            M2.motor_fre = 0; //切模式时先停电机
            M2.pid_speed.reset();
            M2.pid_location.reset();
            break;
        }


    }
}

// void vofa_motor_fv(int16_t *tar_fv, int16_t default_val)
// {
//     static int16_t last_fv = default_val; //默认0
//     float val = vofa1.get("motor_fre", (float)default_val);
//     int16_t new_fv = (int16_t)val;

//     if (new_fv != last_fv) {
//         *tar_fv = new_fv;
//         last_fv = new_fv;
//     }
// }


void vofa_update_if_changed(Vafa_data *d, uint8_t idx, float *target, uint8_t res_pid)
{
    if (!d->changed[idx]) return;// 只在值变了的时候

    *target = d->val[idx];      // 写入目标

    bool dir_reversed = (d->last_val[idx] * d->val[idx] < 0); // 检测正负号是否反转

    d->last_val[idx] = d->val[idx]; // 更新last
    d->changed[idx] = false; // 标记无变化

    // 决定选哪个电机和是否全清
    StepperMotor *m = nullptr;
    bool force_reset = false;
    switch (res_pid)
    {
    case 1: m = &M1; force_reset = true;  break;  // PID参数：永远清
    case 2: m = &M2; force_reset = true;  break;
    case 3: m = &M1; break;                        // 目标值：只有反方向才清
    case 4: m = &M2; break;
    default: return;
    }

    if (force_reset || dir_reversed)
    {
        m->motor_fre = 0;
        m->motor_fre_applied_ = 0;
        m->motor_fre_last_ = 0;
        m->pid_speed.reset();
        m->pid_location.reset();
    }
}




void vofa_detect_changes(Vafa_data *d) // 检测数据的变化 写入标志位到changed
{
    for (uint8_t i = 0; i < d->len; i++)
    {
        d->changed[i] = (d->val[i] != d->last_val[i]);
    }
}

void Task_VofaRx(void *argument)
{ // 任务
    vofa_data_init();
    
    while (1)
    {
        vofa1.parse();                                                    // 解析收到的帧
        vofa1.vofa_get_batch(&vofa1, IA_Names, Vafa_IA.val, Vafa_IA.len); // 读取IA_Names参数值
        // printf("iq_p:%.2f iq_i:%.2f id_p:%.2f id_i:%.2f\r\n",Vafa_IA.val[0], Vafa_IA.val[1], Vafa_IA.val[2], Vafa_IA.val[3]);

        vofa1.vofa_get_batch(&vofa1, Speed_Names, Vafa_Speed.val, Vafa_Speed.len); // 读取Speed_Names参数值
        // printf("speed_p:%.2f speed_i:%.2f speed_d:%.2f\r\n",Vafa_Speed.val[0], Vafa_Speed.val[1], Vafa_Speed.val[2]);

        vofa1.vofa_get_batch(&vofa1, Target_Names, Vafa_Target.val, Vafa_Target.len); // 读取Target_Names参数值
        // printf("Target_Iq:%.2f Target_Id:%.2f Target_speed:%.2f Target_location:%.2f\r\n",Vafa_Target.val[0], Vafa_Target.val[1], Vafa_Target.val[2], Vafa_Target.val[3]);

        vofa1.vofa_get_batch(&vofa1, Location_Names, Vafa_Location.val, Vafa_Location.len); // 读取Location_Names参数值
        vofa1.vofa_get_batch(&vofa1, CLI_Names, Vafa_CLI.val, Vafa_CLI.len);                // 读取CLI_Names参数值
        vofa1.vofa_get_batch(&vofa1, Button_Names, Vafa_Button.val, Vafa_Button.len);       // 读取Button_Names参数值

        vofa_detect_changes(&Vafa_IA); // 检测数据的变化 写入标志位到changed
        vofa_detect_changes(&Vafa_Speed);
        vofa_detect_changes(&Vafa_Target);
        vofa_detect_changes(&Vafa_Location);
        vofa_detect_changes(&Vafa_CLI);
        vofa_detect_changes(&Vafa_Button);

        /* 只在值变了才写入 FOC 参数 */
        // vofa_update_if_changed(&Vafa_IA,     0, &pid_iq._kp,  true);
        // vofa_update_if_changed(&Vafa_IA,     1, &pid_iq._ki,  true);
        // vofa_update_if_changed(&Vafa_IA,     2, &pid_id._kp,  true);
        // vofa_update_if_changed(&Vafa_IA,     3, &pid_id._ki,  true);

        // vofa_update_if_changed(&Vafa_Target, 0, &foc1._target_Iq,  false);
        // vofa_update_if_changed(&Vafa_Target, 1, &foc1._target_Id,  false);
        // vofa_update_if_changed(&Vafa_Target, 2, &foc1._target_speed,  false);//目标速度
        // vofa_update_if_changed(&Vafa_Target, 3, &foc1._target_location1,  false);//目标位置0-2pi
        // vofa_update_if_changed(&Vafa_Target, 4, &foc1._target_location2,  false); //目标位置0-360

        // vofa_update_mode(&foc1.work_mode,None_Mode); //未按 返回默认模式

        static float Motor_M1TOM2 = 0.0;
        vofa_update_if_changed(&Vafa_Button, 0, &Motor_M1TOM2, 0);

        if ((uint16_t)Motor_M1TOM2 == 0) // 电机1
        {
            if(M1.work_mode == Normal_Mode) vofa_update_if_changed(&Vafa_Button, 1, &M1.motor_fre, 0);
            vofa_update_if_changed(&Vafa_Button, 2, &M1.MS_FULL, 0);
            vofa_update_if_changed(&Vafa_Button, 3, &M1.motor_step_max, 0);
            vofa_update_if_changed(&Vafa_Speed, 0, &M1.pid_speed._kp, 1);//速度PID
            vofa_update_if_changed(&Vafa_Speed, 1, &M1.pid_speed._ki, 1);
            vofa_update_if_changed(&Vafa_Speed, 2, &M1.pid_speed._kd, 1);
            vofa_update_if_changed(&Vafa_Location,  0, &M1.pid_location._kp,  1);//位置PID
            vofa_update_if_changed(&Vafa_Location,  1, &M1.pid_location._ki,  1);
            vofa_update_if_changed(&Vafa_Location,  2, &M1.pid_location._kd,  1);
            vofa_update_if_changed(&Vafa_Target, 2, &M1._target_speed,  3);//目标速度
            vofa_update_if_changed(&Vafa_Target, 4, &M1._target_location2,3);//目标位置0-2pi
            vofa_update_mode(&M1.work_mode,Normal_Mode,1); //未按 返回默认模式
        }
        else // 电机2
        {
            if(M2.work_mode == Normal_Mode) vofa_update_if_changed(&Vafa_Button, 1, &M2.motor_fre, 0);
            vofa_update_if_changed(&Vafa_Button, 2, &M2.MS_FULL, 0);
            vofa_update_if_changed(&Vafa_Button, 3, &M2.motor_step_max, 0);
            vofa_update_if_changed(&Vafa_Speed, 0, &M2.pid_speed._kp, 2);//速度PID
            vofa_update_if_changed(&Vafa_Speed, 1, &M2.pid_speed._ki, 2);
            vofa_update_if_changed(&Vafa_Speed, 2, &M2.pid_speed._kd, 2);
            vofa_update_if_changed(&Vafa_Location,0, &M2.pid_location._kp,2);//位置PID
            vofa_update_if_changed(&Vafa_Location,1, &M2.pid_location._ki,2);
            vofa_update_if_changed(&Vafa_Location,2, &M2.pid_location._kd,2);
            vofa_update_if_changed(&Vafa_Target, 2,&M2._target_speed,4);//目标速度
            vofa_update_if_changed(&Vafa_Target, 4,&M2._target_location2,4);//目标位置0-2pi
            vofa_update_mode(&M2.work_mode,Normal_Mode,2); //未按 返回默认模式
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
