#include <led.h>
#include "esp_log.h"
#include <common.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <led_strip.h>
#include <string.h>

static const char *TAG = "LED";
static led_rgb rgb = {155, 155, 155};
static led_view_mode view_mode = LED_VIEW_FLOW;
static int breath_direction = 1;
static int brightness = 0;
int led_array[LED_STRIP_LED_ROWS][LED_STRIP_LED_COLS];         // 根据led的矩阵生成的数组
int led_spiral_array[LED_STRIP_LED_ROWS * LED_STRIP_LED_COLS]; // 根据上面的数组生成的led索引螺旋一维数组，实现向内螺旋流水灯
static int led_flow_index = 0;                                 // 流水灯索引

void init_led(void)
{
    json_done = xSemaphoreCreateBinary();
    init_led_array();
}

void init_led_array(void)
{
    int start = 0;
    for (int i = 0; i < LED_STRIP_LED_ROWS; i++)
    {
        for (int j = 0; j < LED_STRIP_LED_COLS; j++)
        {
            led_array[i][j] = start++;
        }
    }
    SpiralTravelOfArray();
}

/**
 *  螺旋遍历数组
 * | 1 , 2, 3, 4 |
 * | 5 , 6, 7, 8 |
 * | 9 ,10,11,12 |
 * | 13,14,15,16 |
 * 顺时针遍历上面的二位数组，结果为[1,2,3,4,8,12,16,15,14,13,9,5,6,7,11,10]
 * 2024/3/5
 **/
void SpiralTravelOfArray(void)
{
    int indexX = 0; // 数组纵轴索引
    int indexY = 0; //  数组横轴索引
    int maxX = LED_STRIP_LED_ROWS - 1;
    int maxY = LED_STRIP_LED_COLS - 1;
    int resultIndex = 0;

    while (INT_MAX != led_array[indexX][indexY])
    {
        // 向左
        while (indexY <= maxY && INT_MAX != led_array[indexX][indexY])
        {
            led_spiral_array[resultIndex] = led_array[indexX][indexY];
            led_array[indexX][indexY] = INT_MAX;
            resultIndex++;
            if (indexY == maxY || INT_MAX == led_array[indexX][indexY + 1])
            {
                indexX++;
                break;
            }
            indexY++;
        }
        // 向下
        while (indexX <= maxX && INT_MAX != led_array[indexX][indexY])
        {
            led_spiral_array[resultIndex] = led_array[indexX][indexY];
            led_array[indexX][indexY] = INT_MAX;
            resultIndex++;
            if (indexX == maxX || INT_MAX == led_array[indexX + 1][indexY])
            {
                indexY--;
                break;
            }
            indexX++;
        }
        // 向右
        while (indexY >= 0 && INT_MAX != led_array[indexX][indexY])
        {
            led_spiral_array[resultIndex] = led_array[indexX][indexY];
            led_array[indexX][indexY] = INT_MAX;
            resultIndex++;
            if (indexY == 0 || INT_MAX == led_array[indexX][indexY - 1])
            {
                indexX--;
                break;
            }
            indexY--;
        }
        // 向上
        while (indexX >= 0 && INT_MAX != led_array[indexX][indexY])
        {
            led_spiral_array[resultIndex] = led_array[indexX][indexY];
            led_array[indexX][indexY] = INT_MAX;
            resultIndex++;
            if (indexX == 0 || INT_MAX == led_array[indexX - 1][indexY])
            {
                indexY++;
                break;
            }
            indexX--;
        }
    }
}

/**
 * 呼吸灯模式
 */
void led_breath(led_strip_handle_t led_strip)
{

    for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++)
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, rgb.red, rgb.green, rgb.blue));
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    rgb.red = rgb.red * brightness / 255;
    rgb.green = rgb.green * brightness / 255;
    rgb.blue = rgb.blue * brightness / 255;
    if (brightness == 0 || brightness == 255)
    {
        breath_direction = -breath_direction;
    }
    brightness += breath_direction;
}

/**
 * 流水灯模式
 */
void led_flow(led_strip_handle_t led_strip)
{
    int tails = 3;
    for (int i = 0; i < tails; i++)
    {
        int act_index = led_flow_index + i;
        if (act_index >= LED_STRIP_LED_NUMBERS)
        {
            act_index = act_index - LED_STRIP_LED_NUMBERS - 1;
        }
        int led_num = led_spiral_array[act_index];
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, led_num, rgb.red, rgb.green, rgb.blue));
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    vTaskDelay(pdMS_TO_TICKS(20));
    if (led_flow_index == LED_STRIP_LED_NUMBERS - 1)
    {
        led_flow_index = 0;
    }
    else
    {
        led_flow_index++;
    }
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
}

void led_task(void *pvParameters)
{
    led_strip_handle_t led_strip = configure_led();
    ESP_LOGI(TAG, "Start blinking LED strip");
    while (1)
    {
        // 信号量，类似于锁
        if (xSemaphoreTake(data_ready, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            set_color_by_weather();
            xSemaphoreGive(json_done); // 释放信号量
        }
        if (view_mode == LED_VIEW_BREATH)
        {
            led_breath(led_strip);
        }
        else if (view_mode == LED_VIEW_FLOW)
        {
            led_flow(led_strip);
        }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
}

/**
 * 通过信号量判断是否获取天气信息，然后根据天气来初始化颜色
 * 晴天 呼吸灯， 雨天 流水灯
 */
void set_color_by_weather(void)
{
    if (strstr(w_msg.text, "晴") != NULL)
    {
        view_mode = LED_VIEW_BREATH;
        rgb.red = 255;
        rgb.green = 153;
        rgb.blue = 18;
    }
    else if (strstr(w_msg.text, "雨") != NULL)
    {
        view_mode = LED_VIEW_FLOW;
        rgb.red = 255;
        rgb.green = 18;
        rgb.blue = 18;
    }
}

led_strip_handle_t configure_led(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_BLINK_GPIO,   // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_LED_NUMBERS,        // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812,            // LED strip model
        .flags.invert_out = false,                // whether to invert the output signal
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        .rmt_channel = 0,
#else
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .flags.with_dma = false,               // DMA feature is available on ESP target like ESP32-S3
#endif
    };

    // LED Strip object handle
    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
    return led_strip;
}