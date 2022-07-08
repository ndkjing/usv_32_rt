/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-06     RT-Thread    first version
 */

#include <rtdbg.h>
#include <rtthread.h>
#include "gpio.h"
#include "compass.h"
#include "gps.h"
#include "energy_adc.h"
#include "raspberry.h"
#include "manager.h"
#include "lora.h"
#include "motor_pwm.h"
#include "gear_pwm.h"
#include "water.h"
/*
 * M1,M2-PA6,PA7
 * gear -PA0,PA1
 * lora -9600,9.6k,add:24,03
 */
int main(void)
{
    init_gpio();//gpio初始化
    thread_energy();
    thread_water();
    thread_raspberry();
    thread_compass();
    thread_gps();
    thread_lora();
    thread_manager();
    thread_pwm();
    thread_gear_pwm();
    wdt_sample();
    return RT_EOK;
}
