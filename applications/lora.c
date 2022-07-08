/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-29     XXL       the first version
 */
#include "lora.h"
#include "manager.h"

#define LORA_UART_NAME       "uart3"    /* 串口设备名称 */
#define THREAD_PRIORITY         3
#define THREAD_STACK_SIZE       2048
#define THREAD_TIMESLICE        5

static rt_thread_t thread_lora_rec = RT_NULL;
static rt_thread_t thread_lora_send = RT_NULL;
static rt_device_t lora_dev;                /* 串口设备句柄 */
struct serial_configure lora_config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */
struct rt_semaphore lora_sem;  // 信号量

rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&lora_sem);
    return RT_EOK;
}

void init_lora()
{
    /* step1：查找串口设备 */
    lora_dev = rt_device_find(LORA_UART_NAME);
    /* step2：修改串口配置参数 */
    lora_config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
    lora_config.data_bits = DATA_BITS_8;           //数据位 8
    lora_config.stop_bits = STOP_BITS_1;           //停止位 1
    lora_config.bufsz     = 128;                   //修改缓冲区 buff size 为 128
    lora_config.parity    = PARITY_NONE;           //无奇偶校验位

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(lora_dev, RT_DEVICE_CTRL_CONFIG, &lora_config);
    /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(lora_dev, RT_DEVICE_FLAG_INT_RX);
    /* 初始化信号量 */
    rt_sem_init(&lora_sem, "lora_sem", 0, RT_IPC_FLAG_FIFO);
    rt_device_set_rx_indicate(lora_dev, rx_callback);
}

/* 线程 的入口函数 */
static void lora_send(void *parameter)
{
    char str[] = "RT-ok!\r\n";
    while (1)
    {
//        if (lng>100.0) {
            rt_device_write(lora_dev, 0, str, (sizeof(str) - 1));  //向指定串口发送数据
//        }
        rt_thread_mdelay(300);
    }
}

static void lora_rec(void *parameter)
{
    char lora_char;//接收字节
    char lora_buffer[32]={0};//摇杆数据
    int lora_buffer_index=0;//索引
    char row_s[5]={0};
    char col_s[5]={0};
    char key_s[6]={0};
    int data_type=1; //1:摇杆数据   2:按键数据
    while (1)
    {
        while(rt_device_read(lora_dev,0, &lora_char, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量释放后再次读取数据 */
           rt_sem_take(&lora_sem, RT_WAITING_FOREVER);
        }
        //接收A。。。Z数据，遥控
        if(lora_char=='A' || lora_char=='H')
        {
            lora_buffer_index=0;

            lora_buffer[lora_buffer_index]=lora_char;
            lora_buffer_index+=1;
            if (lora_char=='A')
            {
                data_type=1;
            }
            else {
                data_type=2;
            }
        }
        else if (lora_buffer_index !=0)
        {
            lora_buffer[lora_buffer_index]=lora_char;
            lora_buffer_index+=1;
        }
        //如果接收到完整数据后分割字符串处理
        if(lora_char=='Z'&&lora_buffer_index!=0)
        {
            lora_buffer[lora_buffer_index] = '\0';
            if (data_type==1)
            {
                sscanf(lora_buffer,"A%[0-9],%[0-9]Z,",col_s,row_s);
                col = atoi(col_s);//直行
                row = atoi(row_s);//转弯
                // 控制电机输出 将摇杆值0-99转换到1000-2000
                if ((col>45) && (col<55))col=50;//直行
                if ((row>45) && (row<55))row=50;//转弯
//                rt_kprintf("col %d row %d\r\n",col,row);
//                rt_kprintf(lora_buffer);
            }
            if (data_type==2)
            {
                sscanf(lora_buffer,"H%[0-9]Z,",key_s);
                char pos1_s[2]={0};
                char pos2_s[2]={0};
                char pos3_s[2]={0};
                pos1_s[0]=key_s[0];
                pos2_s[0]=key_s[1];
                pos3_s[0]=key_s[2];
                int key1 =  atoi(pos1_s);
                int key2 =  atoi(pos2_s);
                int key3 =  atoi(pos3_s);
                if (key1%2==1)
                {
                    key[0]=1;
                }
                else {
                    key[0]=0;
                }
                if (key1%3==1)
                {
                    key[1]=1;
                }
                else {
                    key[1]=0;
                }
                if (key2%3==1)
                {
                    key[2]=1;
                }
                else {
                    key[2]=0;
                }
//                rt_kprintf("key1:%d\r\n",key1);
//                if (key==0)  //接受到全是0
//                {
//
//                }
//                else
//                {
//                    pos1 = key/10000;
//                    pos1 = key/1000%10;
//                    pos1 = key/100%10;
//                    // TODO crc校验
//                    if (key1/10000==1)
//                    {
//                        key[0]=1;
//                    }
//
//                    rt_kprintf("keys 1:%c\r\n",key_s[0]);
//                }

//
            }
            lora_buffer_index =0;
            rec_tick = rt_tick_get(); //更新接受到使能时间戳
        }
        if (lora_buffer_index>20)
        {
            lora_buffer_index=0;
        }
    }
}

/* 线程 */
int thread_lora(void)
{
    init_lora();
    /* 创建线程 */
    thread_lora_rec = rt_thread_create("lora_rec",
                      lora_rec, RT_NULL,
                      THREAD_STACK_SIZE,
                      THREAD_PRIORITY,
                      THREAD_TIMESLICE);
    /* 如果获得线程控制块，启动这个线程 */
    if (thread_lora_rec != RT_NULL)
        rt_thread_startup(thread_lora_rec);

    /* 创建线程 */
//    thread_lora_send = rt_thread_create("lora_send",
//                       lora_send,
//                       RT_NULL,
//                       1025,
//                       5,
//                       THREAD_TIMESLICE);
    /* 如果获得线程控制块，启动这个线程 */
//    if (thread_lora_send != RT_NULL)
//        rt_thread_startup(thread_lora_send);

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_lora, thread_lora);












