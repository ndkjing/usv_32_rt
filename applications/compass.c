/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-28     XXL       the first version
 */
#include "compass.h"
#include "manager.h"

static rt_thread_t t_compass = RT_NULL;
rt_device_t compass_dev;
struct rt_semaphore compass_sem;
struct serial_configure compass_configs = RT_SERIAL_CONFIG_DEFAULT;

rt_err_t rx_compass_callback(rt_device_t dev_, rt_size_t size_)
{
    rt_sem_release(&compass_sem);
    return RT_EOK;
}

int init_compass()
{
    rt_err_t ret = 0;
    compass_dev = rt_device_find("uart1");
    if(compass_dev == RT_NULL){
        return -EINVAL;
    }

    /* step2：修改串口配置参数 */
    compass_configs.baud_rate = BAUD_RATE_115200;        //修改波特率为 115200(不能9600，鬼知道为啥)
    compass_configs.data_bits = DATA_BITS_8;           //数据位 8
    compass_configs.stop_bits = STOP_BITS_1;           //停止位 1
    compass_configs.bufsz     = 512;                   //修改缓冲区 buff size 为512
    compass_configs.parity    = PARITY_NONE;           //无奇偶校验位
    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(compass_dev, RT_DEVICE_CTRL_CONFIG, (void *)&compass_configs);
    /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
    rt_device_set_rx_indicate(compass_dev, rx_compass_callback);
    ret = rt_device_open(compass_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);  //中断
    if(ret < 0){
        rt_kprintf("ret : %d\n",ret);
        return ret;
    }
    /* 初始化信号量 */
    ret = rt_sem_init(&compass_sem,"rx_sem", 0, RT_IPC_FLAG_FIFO);
    if(ret < 0){
        return ret;
    }
    return 0;
}

void check_compass(void *parameter)
{
    char g_char;
    int compass_index = 0;
    char compass_buffer[128]={0};
    char compass_s[15]={0};
    //Magx:-1863,Magy:28,Magz:2565,Yaw:179.1
    while(1)
    {
        while(rt_device_read(compass_dev, 0, &g_char, 1) != 1)//等待接收完
        {
            rt_sem_take(&compass_sem, RT_WAITING_FOREVER);//释放信息量
        }
        if (g_char=='Y')
        {
            compass_buffer[compass_index]=g_char;
            compass_index+=1;
        }
        else if (compass_index!=0)
        {
            compass_buffer[compass_index]=g_char;
            compass_index+=1;
        }
        if (g_char == '\n' && compass_index!=0)
        {
            compass_buffer[compass_index] = '\0';
            sscanf(compass_buffer,"Yaw:%s",compass_s);//Yaw:99.1
            //右手定则（正南0度）：180 -（-90）-0-90 ——>0-360
            current_theta = atoff(compass_s)+180;
            compass_index =0;
//            rt_kprintf("current_theta:%d\n",current_theta);
        }
    }
}

void check_radar()
{
    int r_hex=0X00;
    int pre_r_hex=0X00;
    int t_id;
    int t_angle;
    int t_distance;
    int gps_index = 0;  // GPS数据
    int gps_buffer[128] = {0};  // GPS数据
    while(1)
    {
        while(rt_device_read(compass_dev, 0, &r_hex, 1) != 1)//等待接收完
        {
            rt_sem_take(&compass_sem, RT_WAITING_FOREVER);//释放信息量
        }
//        rt_kprintf("G%d,%d,%dZ\n",ship_id,r_hex,pre_r_hex);
        if(r_hex==170 && pre_r_hex==170)
        {
            gps_buffer[gps_index]=r_hex;
            gps_index+=1;
        }
        else if (gps_index!=0) {
            gps_buffer[gps_index]=r_hex;
            gps_index+=1;
        }
        if (r_hex==85 && pre_r_hex==85)
        {
//            rt_kprintf("F%d,%d,%dZ\n",ship_id,gps_buffer[1],gps_buffer[2]);
            if (gps_buffer[1]==12 && gps_buffer[2]==7)
            {
                t_id = gps_buffer[3];
                t_angle = gps_buffer[7];
                t_distance = gps_buffer[5]*256+gps_buffer[6];
                obstacle_tick=rt_tick_get();
                if (t_distance<min_distance)
                {
                    min_distance=t_distance;
                }
                if (t_id<10)
                {
                   target_id[t_id]=1;
                   target_distance[t_id]=t_distance;
                   target_angle[t_id]=t_angle;
                }
//                rt_kprintf("E%d,%d,%d,%d,%dZ\n",ship_id,t_id,t_distance,t_angle,min_distance);
            }
            gps_index =0;
        }
        if(gps_index>13) gps_index=0;
        pre_r_hex = r_hex;
   }
}
void thread_compass()
{
    init_compass();
    /* 创建线程 */
    t_compass = rt_thread_create("thread_compass",
                                 check_radar,
                                 RT_NULL,
                                 2048,
                                 4,
                                 5);

    /* 如果获得线程控制块，启动这个线程 */
    if (t_compass != RT_NULL)
    {
        rt_thread_startup(t_compass);
    }
}



