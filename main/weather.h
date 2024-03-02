#ifndef _WEATHER_H
#define _WEATHER_H

#include "esp_err.h"
#include "esp_http_client.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define GPIO_INPUT_PIN GPIO_NUM_0

void init_weather(void);

esp_err_t initnvs(void);

void http_test_task(void *pvParameters);

void http_rest_with_url(void);

esp_err_t _http_event_handler(esp_http_client_event_t *evt);

#ifdef __cplusplus
}
#endif


#endif
