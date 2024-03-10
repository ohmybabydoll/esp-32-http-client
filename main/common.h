#ifndef _COMMON_H
#define _COMMON_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int temp;         // 温度
        char *name;       // 城市名称
        char *text;       // 天气
        char *wind_class; // 风力
        char *wind_dir;   // 风向
        int rh;           // 湿度
    } weather_msg;

    /**
     * 这里添加 static 的原因是：weather.c 和 led.c 都引用了 common.h,编译的时候 weather.c 和led.c都被
     * 编译成立目标文件，再被链接器链接起来，就相当于 common.h 的代码重复了一次，如果不加 static 就会重复
     * 定义，报错 multiple definition of .. first defined here
    */
     


     void init_semaphore(void);

#ifdef __cplusplus
}
#endif

#endif