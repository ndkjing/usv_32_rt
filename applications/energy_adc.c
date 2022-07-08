/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-02     DUAN       the first version
 */
#include "energy_adc.h"
#include "manager.h"

#define ADC_DEV_NAME        "adc1"      /* ADC 设备名称 */
#define ADC_DEV_CHANNEL     4          /* ADC 通道 */
#define REFER_VOLTAGE       327         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
//#define CONVERT_BITS        (1 << 12)   /* 转换位数为12位 */

//#define ARRAY_DIM(a) (sizeof(a) / sizeof((a)[0]))
//const static int Battery_Level_Percent_Table[11] = {3000, 3650, 3700, 3740, 3760, 3795, 3840, 3910, 3980, 4070, 4150};

rt_adc_device_t adc_dev;
static rt_thread_t t_energy = RT_NULL;
rt_uint32_t value=0, vol=0;
float_t Voltage;
int adc_init()
{
    rt_err_t ret = RT_EOK;

    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
        return RT_ERROR;
    }
    /* 使能设备 */
    ret = rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);

    return ret;
}

//int toPercentage(int voltage)
//{
//    int i = 0;
//    if(voltage < Battery_Level_Percent_Table[0])
//        return 0;
//
//    for(i; i<ARRAY_DIM(Battery_Level_Percent_Table); i++){
//        if(voltage < Battery_Level_Percent_Table[i])
//            return i*10 - (10UL * (int)(Battery_Level_Percent_Table[i] - voltage)) /
//            (int)(Battery_Level_Percent_Table[i] - Battery_Level_Percent_Table[i-1]);;
//    }
//
//    return 100;
//}

void check_dump_energy(void *parameter)
{
     int var=0;
     float_t all_num=0.0;
     while(1)
     {
         for (var = 0; var < 20; ++var)
         {
             rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);
             /* 读取采样值 */
             value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);
        //     rt_kprintf("%d\r\n",value);
             /* 转换为对应电压值 */
             vol = value * REFER_VOLTAGE / 4095;
             /* 转换为电池电压值(1/8.5) */
             Voltage=vol*17/200.0;
             all_num+=Voltage;
             rt_thread_mdelay(100);
         }

         Voltage=all_num/20+0.6;    //0.6为补偿值
         all_num=0;
         /* 转换为x% */
//         dump_energy=(Voltage-22.2)/3.0*100;
         dump_energy = ((tan(0.42*(Voltage-23.75)))+1)*75;//电压转化百分比
//         dump_energy=toPercentage(value);
         if (dump_energy<10.0)  dump_energy=1.0;
        //rt_kprintf("%d\r\n",(int)dump_energy);
         if (dump_energy>99.0)  dump_energy=99;
         rt_adc_disable(adc_dev, ADC_DEV_CHANNEL);
//         rt_kprintf("vol:%d  voltage:%d  dump_energy:%d\n",vol,(int)(Voltage*10),(int)dump_energy);

     }
}


void thread_energy()
{
    adc_init();
    /* 创建线程 */
        t_energy = rt_thread_create("thread_energy",
                check_dump_energy, RT_NULL,
                                1024,
                                10,
                                5);

        /* 如果获得线程控制块，启动这个线程 */
        if (t_energy != RT_NULL)
            rt_thread_startup(t_energy);
}


