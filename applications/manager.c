/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-28     XXL       the first version
 */
#include "manager.h"
#include "gpio.h"
#include "motor_pwm.h"
#include "math.h"
#include "auto_cruise.h"
#include "gear_pwm.h"
#include "wdt_usv.h"
//全局变量
////全局变量
int ship_id=8;  // 船编号
int is_connect=0; // 船与服务器是否连接上
rt_tick_t rec_server_tick = 0; //接受服务器心跳包时间
float_t lng=0.0;  // 经度
float_t lat=0.0;   // 纬度
int speed;
float_t home_lng=0.0;  // 返航点经度
float_t home_lat=0.0;   // 返航点纬度
float_t current_theta=0;//当前角
int is_auto=0;//是否处于自动巡航标志位
float_t dump_energy = 0;  //  当前剩余电量
float_t target_lng=0.0;  // 目标经度
float_t target_lat=0.0;   // 目标纬度
int control_val=-1;//控制值
int control_mode=1;//控制模式
int network_backhome=0;//断网返航
int energy_backhome=0;  //低电量返航
int obstacle_avoid=0;  //是否避障
int hand_speed=3; //遥控速度比例
int auto_speed=3; //自动巡航速度比例
float_t pid[3]={0.5,0.01,1}; //pid参数
int compass_status=0; //罗盘  0：读取数据  1：开始校准   2：结束校准
u_int row=50;//遥控器转弯分量
u_int col=50;//遥控器直行分量
int key[8]={0};//遥控器按键数组[使能,舵机，抽水]
rt_tick_t rec_tick = 0; //接受到使能时间戳
int current_mode=2; // 当前控制电机值来源1:遥控器  2:电脑手动控制  3:自动控制
int angle_error=0.0; // 误差角度
//服务器发送抽水瓶号深度容量 深度放大十倍 容量减小十倍
int relay=0;  // 接受到服务器的水泵指令
int draw_status=0;// 船水泵状态
int bottle_id=0; //抽水瓶号
int bottle_deep=0; //抽水深度
int bottle_amount=0;//抽水容量
int full_draw_time=0;// 抽到需要容量需要的时间
int dump_draw_time=0;//剩余抽水时间
u_int max_draw_amount=1000; // 最大采样容量
//障碍物检测数据
int b_obstacle_avoid=0;
int target_id[10]={0}; // 障碍物id列表
int target_distance[10]={0}; // 障碍物距离列表
int target_angle[10]={0};// 障碍物角度列表
int min_distance=10000; // 检测到障碍物最近距离
rt_tick_t obstacle_tick = 0; //检测到障碍物时间 超时没见到清除所有障碍物
  //水质检测数据
int wt=0;  // 水温
int ph=0;  // 酸碱度
int doDo=0;  // 溶解氧
int ec=0;  // 电导率
int td=0;  //  浊度
////定义变量
u_int left_pwm  = 1500;//左电机应该输出值
u_int right_pwm = 1500;//左电机应该输出值
int remote_forward_pwm = 1500;//遥控直行输出
int remote_steer_pwm = 1500;//遥控转弯输出
int draw_speed=2000;// 抽水容量为2000ml/分钟
rt_tick_t draw_start_tick = 0; //抽水开始时间戳
int is_draw=0; //遥控器抽水超时标志位
u_int max_geer_pwm=2400;
u_int min_geer_pwm=800;
static rt_thread_t t_manager = RT_NULL;
float d2r = 3.14159/180.0;
rt_tick_t draw_start_tick_1 = 0; //测试时间戳
int deep_pwm = 800;
void manager(void *parameter)
{
    while(1)
    {
        //1.5秒没检测到障碍物将最近障碍物距离设置为默认值
        if (rt_tick_get()-obstacle_tick>1500) {
            min_distance=10000; // 检测到障碍物默认距离
        }
        // 遥控器超时释放使能
        if (rt_tick_get()-rec_tick>1500)
        {
            key[0]=0;
        }
        // 16秒收不到服务器消息将连接上服务器标志位置位0
        if (rt_tick_get()-rec_server_tick>16000)
        {
            is_connect=0;
        }
        // 状态灯颜色改变
        if (key[0]==1)
        {
            control_status_led(1); //使能遥控器黄色
        }
        else if (network_backhome==1 || energy_backhome==1) {
            control_status_led(2);// 需要返航了红灯
        }
        else if (is_connect==1) {
            control_status_led(3);// 连上网络了绿色
        }
        else {
            control_status_led(2);// 默认红灯
        }
        // 控制状态判断
        if (key[0]==1)
        {
            current_mode=1;
        }
        else if (target_lat>1 && target_lng>1 && control_mode!=4)
        {
            current_mode=3;
        }
        else {
            current_mode=2;
        }
        if (current_mode==1)
        {
            // 控制电机输出 将摇杆值0-99转换到1000-2000
            remote_forward_pwm = (100 - col) * 10 + 1000;//2000-1000
            remote_steer_pwm = row * 10 + 1000;//1000-2000
            //左高右高-电机前进
            left_pwm  = 1500 + (remote_forward_pwm - 1500) + (int)((remote_steer_pwm - 1500)/2);
            right_pwm = 1500 + (remote_forward_pwm - 1500) - (int)((remote_steer_pwm - 1500)/2);
            set_pwm(left_pwm,right_pwm);//设置目标输出PWM
        }
        else if (current_mode==2)
        {
            int speed_pwm = 100*hand_speed;
            int forward_pwm=1500;
            int steer_pwm=1500;
            if (control_val ==-1){
                forward_pwm = 0;
                steer_pwm = 0;
            }
            else
            {
                if (control_val < 90){
                    forward_pwm = (int)(speed_pwm * cos(control_val*d2r));
                    steer_pwm = -1 * (int)(speed_pwm * sin(control_val*d2r));
                }
                else if (control_val < 180){
                    forward_pwm = -1 * (int)(speed_pwm * sin((control_val - 90)*d2r));
                    steer_pwm = (int)(speed_pwm * cos((control_val - 90)*d2r));
                }
                else if (control_val < 270){
                    forward_pwm = -1 * (int)(speed_pwm * cos((control_val - 180)*d2r));
                    steer_pwm = -1 * (int)(speed_pwm * sin((control_val - 180)*d2r));
                }
                else{
                    forward_pwm = (int)(speed_pwm * sin((control_val - 270)*d2r));
                    steer_pwm = (int)(speed_pwm * cos((control_val - 270)*d2r));
                }
            }
            left_pwm = 1500 + forward_pwm + steer_pwm;
            right_pwm = 1500 + forward_pwm - steer_pwm;
            set_pwm(left_pwm,right_pwm);//设置目标输出PWM
        }
        else if (current_mode==3)
        {
            u_int left_pwm=1500;
            u_int right_pwm=1500;
            auto_cruise_pid(&left_pwm,&right_pwm);
            set_pwm(left_pwm,right_pwm);//设置目标输出PWM
        }
        // 抽水遥控器使能
        if (key[0]==1)
        {
            // 舵机被按下
            if (key[1]==1){
                set_gear_pwm(min_geer_pwm);
           }
            else {
                set_gear_pwm(max_geer_pwm);
            }
            // 抽水
            if (key[2]==1 && is_draw!=1){
                if (draw_start_tick==0)
                {
                    draw_start_tick=rt_tick_get();
                }
                // 判断是否超时
                if (rt_tick_get()-draw_start_tick < (int)(1000*60.0*max_draw_amount/draw_speed))
                {
                    draw(1);
                }
                else{
                    draw(0);
                    is_draw=1;
                    draw_start_tick=0;
                }

           }
            else {
                draw(0);
                is_draw=0;
                draw_start_tick=0;
            }
        }
        else{
            if (bottle_id!=0 && draw_status!=4)
            {
                // 计算深度对应PWM值并判断深度是否已经达到
                deep_pwm = (int)(1400-bottle_deep*(1400-min_geer_pwm)/5);
//                rt_kprintf("current_gear_pwm %d ,deep_pwm %d ",current_gear_pwm,deep_pwm);
//                rt_thread_mdelay(500); // 正式运行不需要
                if (current_gear_pwm!=deep_pwm)
                {
                    draw_status=1;
                    set_gear_pwm(deep_pwm);
                }
                else {
                    // 判断是否超时
                    if (draw_start_tick==0)
                    {
                        draw_start_tick=rt_tick_get();
                    }
                    full_draw_time =(int)(60.0*10*bottle_amount/draw_speed);
//                    rt_kprintf("full_draw_time %d dump_draw_time %d ",full_draw_time*1000,rt_tick_get()-draw_start_tick);
                    if (rt_tick_get()-draw_start_tick < full_draw_time*1000)
                    {
//                        rt_kprintf("full_draw_time %d",bottle_id);
                        dump_draw_time = full_draw_time-(rt_tick_get()-draw_start_tick)/1000;
                        draw(bottle_id);
                        draw_status=2;
                    }
                    else{
                        full_draw_time=0;
                        dump_draw_time = 0;
                        draw(0);
//                        bottle_id=0;
                        draw_start_tick=0;
                        draw_status=4;
                    }
                }
            }
            else {
               //没有抽水瓶号
                full_draw_time=0;
                dump_draw_time = 0;
                draw(0);
                draw_start_tick=0;
                set_gear_pwm(max_geer_pwm);
            }
        }
        rt_thread_mdelay(40);
        feed_dog();
    }
}

/* 线程 */
int thread_manager(void)
{
    /* 创建线程 */
    t_manager = rt_thread_create("thread_manager",
                                  manager,
                                  RT_NULL,
                                  2048,
                                  3,
                                  5);

    /* 如果获得线程控制块，启动这个线程 */
    if (t_manager != RT_NULL)
        rt_thread_startup(t_manager);
    return 0;
}





