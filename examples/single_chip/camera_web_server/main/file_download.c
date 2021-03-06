#include "file_download.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
// #include "protocol_examples_common.h"
#include "errno.h"
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include <sys/socket.h>
#include <netdb.h>


#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

// #include "esp_netif.h"
#include "esp_event.h"

static int socket_id = -1;
FILE* f=NULL;
static const char *TAG = "FILE_DOWNLOAD";

char CFileName[30];
char FileName[30];
char IP[16];
int port;

static void __attribute__((noreturn)) task_fatal_error()
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}
void file_download_init(char *cfn,char *fn,char*ip,int P)
{
    memset(CFileName,0,30);
    memcpy(CFileName,cfn,strlen(cfn));

    memset(FileName,0,30);
    memcpy(FileName,fn,strlen(fn));

    memset(IP,0,16);
    memcpy(IP,ip,strlen(ip));

    port  = P;
}
char *enhance_strstr(char *buffer, char *goal, int len)
{
    uint16_t buffer_len = len;
    uint16_t goal_len = strlen(goal);
    uint16_t count_goal = 0;
    char *p = NULL;
    for (uint16_t i = 0; i < buffer_len - goal_len + 1; i++)
    {

        p = &buffer[i];
        for (count_goal = 0; count_goal < goal_len; count_goal++)
        {
            if (goal[count_goal] == buffer[i + count_goal])
            {
                continue;
            }
            else
            {
                p = NULL;
                break;
            }
        }
        if (count_goal >= goal_len)
        {
            return p;
        }
    }
    return NULL;
}

static bool connect_to_http_server()
{
    // ESP_LOGI(TAG, "Server IP: %s Server Port:%s", EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);
    ESP_LOGI(TAG, "Server IP: %s. Server Port:%d", IP, port);

    int  http_connect_flag = -1;
    struct sockaddr_in sock_info;

    socket_id = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        ESP_LOGE(TAG, "Create socket failed!");
        return false;
    }

    // set connect info
    memset(&sock_info, 0, sizeof(struct sockaddr_in));
    sock_info.sin_family = AF_INET;
    sock_info.sin_addr.s_addr = inet_addr(IP);
    sock_info.sin_port = htons(port);

    // connect to http server
    http_connect_flag = lwip_connect(socket_id, (struct sockaddr *)&sock_info, sizeof(sock_info));
    if (http_connect_flag == -1) {
        ESP_LOGE(TAG, "Connect to server failed! errno=%d", errno);
        close(socket_id);
        return false;
    } else {
        ESP_LOGI(TAG, "Connected to server");
        return true;
    }
    return false;
}

static void openfile(void)
{
    char completePath[40]={0};
    snprintf(completePath,40,"/sdcard%s",FileName);
    printf("completePath is:%s.\r\n",completePath);
    f = fopen(completePath, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
}

void file_download_store_task(void)
{
    char *pos = 0;
    unsigned int totalSize = 0;
    unsigned int file_size = 0;
    int size = 0;
    char text[TEXT_BUFFSIZE];
    int ret = 0;
    openfile();
    if (connect_to_http_server()) {
        ESP_LOGI(TAG, "Connected to http server");
    } else {
        ESP_LOGE(TAG, "Connect to http server failed!");
        task_fatal_error();
    }

    const char *GET_FORMAT =
        "GET %s%s HTTP/1.0\r\n"
        "Host: %s:%d\r\n"
        "User-Agent: esp-idf/1.0 esp32\r\n\r\n";
    char *http_request = NULL;

    int get_len = asprintf(&http_request, GET_FORMAT, "/",CFileName, IP, port);
    if (get_len < 0) {
        ESP_LOGE(TAG, "Failed to allocate memory for GET request buffer");
        task_fatal_error();
    }

    printf("\r\n\r\n%s\r\n\r\n",http_request);
    int res = send(socket_id, http_request, get_len, 0);
    free(http_request);

    if (res < 0) {
        ESP_LOGE(TAG, "Send GET request to server failed");
        task_fatal_error();
    } else {
        ESP_LOGI(TAG, "Send GET request to server succeeded");
    }

    bool flag = true;
    bool found_head =false;
    while (flag) 
    {
        memset(text, 0, TEXT_BUFFSIZE);
        int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);
        if(buff_len>0)
        {
            if(!found_head)
            {
                char *ptr = strstr(text, "Content-Length:");
                if (ptr)
                {
                    ret = sscanf(ptr, "%*[^ ]%d", &file_size);
                    if (ret < 0)
                    {
                        printf("Content-Length error.");
                    }
                    printf("file size %d\n",file_size);
                }
            }
            pos = strstr(text, "\r\n\r\n");
            if (pos != NULL)
            {
                pos += 4;
                int len = pos - text;
                found_head = true;
                size = buff_len - len;
                totalSize += size;
                fwrite(pos,1,size,f);
            }
            fwrite(text,1,buff_len,f);
            totalSize+=buff_len;
        }
        else if(buff_len<=0)
        {
            printf("trying\r\n");

        }

        if(file_size==totalSize)
        {
            //允许文件再次写入

            ESP_LOGI(TAG, "Connection closed, all packets received");
            fclose(f);
            break;
        }
        ESP_LOGI(TAG, "Total Write binary data length : %d", file_size);
    }
// END:
//     fclose(f);
}
