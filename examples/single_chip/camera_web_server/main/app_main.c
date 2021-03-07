/* ESPRESSIF MIT License
 * 
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 * 
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "app_httpd.h"
#include "app_mdns.h"
#include "app_board.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <stdio.h>
#include "file_download.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"


extern EventGroupHandle_t xEventGroup;
#define BIT_0 (1<<0)
extern  void http_get_taskq(void *pvParameters);

const char *TAG="MAIN";

void app_main()
{
    app_board_main();
    app_httpd_main();
    app_mdns_main();

    ESP_LOGI("esp-cam Version",CONFIG_ESP_CAM_VERSION);


    xEventGroupWaitBits(xEventGroup,BIT_0,true,true,pdFALSE);
    ESP_LOGI(TAG, "Connected to WiFi.");
    // xEventGroupWaitBits(s_connect_event_group, CONNECTED_BITS, true, true, portMAX_DELAY);
    // printf("----------------START----------------\r\n");
    // file_download_init("zfb.jpg","/zfb.jpg","192.168.1.20","8080");
    // vTaskDelay(3000/portTICK_PERIOD_MS);
    //     xTaskCreate(&http_get_taskq, "http_get_task", 4096, NULL, 5, NULL);
    // // xTaskCreate(&file_download_store_task,"file_download_store_task",8192,NULL,3,NULL);
    // printf("-----------------END-----------------\r\n");

}