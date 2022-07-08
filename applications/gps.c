/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-28     XXL       the first version
 */
#include "gps.h"
#include "manager.h"
static rt_thread_t t_gps = RT_NULL;
rt_device_t gps_dev;
struct rt_semaphore gps_sem;
struct serial_configure gps_configs = RT_SERIAL_CONFIG_DEFAULT;

rt_err_t rx_gps_callback(rt_device_t dev_, rt_size_t size_)
{
    rt_sem_release(&gps_sem);
    return RT_EOK;
}

int init_gps()
{
    rt_err_t ret = 0;
    gps_dev = rt_device_find("uart4");
    ret = rt_device_open(gps_dev,  RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);  //中断
    if(ret < 0){
        rt_kprintf("rt_device_open[uart3] failed...\n");
        return ret;
        }
    /* step2：修改串口配置参数 */
     gps_configs.baud_rate = BAUD_RATE_115200;        //修改波特率为 115200
     gps_configs.data_bits = DATA_BITS_8;           //数据位 8
     gps_configs.stop_bits = STOP_BITS_1;           //停止位 1
     gps_configs.bufsz     = 1024;                   //修改缓冲区 buff size 为512
     gps_configs.parity    = PARITY_NONE;           //无奇偶校验位
     /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
     rt_device_control(gps_dev, RT_DEVICE_CTRL_CONFIG, (void *)&gps_configs);
     /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
     rt_device_set_rx_indicate(gps_dev, rx_gps_callback);
     /* 初始化信号量 */
     ret = rt_sem_init(&gps_sem,"rx_sem", 0, RT_IPC_FLAG_FIFO);
     if(ret < 0){
         rt_kprintf("rt_sem_init failed...\n");
         return ret;
     }
     return 0;
}

void check_gps_ascii(void *parameter)
{
    char g_char;
    int gps_index = 0;  // GPS数据
    char gps_buffer[128] = {0};  // GPS数据
    char lng_s_int[15]={0};
    char lat_s_int[15]={0};
    char lng_s_float[15]={0};
    char lat_s_float[15]={0};
    while(1)
    {
        while(rt_device_read(gps_dev, 0, &g_char, 1) != 1)//等待接收完
        {
            rt_sem_take(&gps_sem, RT_WAITING_FOREVER);//释放信息量
        }
        if(g_char=='$')
        {
            gps_buffer[gps_index]=g_char;
            gps_index+=1;
        }
        else if (gps_index!=0) {
            gps_buffer[gps_index]=g_char;
            gps_index+=1;
        }
        if (g_char=='E'&&gps_index!=0) {
            gps_buffer[gps_index]='\0';
            // $GPRMC,092927.000,A,2235.9058,N,11400.0518,E,0.000,74.11,151216,,D*49
            // $GNGGA,121252.000,3937.3032,N,11611.6046,E,1,05,2.0,45.9,M,-5.7,M,,0000*77
            sscanf(gps_buffer,"$GPRMC,%*[^,],A,%2s%[^,],N,%3s%[^,],E",lat_s_int,lat_s_float,lng_s_int,lng_s_float);
            //度分转度格式
            lng = atof(lng_s_int)+atof(lng_s_float)/60.000000;
            lat = atof(lat_s_int)+atof(lat_s_float)/60.000000;
            gps_index =0;
            rt_kprintf("lng %d lat %d \n",(int)(lng*1000000),(int)(lat*1000000));

        }
        if(gps_index>50) gps_index=0;

    }

}

void check_gps(void *parameter)
{
    int r_hex=0X00;
    int lng_;
    int lat_;
    int speed_; // 速度放大十倍
    int gps_index = 0;  // GPS数据
    int gps_buffer[128] = {0};  // GPS数据
    while(1)
    {
        while(rt_device_read(gps_dev, 0, &r_hex, 1) != 1)//等待接收完
        {
            rt_sem_take(&gps_sem, RT_WAITING_FOREVER);//释放信息量
        }
//        rt_kprintf("val:%d gps_index:%d\r\n",r_hex,gps_index);
        if(r_hex==85)
        {
            gps_buffer[gps_index]=r_hex;
            gps_index+=1;
        }
        else if (gps_index!=0) {
            gps_buffer[gps_index]=r_hex;
            gps_index+=1;
        }
        if (gps_index==11) {
            if (gps_buffer[1]==82)
            {
//                rt_kprintf("gps_buffer 82 [1] %d\n",gps_buffer[1]);
            }
            if (gps_buffer[1]==83)
            {
                current_theta = (gps_buffer[7]*256+gps_buffer[6])/32768.0*180;
//                rt_kprintf("gps_buffer 83 angle %d\n",(int)angle_z);
            }
            if (gps_buffer[1]==87)
            {
                lng_ = gps_buffer[5]*pow(2,24)+gps_buffer[4]*pow(2,16)+gps_buffer[3]*pow(2,8)+gps_buffer[2];
                lat_ = gps_buffer[9]*pow(2,24)+gps_buffer[8]*pow(2,16)+gps_buffer[7]*pow(2,8)+gps_buffer[6];
                lng = lng_/10000000+(lng_%10000000)/100000.0/60.0;
                lat = lat_/10000000+(lat_%10000000)/100000.0/60.0;
//                rt_kprintf("gps_buffer 87 lng_ %d  lat_ %d\n",lng_,lat_);
//                rt_kprintf("gps_buffer 87 lng %d  lat %d\n",(int)(lng*1000000),(int)(lat*1000000));
            }
            if (gps_buffer[1]==88)
            {
                speed_ = (gps_buffer[9]*pow(2,24)+gps_buffer[8]*pow(2,16)+gps_buffer[7]*pow(2,8)+gps_buffer[6]);
                speed=speed_;
//                rt_kprintf("gps_buffer 88 speed_ %d \n",speed_);
            }
            gps_index =0;
        }
        if(gps_index>11) gps_index=0;
    }
}

void thread_gps()
{
    init_gps();
    /*创建线程*/
    t_gps = rt_thread_create("thread_gps",
            check_gps,
            RT_NULL,
            2048,
            5,
            5);
    if(t_gps!=RT_NULL)
    {
            rt_thread_startup(t_gps);
    }

}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_gps, thread_gps);


