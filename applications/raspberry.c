/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-07     DUAN       the first version
 */
#include "raspberry.h"
#include "manager.h"
#include <stdlib.h>
#include <string.h>
#define THREAD_PRIORITY         10
#define THREAD_STACK_SIZE       2048
#define THREAD_TIMESLICE        5

static rt_thread_t thread_raspberry_rec = RT_NULL;
static rt_thread_t thread_raspberry_send = RT_NULL;

#define LORA_UART_NAME       "uart2"    /* 串口设备名称 */
static rt_device_t raspberry_serial;                /* 串口设备句柄 */
struct serial_configure raspberry_config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */

struct rt_semaphore raspberry_sem;  // 信号量
int send_status_data=1; //发送状态消息
int send_detect_data=0; //发送检测的水质/深度消息
int send_confirm_data=0; //发送接受到消息的确认
char lora_buffer[50]={0};//数据

rt_err_t rx_jz_callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&raspberry_sem);
    return RT_EOK;
}

void init_raspberry()
{
    /* step1：查找串口设备 */
    raspberry_serial = rt_device_find(LORA_UART_NAME);
    /* step2：修改串口配置参数 */
    raspberry_config.baud_rate = BAUD_RATE_115200;        //修改波特率为 9600
    raspberry_config.data_bits = DATA_BITS_8;           //数据位 8
    raspberry_config.stop_bits = STOP_BITS_1;           //停止位 1
    raspberry_config.bufsz     = 7000;                   //修改缓冲区 buff size 为 128
    raspberry_config.parity    = PARITY_NONE;           //无奇偶校验位

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(raspberry_serial, RT_DEVICE_CTRL_CONFIG, &raspberry_config);

    /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(raspberry_serial, RT_DEVICE_FLAG_INT_RX);
    /* 初始化信号量 */
    rt_sem_init(&raspberry_sem, "raspberry_sem", 0, RT_IPC_FLAG_FIFO);
    rt_device_set_rx_indicate(raspberry_serial, rx_jz_callback);
}

/* 线程 的入口函数 */
static void lora_send(void *parameter)
{
    rt_tick_t draw_send_tick = 0; //测试时间戳
    while(1)
    {
        if (send_status_data==1)
        {
            if (rt_tick_get()-draw_send_tick>1000)
            {
                rt_kprintf("C%d,%d,%d,%d,%dZ\n",ship_id,bottle_id,draw_status,dump_draw_time,full_draw_time);
                draw_send_tick = rt_tick_get();
                rt_thread_mdelay(50);
            }
            if (draw_status==4)
            {
                rt_kprintf("B%d,%d,%d,%d,%d,%dZ\n",ship_id,wt,ph,doDo,ec,td);
                rt_thread_mdelay(50);
            }
            // 15米以内发送障碍物到服务器
            if (min_distance<1500)
            {
                for(int i=0;i<10;i++)
                {
                    if(target_id[i]==1)
                    {
                        rt_kprintf("D%d,%d,%d,%dZ\n",ship_id,i,target_angle[i],target_distance[i]);
                        rt_thread_mdelay(50);
                    }
                }
            }
            rt_kprintf("A%d,%d,%d,%d,%d,%d,%d,%dZ\n",ship_id,(int)(lng*1000000),(int)(lat*1000000),(int)dump_energy,(int)current_theta,current_mode,angle_error,speed);
            rt_thread_mdelay(150);
//            rt_kprintf("S%d,%d,%d,%d,%d,%d,%d,%d,%d,%dZ\n",(int)(target_lng*1000000),(int)(target_lat*1000000),relay,control_val,control_mode,network_backhome,energy_backhome,obstacle_avoid,hand_speed,auto_speed);
        }
        else if (send_confirm_data==1) {
            rt_kprintf(lora_buffer);//发送确认数据到服务器
            rt_thread_mdelay(50);
            send_status_data=1;
            send_detect_data=0;
        }
        rt_thread_mdelay(50);
    }
}

static void lora_rec(void *parameter)
{
    char lora_char;//接收字节
    int info_type=0; //消息类型
//    char lora_buffer[50]={0};//数据
    int lora_buffer_index=0;//索引
    char c1[20];
    char c2[20];
    char c3[20];
    char c4[20];
    char c5[20];
    char c6[20];
    while (1)
    {
        while(rt_device_read(raspberry_serial,0, &lora_char, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量释放后再次读取数据 */
           rt_sem_take(&raspberry_sem, RT_WAITING_FOREVER);
        }
        rec_server_tick =rt_tick_get();
//        rt_kprintf("%c\n",lora_char);
////接收R开头，Z配置目标点数据R0,5,114.123456,30.123456Z船号，接收点数，采集总点数，经度，纬度
        if(lora_char=='S')
        {
            lora_buffer[lora_buffer_index]=lora_char;
            lora_buffer_index+=1;
        }
        else if (lora_buffer_index ==1) {
            lora_buffer[lora_buffer_index]=lora_char;
            lora_buffer_index+=1;
            info_type=atoi(&lora_char);
        }
        else if (lora_buffer_index !=0) {
            lora_buffer[lora_buffer_index]=lora_char;
            lora_buffer_index+=1;
        }
        //如果接收到完整数据后分割字符串处理
        if(lora_char=='Z'&&lora_buffer_index!=0)
        {
            lora_buffer[lora_buffer_index] = '\0';
            if (info_type==1)
            {
                sscanf(lora_buffer,"S%[^,],%[^,],%[^Z]Z",c1,c2,c3);
                target_lng=atoi(c2)/1000000.0;
                target_lat=atoi(c3)/1000000.0;
                send_status_data=0;
                send_confirm_data=1;
            }
            else if (info_type==2) {
                sscanf(lora_buffer,"S%[^,],%[^,],%[^,],%[^Z]Z",c1,c2,c3,c4);
                bottle_id = atoi(c2);
//                rt_kprintf("bottle_id:%d",bottle_id);
                bottle_deep=atoi(c3);
                bottle_amount=atoi(c4);
                draw_status=0;
                send_status_data=0;
                send_confirm_data=1;
            }
            else if (info_type==3) {
                sscanf(lora_buffer,"S%[^,],%[^,],%[^Z]Z",c1,c2,c3);
                control_val=atoi(c2);
                control_mode=atoi(c3);
                if (control_mode==3)
                {
                    target_lat=0;
                    target_lat=0;
                }
//                send_status_data=0;
//                send_confirm_data=1;
            }
            else if (info_type==4) {
                sscanf(lora_buffer,"S%[^,],%[^,],%[^,],%[^,],%[^,],%[^Z]Z",c1,c2,c3,c4,c5,c6);
                network_backhome=atoi(c2);
                energy_backhome=atoi(c3);
                obstacle_avoid=atoi(c4);
                hand_speed=atoi(c5);
                auto_speed=atoi(c6);
                send_status_data=0;
                send_confirm_data=1;
            }
            else if (info_type==5) {
                sscanf(lora_buffer,"S%[^,],%[^,],%[^,],%[^Z]Z",c1,c2,c3,c4);
                pid[0]=atoi(c2)/100.0;
                pid[1]=atoi(c3)/100.0;
                pid[2]=atoi(c4)/100.0;
                send_status_data=0;
                send_confirm_data=1;
            }
            else if (info_type==6) {
                sscanf(lora_buffer,"S%[^,],%[^Z]Z",c1,c2);
                compass_status=atoi(c2);
                send_status_data=0;
                send_confirm_data=1;
            }
            else if (info_type==8) {
                is_connect=1;
                rec_server_tick =rt_tick_get();
//                send_status_data=0;
//                send_confirm_data=1;
            }
//            memset(lora_buffer,0,sizeof(lora_buffer));
            lora_buffer_index =0;
        }

        if (lora_buffer_index>=49)
        {
            lora_buffer_index=0;
        }
  }




}

/* 线程 */
int thread_raspberry(void)
{
    init_raspberry();
    /* 创建线程 */
    thread_raspberry_rec =  rt_thread_create("lora_rec",
                            lora_rec, RT_NULL,
                            8000,
                            2, 20);
    /* 如果获得线程控制块，启动这个线程 */
    if (thread_raspberry_rec != RT_NULL)
        rt_thread_startup(thread_raspberry_rec);

    /* 创建线程 */
    thread_raspberry_send = rt_thread_create("lora_send",
                            lora_send, RT_NULL,
                            2048,
                            2, THREAD_TIMESLICE);
    /* 如果获得线程控制块，启动这个线程 */
    if (thread_raspberry_send != RT_NULL)
        rt_thread_startup(thread_raspberry_send);

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_raspberry, thread_raspberry);

