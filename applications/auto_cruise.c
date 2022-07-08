/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-23     DUAN       the first version
 */
#include "auto_cruise.h"
#include "manager.h"
int max_control=300;
int temp_forward_pwm=0;
int forward_pwm=0;
int steer_pwm=0;
double AD_distance=0.0;//等腰梯形计算法
double BC_distance=0.0;//BC
double AC_distance=0.0;//AC
double ALL_distance=0.0;//总距离
double radius=3.0;//航点半径
int targe_theta=0; //目标角

float Ek=0.0;               //定义偏差值
float Ekk=0.0;           //定义上一个偏差值
float Kp=0.5,Ki=0.0,Kd=0;           //定义比例、积分、微分系数
float PID_value=0.0;         //定义PID值（控制执行器的变量）
float PID_value_change=0.0;
float integral=0.0,differential=0.0;           //定义积分值
float integral_limit=1800.0; //积分上限
double radius_of_earth=6371.580;//地球半径
double pi2degree = 3.1415926/180.00;

//计算返航角及PWM输出
void auto_cruise_pid(u_int *left_pwm ,u_int* right_pwm)
{
    max_control=100*auto_speed;
    double longitude_PI=0,latitude_PI=0,tragetLO_PI=0,trageLA_PI=0;//转化成PI
    double longitude_difference=0,latitude_difference=0;//经纬度差值
    double ALL_distance_temp=0.0;//距离

    longitude_PI=lng*pi2degree;//经度pi
    latitude_PI=lat*pi2degree;//纬度pi
    tragetLO_PI=target_lng*pi2degree;//目标经度pi
    trageLA_PI=target_lat*pi2degree;//目标纬度pi
    longitude_difference=tragetLO_PI-longitude_PI;
    latitude_difference=trageLA_PI-latitude_PI;
//    AD_distance=2000*sin(longitude_difference/2)*cos(trageLA_PI)*radius_of_earth;//   AD=1000*2*sin(经度差/2)*cos(目标纬度派)*地球半径;
//    BC_distance=2000*sin(longitude_difference/2)*cos(latitude_PI)*radius_of_earth;//   BC=1000*2*sin(经度差/2)*cos(此时纬度派)*地球半径;
//    AC_distance=2000*sin(latitude_difference/2)*radius_of_earth; //   AC距离=1000*2*sin(纬度差/2)*地球半径;
    ALL_distance=2000*radius_of_earth*asin(sqrt(pow(sin(latitude_difference/2),2)+cos(trageLA_PI)*cos(latitude_PI)*pow(sin(longitude_difference/2),2)));
    targe_theta=atan2(sin(longitude_difference)*cos(trageLA_PI),(cos(latitude_PI)*sin(trageLA_PI)-sin(latitude_PI)*cos(trageLA_PI)*cos(longitude_difference)))*180/3.1415926;
    ALL_distance_temp=ALL_distance;
    if (ALL_distance<1)
    {
        *left_pwm  = 1500 ;
        *right_pwm = 1500;
        return ;
    }
    //右手定则（正北0度）：0-(-90)-180-(90)——>0-90-180-270-360
    if (targe_theta>0){
        targe_theta=360-targe_theta;
    }
    else {
        targe_theta=abs(targe_theta);
    }
    Ek=(float)targe_theta-(float)current_theta;//偏差角
    if (abs(Ek)>=180)       //0~180右,(180~360左),-180~0左,(-360~-180右)//不加这个在正北前进时会向右打转
    {
        if (Ek>0)           //应该右转
        {
            Ek-=360;        //-180~0
        }
        else {              //应该左转
            Ek+=360;        //0~180
        }
    }
    angle_error = (int)Ek;
    if (abs(Ek)>90)
        {
            ALL_distance=0;                         //原地转圈
        }
    else {
            if (ALL_distance>=10)
            {
                ALL_distance=10;
            }
            ALL_distance*=cos(Ek*3.1415926/180.0);  //角度越大,总距离越大,转弯分量越大,前进分量越小
        }
    if (abs(Ek)<=90)integral+=Ek;                   //积分
    if (integral>integral_limit)
    {
        integral=integral_limit;
    }
    else if (integral<-integral_limit)
        {
            integral=-integral_limit;
        }
    differential=Ek-Ekk;                          //微分
    PID_value=Kp*Ek+Ki*integral+Kd*differential;  ////PID公式 ; Ek:-180~180
    Ekk=Ek;                                       //上一次偏差
    steer_pwm = (int)((1.0 / (1.0 + exp (-0.05 * PID_value)) - 0.5) * 1000);          //-500~500(PID_value=90到达最大值)
    if (ALL_distance_temp<=5.5)
    {
        forward_pwm=(int)(ALL_distance_temp*35);
    }
    else {
        forward_pwm = (int)((1.0 / (1.0 + exp (-0.4 * ALL_distance)) - 0.5) * 1000); //0~500(ALL_distance>0;ALL_distance=10到达最大值)
    }
//    # 缩放到指定最大值范围内,不超过500值
    if (forward_pwm + abs(steer_pwm) > max_control)
    {
        temp_forward_pwm = forward_pwm;
        forward_pwm      = (int)(max_control * ((float)temp_forward_pwm) / (temp_forward_pwm + abs(steer_pwm)));
        steer_pwm        = (int)(max_control * ((float)steer_pwm / (temp_forward_pwm + abs(steer_pwm))));
    }
    ////左高右高-电机前进               Ek>0;PID_value>0;steer_pwm>0;左转
    *left_pwm  = 1500 + forward_pwm - steer_pwm;
    *right_pwm = 1500 + forward_pwm + steer_pwm;

//    rt_kprintf("left_pwm%d  right_pwm %d",*left_pwm,*right_pwm);
}
