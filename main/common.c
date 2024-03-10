#include <common.h>

SemaphoreHandle_t data_ready; // 信号量，天气数据已生成，可以供led.c调用
SemaphoreHandle_t json_done;  // led.c已使用完数据，weather.c 可以将json释放掉
weather_msg w_msg = {0, "", "", "", "", 0};

void init_semaphore(void)
{
    data_ready = xSemaphoreCreateBinary();
    json_done = xSemaphoreCreateBinary();
}