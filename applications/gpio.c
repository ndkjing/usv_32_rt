/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-06     DUAN       the first version
 */
#include "gpio.h"

/* 初始化 */
void init_gpio(void)
{
    ////设置管脚的模式
    //状态灯引脚
    rt_pin_mode(LED1, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2, PIN_MODE_OUTPUT);
    rt_pin_mode(LED3, PIN_MODE_OUTPUT);
    //lora模式引脚
    rt_pin_mode(MD0, PIN_MODE_OUTPUT);
    rt_pin_mode(MD1, PIN_MODE_OUTPUT);
    //PW模式引脚
    rt_pin_mode(PW_IN, PIN_MODE_INPUT_PULLUP);     //上拉输入模式
    rt_pin_mode(PW_OUT, PIN_MODE_OUTPUT);
    //继电器引脚
    rt_pin_mode(relay1, PIN_MODE_OUTPUT);
    rt_pin_mode(relay2, PIN_MODE_OUTPUT);
    rt_pin_mode(relay3, PIN_MODE_OUTPUT);   //备用
    rt_pin_mode(relay4, PIN_MODE_OUTPUT);   //备用
    //电磁阀引脚
    rt_pin_mode(valve1, PIN_MODE_OUTPUT);
    rt_pin_mode(valve2, PIN_MODE_OUTPUT);
    rt_pin_mode(valve3, PIN_MODE_OUTPUT);
    rt_pin_mode(valve4, PIN_MODE_OUTPUT);
    rt_pin_mode(valve5, PIN_MODE_OUTPUT);
    rt_pin_mode(valve6, PIN_MODE_OUTPUT);
    rt_pin_mode(valve7, PIN_MODE_OUTPUT);
    rt_pin_mode(valve8, PIN_MODE_OUTPUT);
    ////初始化电平
    rt_pin_write(MD0, PIN_LOW);
    rt_pin_write(MD1, PIN_LOW);
    control_status_led(2);

}

// 控制板子状态灯
void control_status_led(int status_code)
{
    switch(status_code)
    {
        //绿色
    case 1:
        rt_pin_write(LED1, PIN_LOW);
        rt_pin_write(LED2, PIN_HIGH);
        rt_pin_write(LED3, PIN_LOW);
        break;
        //红色
    case 2:
        rt_pin_write(LED1, PIN_HIGH);
        rt_pin_write(LED2, PIN_LOW);
        rt_pin_write(LED3, PIN_LOW);
        break;
        //黄色
    case 3:
        rt_pin_write(LED1, PIN_LOW);
        rt_pin_write(LED2, PIN_LOW);
        rt_pin_write(LED3, PIN_HIGH);
        break;
    default:
        rt_pin_write(LED1, PIN_LOW);
        rt_pin_write(LED2, PIN_LOW);
        rt_pin_write(LED3, PIN_LOW);
        break;
    }
}

void draw(int bottle_id)
{
    switch (bottle_id)
    {
    // 停止抽水和电磁阀
    case 0:
        rt_pin_write(relay1, PIN_LOW);
        rt_pin_write(relay2, PIN_LOW);
        rt_pin_write(valve1, PIN_LOW);
        rt_pin_write(valve2, PIN_LOW);
        rt_pin_write(valve3, PIN_LOW);
        rt_pin_write(valve4, PIN_LOW);
        rt_pin_write(valve5, PIN_LOW);
        rt_pin_write(valve6, PIN_LOW);
        rt_pin_write(valve7, PIN_LOW);
        rt_pin_write(valve8, PIN_LOW);
        break;
    // 往1号瓶抽水
    case 1:
        rt_pin_write(relay1, PIN_HIGH);
        rt_pin_write(relay2, PIN_LOW);
        rt_pin_write(valve1, PIN_HIGH);
        rt_pin_write(valve2, PIN_LOW);
        rt_pin_write(valve3, PIN_LOW);
        rt_pin_write(valve4, PIN_LOW);
        rt_pin_write(valve5, PIN_LOW);
        rt_pin_write(valve6, PIN_LOW);
        rt_pin_write(valve7, PIN_LOW);
        rt_pin_write(valve8, PIN_LOW);
        break;
    // 往2号瓶抽水
    case 2:
        rt_pin_write(relay1, PIN_HIGH);
        rt_pin_write(relay2, PIN_LOW);
        rt_pin_write(valve1, PIN_LOW);
        rt_pin_write(valve2, PIN_HIGH);
        rt_pin_write(valve3, PIN_LOW);
        rt_pin_write(valve4, PIN_LOW);
        rt_pin_write(valve5, PIN_LOW);
        rt_pin_write(valve6, PIN_LOW);
        rt_pin_write(valve7, PIN_LOW);
        rt_pin_write(valve8, PIN_LOW);
        break;
// 往3号瓶抽水
    case 3:
        rt_pin_write(relay1, PIN_HIGH);
        rt_pin_write(relay2, PIN_LOW);
        rt_pin_write(valve1, PIN_LOW);
        rt_pin_write(valve2, PIN_LOW);
        rt_pin_write(valve3, PIN_HIGH);
        rt_pin_write(valve4, PIN_LOW);
        rt_pin_write(valve5, PIN_LOW);
        rt_pin_write(valve6, PIN_LOW);
        rt_pin_write(valve7, PIN_LOW);
        rt_pin_write(valve8, PIN_LOW);
        break;
    // 往4号瓶抽水
    case 4:
        rt_pin_write(relay1, PIN_HIGH);
        rt_pin_write(relay2, PIN_LOW);
        rt_pin_write(valve1, PIN_LOW);
        rt_pin_write(valve2, PIN_LOW);
        rt_pin_write(valve3, PIN_LOW);
        rt_pin_write(valve4, PIN_HIGH);
        rt_pin_write(valve5, PIN_LOW);
        rt_pin_write(valve6, PIN_LOW);
        rt_pin_write(valve7, PIN_LOW);
        rt_pin_write(valve8, PIN_LOW);
        break;
    // 往5号瓶抽水 五号瓶设置为检测仓
    case 5:
        rt_pin_write(relay1, PIN_HIGH);
        rt_pin_write(relay2, PIN_LOW);
        rt_pin_write(valve1, PIN_LOW);
        rt_pin_write(valve2, PIN_LOW);
        rt_pin_write(valve3, PIN_LOW);
        rt_pin_write(valve4, PIN_LOW);
        rt_pin_write(valve5, PIN_HIGH);
        rt_pin_write(valve6, PIN_LOW);
        rt_pin_write(valve7, PIN_LOW);
        rt_pin_write(valve8, PIN_LOW);
        break;
    // 往6号瓶抽水
    case 6:
            rt_pin_write(relay1, PIN_HIGH);
            rt_pin_write(relay2, PIN_LOW);
            rt_pin_write(valve1, PIN_LOW);
            rt_pin_write(valve2, PIN_LOW);
            rt_pin_write(valve3, PIN_LOW);
            rt_pin_write(valve4, PIN_LOW);
            rt_pin_write(valve5, PIN_LOW);
            rt_pin_write(valve6, PIN_HIGH);
            rt_pin_write(valve7, PIN_LOW);
            rt_pin_write(valve8, PIN_LOW);
            break;
    // 往7号瓶抽水
    case 7:
            rt_pin_write(relay1, PIN_HIGH);
            rt_pin_write(relay2, PIN_LOW);
            rt_pin_write(valve1, PIN_LOW);
            rt_pin_write(valve2, PIN_LOW);
            rt_pin_write(valve3, PIN_LOW);
            rt_pin_write(valve4, PIN_LOW);
            rt_pin_write(valve5, PIN_LOW);
            rt_pin_write(valve6, PIN_LOW);
            rt_pin_write(valve7, PIN_HIGH);
            rt_pin_write(valve8, PIN_LOW);
            break;
    // 往8号瓶抽水
    case 8:
            rt_pin_write(relay1, PIN_HIGH);
            rt_pin_write(relay2, PIN_LOW);
            rt_pin_write(valve1, PIN_LOW);
            rt_pin_write(valve2, PIN_LOW);
            rt_pin_write(valve3, PIN_LOW);
            rt_pin_write(valve4, PIN_LOW);
            rt_pin_write(valve5, PIN_LOW);
            rt_pin_write(valve6, PIN_LOW);
            rt_pin_write(valve7, PIN_LOW);
            rt_pin_write(valve8, PIN_HIGH);
            break;
    default:
        rt_pin_write(relay1, PIN_LOW);
        rt_pin_write(relay2, PIN_LOW);
        rt_pin_write(valve1, PIN_LOW);
        rt_pin_write(valve2, PIN_LOW);
        rt_pin_write(valve3, PIN_LOW);
        rt_pin_write(valve4, PIN_LOW);
        rt_pin_write(valve5, PIN_LOW);
        rt_pin_write(valve6, PIN_LOW);
        rt_pin_write(valve7, PIN_LOW);
        rt_pin_write(valve8, PIN_LOW);
        break;
}
}



