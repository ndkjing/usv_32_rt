/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-06     DUAN       the first version
 */

#include "gear_pwm.h"
#include "manager.h"

#define gear_dev_NAME        "pwm5"  /* PWM设备名称 */
#define gear_dev_CHANNEL1   1
#define gear_dev_CHANNEL2   2
//#define gear_dev_CHANNEL3   3
//#define gear_dev_CHANNEL4   4
struct rt_device_pwm      *gear_dev;

rt_uint32_t period5= 20000000; /* 周 期 为20ms/50hz， 单 位 为 纳 秒ns */
rt_uint32_t pulse5=  1500000; /* PWM脉 冲 宽 度 值， 单 位 为 纳秒ns   1000000 */
rt_uint32_t current_gear_pwm =1500;
rt_uint32_t target_gear_pwm =1500;
static rt_thread_t gear_pwm = RT_NULL;


void init_gear_pwm()
{
     /* 查 找 设 备 */

     gear_dev = (struct rt_device_pwm *)rt_device_find(gear_dev_NAME);
     if (gear_dev == RT_NULL)
     {
         rt_kprintf("pwm sample run failed! can't find pwm5!\n");
     }
     rt_kprintf("pwm sample run ! find pwm5!\n");
     /* 使 能 设 备 */
         rt_pwm_enable(gear_dev, gear_dev_CHANNEL1);
         rt_pwm_enable(gear_dev, gear_dev_CHANNEL2);
     /* 设 置PWM周 期 和 脉 冲 宽 度 默 认 值 */
      rt_pwm_set(gear_dev, gear_dev_CHANNEL1, period5, pulse5);
      rt_pwm_set(gear_dev, gear_dev_CHANNEL2, period5, pulse5);
}

// 循环一直修改pwm值让当前输出等于目标值
void loop_change_gear(void *parameter)
{
    rt_uint32_t change_pwm_ceil = 1;
    while (1)
    {
        if((current_gear_pwm - target_gear_pwm != 0))
        {
            if (current_gear_pwm!=target_gear_pwm)
            {
                if (current_gear_pwm>target_gear_pwm)
                {
                    current_gear_pwm -= change_pwm_ceil;
                }
                else
                {
                    current_gear_pwm += change_pwm_ceil;
                }
                rt_pwm_set(gear_dev, gear_dev_CHANNEL2, period5, current_gear_pwm*1000);  /* 设 置PWM周 期 和 脉 冲 宽 度 */
            }
        }
        rt_thread_mdelay(6);
    }
}

void set_gear_pwm(u_int gear_pwm)
{
    target_gear_pwm=gear_pwm;
}

// 循环一直修改pwm值让当前输出等于目标值
int thread_gear_pwm()
{
    rt_err_t ret = RT_EOK;
    init_gear_pwm();
    /* 创建 serial 线程 */
    gear_pwm = rt_thread_create("gear_pwm",
                             loop_change_gear,
                             RT_NULL,
                             1024,
                             7,
                             10);
   /* 创建成功则启动线程 */
   if (gear_pwm != RT_NULL)
   {
       rt_thread_startup(gear_pwm);
   }
   else
   {
       ret = RT_ERROR;
   }
   return ret;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(loop_change_gear, pwm sample);
