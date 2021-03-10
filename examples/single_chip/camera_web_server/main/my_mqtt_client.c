#include "my_mqtt_client.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "infra_compat.h"
#include "dev_sign_api.h"
#include "mqtt_api.h"
#include "wrappers.h"
#include "time.h"
#include "cJSON.h"
#include "file_download.h"
#include "esp_log.h"
// #include "my_uart.h"
// #include "my_tools.h"

// extern uint8_t aliyun_flag;
// extern uint8_t ota_start_flag;
// extern parse_event_struct_t my_uart_event;

char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1h9HQrdQSw";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "oe9uVvWedVXXwKMg";//
char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = "tflhmTDEjuEDDZLIJB30";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "61a38a37ce4cc819338ef0fca4d5864e";
/*
char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1MZxOdcBnO";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "h4I4dneEFp7EImTv";//
char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "jiPvZkVO9lhNj2Q9f2KdP4Yln7ACJI3X";*/
static char *TAG = "ALI_MQTT";
#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    printf("\n-------------------example_message_arrive------------------\n");
    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("Message Arrived:");
            EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            EXAMPLE_TRACE("\n");
            break;
        default:
            break;
    }
}

int example_subscribe(void *handle)
{
    int res = 0;
    // const char *fmt = "/%s/%s/user/up";
    const char *fmt = "/sys/%s/%s/thing/service/property/set";
    const char *fmt_ntp = "/ext/ntp/%s/%s/response";
    char *topic = NULL;
    char *topic_ntp = NULL;
    int topic_len = 0;
    int topic_ntp_len=0;

    topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("subscribe failed");
        HAL_Free(topic);
        return -1;
    }
    
    topic_ntp_len = strlen(fmt_ntp)+strlen(g_product_key)+strlen(g_device_name)+1;
    topic_ntp = HAL_Malloc(topic_ntp_len);
    if(topic_ntp==NULL)
    {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic_ntp,0,topic_ntp_len);
    HAL_Snprintf(topic_ntp,topic_ntp_len,fmt_ntp,g_product_key,g_device_name);

    res = IOT_MQTT_Subscribe(handle,topic_ntp,IOTX_MQTT_QOS0,example_message_arrive,NULL);
    if (res < 0) {
        EXAMPLE_TRACE("subscribe failed");
        HAL_Free(topic_ntp);
        return -1;
    }


    HAL_Free(topic);
    return 0;
}
#define PAYLOAD_LEN  200
int example_publish(void *handle)
{
    time_t t;

    int             res = 0;
    // const char     *fmt = "/%s/%s/user/up";
    // /a1GPsB9z5fS/${deviceName}/user/contrl
    const char     *fmt = "/sys/%s/%s/thing/event/property/post";
    // const char     *fmt = "";
    char           *topic = NULL;
    int             topic_len = 0;
    int             payload_len = 0;
    // char           *payload = "{\"message\":\"hello!\"}";
    // char           *payload_fmt = "{\"Temperature\": %.2f,\"Humidity\":%d,\"Gas\":%d}";
    char           *payload_fmt = "{\"id\":%ld,\"params\":{\"HeartRate\":%d,\"RunningTotalTime\":%d,\"RunningDistance\":%d,\"RunningSteps\":%d},\"method\":\"thing.event.property.post\"}";
    char           *payload = NULL;// = "{\"Temperature\": %2f,\"humidity\":%d%%}";

    time(&t);
    topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);

    payload = HAL_Malloc(PAYLOAD_LEN);
    if(payload == NULL)
    {
        EXAMPLE_TRACE("payload memory not enough");
        return -1; 
    }
    memset(payload,0,PAYLOAD_LEN);
    sprintf(payload,payload_fmt,0,1,2);
    sprintf(payload,payload_fmt,t,0,1,2,3);
    printf("\n%s\n",payload);

    printf("Publish payload:\n");
    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0) {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        HAL_Free(topic);
        // aliyun_flag = 0;
        return -1;
    }
    // aliyun_flag=1;

    HAL_Free(topic);
    HAL_Free(payload);
    return 0;
}


void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
    printf("Payload:%s",topic_info->payload);
    // printf("MSG:%s\n",(char *)msg->msg);
}

extern const char *IOT_Extension_StateDesc(const int);
int everything_state_handle(const int state_code, const char *state_message)
{
    /*
     * NOTE: Uncomment below to demonstrate how to dump code descriptions
     *
     * After invoking IOT_Extension_StateDesc(), integer code will be translated into strings
     *
     * It looks like:
     *
     * ...
     * everything_state_handle|102 :: recv -0x0329(pub - '/a1MZxOdcBnO/example1/user/get': 0), means 'Report publish relative parameters such as topic string'
     * ...
     *
     */

    
        // EXAMPLE_TRACE("recv -0x%04X(%s), means '%s'",
        //               -state_code,
        //               state_message,
        //               IOT_Extension_StateDesc(state_code));
   
    EXAMPLE_TRACE("recv -0x%04X(%s)",
                  -state_code,
                  state_message);
    return 0;
}

int identity_response_handle(const char *payload)
{
    EXAMPLE_TRACE("identify: %s", payload);

    return 0;
}

/*
 *  NOTE: About demo topic of /${productKey}/${deviceName}/user/get
 *
 *  The demo device has been configured in IoT console (https://iot.console.aliyun.com)
 *  so that its /${productKey}/${deviceName}/user/get can both be subscribed and published
 *
 *  We design this to completely demonstrate publish & subscribe process, in this way
 *  MQTT client can receive original packet sent by itself
 *
 *  For new devices created by yourself, pub/sub privilege also requires being granted
 *  to its /${productKey}/${deviceName}/user/get for successfully running whole example
 */
void property_set_event_handle(const int devid, const char *serviceid, const int serviceid_len,
        const char *request, const int request_len,
        char **response, int *response_len)
{
    printf("+++++++++++++++++++++++++++++++++++++++\n");
    // iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    // EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
    // printf("RECV:\n%s\n",topic_info->payload);
    // printf("%s\n",request);
    printf("Service Request Received, Devid: %d, Service ID: %.*s, Payload: %s", devid, serviceid_len, serviceid, request);

}
/*
void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    printf("\n-------------------example_message_arrive------------------\n");
    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
             //print topic name and topic message 
            EXAMPLE_TRACE("Message Arrived:");
            EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            EXAMPLE_TRACE("\n");
            break;
        default:
            break;
    }
}
*/
extern uint8_t controller;
void parse_packet(uint16_t topic_len,const char * topic,uint16_t payload_len,const char *payload)
{
    
    int mul=1;
    uint8_t result=0;
    char *first_p_t=NULL;
    char *first_p_p=NULL;
    char *end_p_p=NULL;
    cJSON *items = NULL,*item =NULL,*tmp = NULL;

    if( topic==NULL||topic_len==0||payload_len == 0||payload==NULL)
    {
        return;
    }
    first_p_t = strstr(topic,"/thing/service/");
    first_p_p = strstr(payload,"param1");
    
    if(first_p_t==NULL||first_p_p==NULL)
    {
        printf("no nedd  parse_packet!\n");
        return;
    }
    end_p_p = first_p_p+8;
    while(*(end_p_p+1)!='}')
    {
        end_p_p++;
    }
    while(end_p_p>=first_p_p+8)
    {

        result+=mul*(*end_p_p-'0');
        mul*=10;
        end_p_p--;
    }
    controller = result;
    
    items = cJSON_Parse(payload);
    if (items == NULL) {
		// fprintf(stderr, "pasre json file fail\n");
        printf("pasre json file fail\n");
		return ;
	}
    item = cJSON_GetObjectItem(items, "params");
    tmp = cJSON_GetObjectItem(item, "param1");
    printf("using cJson to read param1:%s\n\n",tmp->valuestring);

}

void parse(uint16_t topic_len,const char * topic,uint16_t payload_len,const char *payload)
{
    cJSON *items = NULL,*item =NULL,*tmp = NULL;
    items = cJSON_Parse(payload);
    // char buffer[6]="";
    if (items == NULL) {
		// fprintf(stderr, "pasre json file fail\n");
        printf("pasre json file fail\n");
		return -1;
	}
    printf("\r\n%s\r\n",payload);
    tmp = cJSON_GetObjectItem(items, "params");
    if(tmp ==NULL)
    {
        printf("pasre item file fail\n");
		return -1;
    }else
    {
        item = cJSON_GetObjectItem(tmp,"ip");
        ESP_LOGI(TAG,"IP is:%s.",item->valuestring);
        file_download_init(NULL,NULL,item->valuestring,NULL);

        item = cJSON_GetObjectItem(tmp,"port");
        ESP_LOGI(TAG,"port is:%s.",item->valuestring);
        file_download_init(NULL,NULL,NULL,item->valuestring);

        item = cJSON_GetObjectItem(tmp,"server_file_name");
        ESP_LOGI(TAG,"server_file_name is:%s.",item->valuestring);
        file_download_init(item->valuestring,NULL,NULL,NULL);

        item = cJSON_GetObjectItem(tmp,"sd_file_name");
        ESP_LOGI(TAG,"sd_file_name is:%s.",item->valuestring);
        file_download_init(NULL,item->valuestring,NULL,NULL);

        ESP_LOGI(TAG,"Cloud config OK.Start Download!");
        file_download_debug();

        extern  void http_get_taskq(void *pvParameters);
        xTaskCreate(&http_get_taskq, "http_get_task", 4096, NULL, 5, NULL);

    }

    cJSON_Delete(items);
}
void event_handle_mqtt(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            printf("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            printf("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            printf("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            printf("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            printf("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            printf("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            printf("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            printf("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            printf("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            printf("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            printf("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            printf("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            printf("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s\n",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            parse(topic_info->topic_len,topic_info->ptopic,topic_info->payload_len,topic_info->payload);
            break;

        default:
            printf("Should NOT arrive here.");
            break;
    }
}

// static int user_timestamp_reply_event_handler(const char *timestamp)
// {
//     LOGD(MOD_SOLO, "time is %s", timestamp);
//     HAL_UTC_Set(atoll(timestamp));
//     return 0;
// }
void My_mqtt_task(void /**parm*/)
{
    void                   *pclient = NULL;
    int                     res = 0;
    int                     loop_cnt = 0;
    iotx_mqtt_param_t       mqtt_params;

    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_SECRET, g_device_secret);

    IOT_RegisterCallback(ITE_IDENTITY_RESPONSE, identity_response_handle);
    IOT_RegisterCallback(ITE_STATE_EVERYTHING, everything_state_handle);
    // IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, user_timestamp_reply_event_handler);

    // IOT_RegisterCallback(ITE_SERVICE_REQUEST, property_set_event_handle);
    // IOT_RegisterCallback(ITE_SERVICE_REQUEST, property_set_event_handle);


    // printf("\nhello mqtt!\n");
    EXAMPLE_TRACE("mqtt example");
    printf("++++++++++++++++++++++++++\n-------------------------\n");

    memset(&mqtt_params, 0x0, sizeof(mqtt_params));
    // printf("\nline 164\n");
   
    // mqtt_params.handle_event.h_fp = example_event_handle;
    mqtt_params.handle_event.h_fp = event_handle_mqtt;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    // printf("\nline 169\n");
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return ;
    }
    // printf("\nline 174\n");
    // aliyun_flag = 1;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_NAME, g_device_name);

    res = example_subscribe(pclient);
    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        // aliyun_flag = 0;
        return ;
    }
    

    while (1) {
        if (0 == loop_cnt % (2530)) {
            example_publish(pclient);
        }
        // if(ota_start_flag==1)
        // {
        //     printf("DELETE My_mqtt_task!\n\n");
        //     vTaskDelete(NULL);
        // }

        IOT_MQTT_Yield(pclient, 200);

        loop_cnt += 1;
        
    }

    return ;
}

