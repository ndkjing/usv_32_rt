/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-28     XXL       the first version
 */
#ifndef APPLICATIONS_MANAGER_H_
#define APPLICATIONS_MANAGER_H_

#include <rtthread.h>
#include "rtconfig.h"
#include <rtdevice.h>
#include "drv_common.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
extern int ship_id;
extern int is_connect;
extern rt_tick_t rec_server_tick;
extern float_t lng;  // 经度
extern float_t lat;   // 纬度
extern int speed;
extern float_t target_lng;  // 目标经度
extern float_t target_lat;   // 目标纬度
extern float_t current_theta;//当前角
extern int is_auto;//是否处于自动巡航标志位
extern float_t dump_energy;  //  当前剩余电量
extern float_t target_lng;  // 目标经度
extern float_t target_lat;   // 目标纬度

extern int control_val;//控制值
extern int control_mode;//控制模式
extern int network_backhome;//断网返航
extern int energy_backhome;  //低电量返航
extern int obstacle_avoid;  //是否避障
extern int hand_speed; //遥控速度比例
extern int auto_speed; //自动巡航速度比例
extern float_t pid[3]; //pid参数
extern int compass_status; //罗盘  0：读取数据  1：开始校准   2：结束校准
extern u_int row;//遥控器转弯分量
extern u_int col;//遥控器直行分量
extern int key[8];//遥控器按键数组[使能,舵机，]
extern int current_mode;
extern rt_tick_t rec_tick; //接受到使能时间戳
extern int angle_error;
extern int relay;  // 水泵开启状态
extern int draw_status;
extern int bottle_id; //抽水瓶号
extern int bottle_deep; //抽水深度
extern int bottle_amount;//抽水容量
extern int full_draw_time;// 抽到需要容量需要的时间
extern int dump_draw_time;//剩余抽水时间
extern u_int max_draw_amount;
//障碍物检测数据
extern int b_obstacle_avoid;
extern int target_id[10]; // 障碍物id列表
extern int target_distance[10]; // 障碍物距离列表
extern int target_angle[10];// 障碍物角度列表
extern int min_distance; // 检测到障碍物最近距离
extern rt_tick_t obstacle_tick; //检测到障碍物时间 超时没见到清除所有障碍物
  //水质检测数据
extern int wt;  // 水温
extern int ph;  // 酸碱度
extern int doDo;  // 溶解氧
extern int ec;  // 电导率
extern int td;  //  浊度
int thread_manager(void);

#endif /* APPLICATIONS_MANAGER_H_ */
