/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-28     XXL       the first version
 */
#include "water.h"
#include "manager.h"
static rt_thread_t t_water_send = RT_NULL;
static rt_thread_t t_water_rec = RT_NULL;
rt_device_t water_dev;
struct rt_semaphore water_sem;
struct serial_configure water_configs1 = RT_SERIAL_CONFIG_DEFAULT;

rt_err_t rx_water_callback(rt_device_t dev_, rt_size_t size_)
{
    rt_sem_release(&water_sem);
    return RT_EOK;
}

int init_water()
{
     /* step1：查找串口设备 */
     water_dev = rt_device_find("uart5");
     /* step2：修改串口配置参数 */
     water_configs1.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
     water_configs1.data_bits = DATA_BITS_8;           //数据位 8
     water_configs1.stop_bits = STOP_BITS_1;           //停止位 1
     water_configs1.bufsz     = 700;                   //修改缓冲区 buff size 为 128
     water_configs1.parity    = PARITY_NONE;           //无奇偶校验位

     /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
     rt_device_control(water_dev, RT_DEVICE_CTRL_CONFIG, &water_configs1);

     /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
     rt_device_open(water_dev, RT_DEVICE_FLAG_INT_RX);
     /* 初始化信号量 */
     rt_sem_init(&water_sem, "water_sem", 0, RT_IPC_FLAG_FIFO);
     rt_device_set_rx_indicate(water_dev, rx_water_callback);
     return 1;
}

void send_water_data()
{
    int index=0;
    rt_uint8_t ph_data[9] = {0x06,0x03,0x00,0x00,0x00,0x04,0x45,0xBE};
    rt_uint8_t cod_data[3] = {0xFF,0xFF,0xFF};
    rt_uint8_t do_data[3] = {0xFF,0xFF,0xFF};
    while (1)
    {
        switch (index)
        {
        case 0:  // PH
            rt_device_write(water_dev, 0, ph_data, 8);
            break;
        case 1:  // cod
            rt_device_write(water_dev, 0, cod_data, 3);
            break;
        case 2:  // do
            rt_device_write(water_dev, 0, do_data, 3);
            break;
        }
//        index+=1;
//        index%=3;
        rt_thread_mdelay(500);
    }
}

void check_water(void *parameter)
{
    int r_hex=0X00;
    int water_index = 0;  // water数据
    int water_buffer[128] = {0};  // water数据
    while(1)
    {
        while(rt_device_read(water_dev, 0, &r_hex, 1) != 1)//等待接收完
        {
            rt_sem_take(&water_sem, RT_WAITING_FOREVER);//释放信息量
        }
        if(r_hex==6)
        {

            water_buffer[water_index]=r_hex;
            water_index+=1;
        }
        else if (water_index!=0) {
            water_buffer[water_index]=r_hex;
            water_index+=1;
        }
        if (water_index==12) {
            ph = water_buffer[3]*256+water_buffer[4];
            wt=water_buffer[7]*256+water_buffer[8];
            water_index =0;
        }
        if(water_index>12) water_index=0;
    }
}

void thread_water()
{
    init_water();
    /*创建线程*/
    t_water_rec = rt_thread_create("thread_water_rec",
            check_water,
            RT_NULL,
            2048,
            5,
            5);
    if(t_water_rec!=RT_NULL)
    {
       rt_thread_startup(t_water_rec);
    }
    /*创建线程*/
    t_water_send = rt_thread_create("thread_water_send",
                send_water_data,
                RT_NULL,
                2048,
                5,
                5);
        if(t_water_send!=RT_NULL)
        {
           rt_thread_startup(t_water_send);
        }
}



