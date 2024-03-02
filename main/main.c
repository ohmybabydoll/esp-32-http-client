#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include <weather.h>



void app_main(void)
{   
    init_weather();
    xTaskCreate(&http_test_task, "http_test_task", 8192, NULL, 5, NULL); // 创建一个task任务，类似于线程。
}