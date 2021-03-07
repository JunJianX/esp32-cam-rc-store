
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
// #include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "file_download.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "baidu.com"
#define WEB_PORT 80
#define WEB_URL "http://baidu.com/"

static const char *TAG = "example";
FILE* f=NULL;
char CFileName[30];
char FileName[30];
char IP[16];
char port[6];

// static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
//     "Host: "WEB_SERVER"\r\n"
//     "User-Agent: esp-idf/1.0 esp32\r\n"
//     "\r\n";

static void __attribute__((noreturn)) task_fatal_error()
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}
void file_download_init(char *cfn,char *fn,char*ip,char *p)
{
    if(cfn)
    {
        memset(CFileName,0,30);
        memcpy(CFileName,cfn,strlen(cfn));
    }
    if(fn)
    {
        memset(FileName,0,30);
        memcpy(FileName,fn,strlen(fn));

    }
    if(ip)
    {
        memset(IP,0,16);
        memcpy(IP,ip,strlen(ip));
    }
    if(port)
    {
        memset(port,0,6);
        memcpy(port,p,strlen(p));
    }

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


 void http_get_taskq(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    uint8_t retry=5;
    uint8_t connect_flag = 0;
    char recv_buf[64];
    char *pos = 0;
    char *http_request = NULL;

    unsigned int totalSize = 0;
    unsigned int file_size = 0;
    int size = 0;
    char text[TEXT_BUFFSIZE];
    int ret = 0;
    int addr_family;
    int ip_protocol;
    openfile();

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr("192.168.1.20");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(8080);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    // inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

    int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        // break;
    }

    ESP_LOGI(TAG, "Socket created, connecting to ");

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d!!!!!!!!!!!!!!!!", errno);
        task_fatal_error();
        // break;
    }
    ESP_LOGI(TAG, "Successfully connected");
    ESP_LOGI(TAG, "--------------------------");

    //////////////
     err = getaddrinfo(IP, port, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            // continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            // continue;
        }
        ESP_LOGI(TAG, "... allocated socket");


    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
            sizeof(receiving_timeout)) < 0) {
        ESP_LOGE(TAG, "... failed to set socket receiving timeout");
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "... set socket receiving timeout success");
    
    do{

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            // close(s);
            // freeaddrinfo(res);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }else
        {
            connect_flag =1;
            break;
            
        }

    }while(retry--);

    if(!connect_flag)
    {
        close(s);
        freeaddrinfo(res);
        task_fatal_error();
    }


    ESP_LOGI(TAG, "... connected");
    freeaddrinfo(res);
    const char *GET_FORMAT =
    "GET %s%s HTTP/1.0\r\n"
    "Host: %s:%d\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n\r\n";

    int get_len = asprintf(&http_request, GET_FORMAT, "/",CFileName, IP, port);
    if (get_len < 0) {
        ESP_LOGE(TAG, "Failed to allocate memory for GET request buffer");
        task_fatal_error();

    }
    printf("\r\n\r\n%s\r\n\r\n",http_request);
    if (write(s, http_request, strlen(http_request)) < 0) {
        ESP_LOGE(TAG, "... socket send failed");
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        free(http_request);
        task_fatal_error();
    }
    free(http_request);
    ESP_LOGI(TAG, "Send GET request to server succeeded");


    /* Read HTTP response */
    // do {
    //     bzero(recv_buf, sizeof(recv_buf));
    //     r = read(s, recv_buf, sizeof(recv_buf)-1);
    //     for(int i = 0; i < r; i++) {
    //         putchar(recv_buf[i]);
    //     }
    // } while(r > 0);

    // ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
    // close(s);
    // for(int countdown = 10; countdown >= 0; countdown--) {
    //     ESP_LOGI(TAG, "%d... ", countdown);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // ESP_LOGI(TAG, "Starting again!");
    bool flag = true;
    bool found_head =false;
    uint8_t retry_d = 0;
    while (flag) 
    {
        memset(text, 0, TEXT_BUFFSIZE);
        int buff_len = recv(s, text, TEXT_BUFFSIZE, 0);
        if(buff_len>0)
        {
            retry_d = 0;
            if(!found_head)
            {
                printf("---------\r\n");
                printf("%s.\r\n",text);
                printf("---------\r\n");
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
            
                pos = strstr(text, "\r\n\r\n");
                if (pos != NULL)
                {
                    pos += 4;
                    int len = pos - text;
                    found_head = true;
                    size = buff_len - len;
                    printf("\r\nsize is:%d.\r\n",size);
                    totalSize += size;
                    
                    fwrite(pos,1,size,f);
                }
            }else
            {
                fwrite(text,1,buff_len,f);
                totalSize+=buff_len;
            }
        }
        else if(buff_len<=0)
        {
            printf("trying\r\n");
            vTaskDelay(100/portTICK_PERIOD_MS);
            retry_d++;
            if(retry_d>50)
            {
                task_fatal_error();
            }
        }

        if(file_size==totalSize)
        {
            //允许文件再次写入

            ESP_LOGI(TAG, "Connection closed, all packets received");
            fclose(f);
            flag =false;
            break;
        }
        ESP_LOGI(TAG, "Total Write binary data length : %d,buff_len: %d", totalSize,buff_len);
    }
    vTaskDelete(NULL);
}