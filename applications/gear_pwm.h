/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-06     DUAN       the first version
 */
#ifndef APPLICATIONS_GEAR_PWM_H_
#define APPLICATIONS_GEAR_PWM_H_


#include <rtthread.h>

void set_gear_pwm(u_int gear_pwm);
int thread_gear_pwm(void);
extern rt_uint32_t current_gear_pwm;

#endif /* APPLICATIONS_GEAR_PWM_H_ */
