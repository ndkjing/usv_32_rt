/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-06     DUAN       the first version
 */
#ifndef APPLICATIONS_GPIO_H_
#define APPLICATIONS_GPIO_H_

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"

#if 1
// 板子状态灯的引脚
#define LED1         GET_PIN(D, 3)  //1
#define LED2         GET_PIN(D, 4)  //2
#define LED3         GET_PIN(D, 5)  //3
#define BEEP         GET_PIN(D, 6)  //4
#else
// 板子状态灯的引脚
#define LED1         GET_PIN(C, 6)  //1
#define LED2         GET_PIN(C, 7)  //2
#define LED3         GET_PIN(C, 8)  //3
#define BEEP         GET_PIN(C, 9)  //4
#endif
//lora模式引脚
#define MD0          GET_PIN(C, 4)  //
#define MD1          GET_PIN(C, 5)  //

//PW模式引脚
#define PW_IN          GET_PIN(E, 0)  //上拉输入，低电平表示上电了
#define PW_OUT          GET_PIN(E, 1)  //高电平锁电

//继电器引脚
//#define relay_current1          GET_PIN(C, 0)  //
//#define relay_current2          GET_PIN(C, 2)  //
#define relay1          GET_PIN(C, 1)  //
#define relay2          GET_PIN(C, 3)  //
#define relay3          GET_PIN(B, 8)  //备用1
#define relay4          GET_PIN(B, 9)  //备用2

//电磁阀引脚
#define valve1          GET_PIN(D, 8)   //
#define valve2          GET_PIN(D, 9)   //
#define valve3          GET_PIN(D, 10)  //
#define valve4          GET_PIN(D, 11)  //
#define valve5          GET_PIN(D, 12)  //
#define valve6          GET_PIN(D, 13)  //
#define valve7          GET_PIN(D, 14)  //
#define valve8          GET_PIN(D, 15)  //

void init_gpio(void);
void control_status_led(int status_code);
void draw(int bottle_id);

#endif /* APPLICATIONS_GPIO_H_ */
