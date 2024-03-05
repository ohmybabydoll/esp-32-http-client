#ifndef _LED_H
#define _LED_H

#include <led_strip_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// GPIO assignment
#define LED_STRIP_BLINK_GPIO  2
// Numbers of the LED in the strip
#define LED_STRIP_LED_NUMBERS 256

// LED 长
#define LED_STRIP_LED_ROWS 16

// LED 宽
#define LED_STRIP_LED_COLS 16

// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

typedef enum {
    LED_VIEW_FLOW, /* 点亮模式，流水灯 */
    LED_VIEW_BREATH /* 点亮模式，呼吸灯 */
} led_view_mode;

typedef struct {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
} led_rgb;

void init_led(void);
void init_led_array(void);
void SpiralTravelOfArray(void);
void led_breath(led_strip_handle_t led_strip);
void led_flow(led_strip_handle_t led_strip);
void led_task(void *pvParameters);
void set_color_by_weather(void);
led_strip_handle_t configure_led(void);

#ifdef __cplusplus
}
#endif


#endif