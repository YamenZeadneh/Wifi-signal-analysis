#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include <string.h>

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "socket.h"
#include "lwip/err.h"
#include "lwip/sys.h"

EventGroupHandle_t Handel_wifi_connection  = NULL;
const char *TAG = "Station";
static int s_retry_num = 0;
#define ssid_name  "Sender" 
#define WIFI_CONNECTED_BIT (1 << 0)
#define WIFI_FAIL_BIT      (1 << 1)

static void wifi_event_handler(void* arg,esp_event_base_t event_base,int32_t event_id,void* event_data){

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
        
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;

        ESP_LOGI(TAG,"Disconnected. Reason: %d",event->reason);

        if (s_retry_num < 10){
            esp_wifi_connect();
            s_retry_num++;

            ESP_LOGI(TAG,"retry to connect to the AP");
        }
        else{
            xEventGroupSetBits(Handel_wifi_connection,WIFI_FAIL_BIT);
        }
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG,"got ip:" IPSTR,IP2STR(&event->ip_info.ip));

        s_retry_num = 0;

        xEventGroupSetBits(Handel_wifi_connection,WIFI_CONNECTED_BIT);
    }
}
static void wifi_csi_rx_cb(void *ctx, wifi_csi_info_t *info)
{
    if (info == NULL || info->buf == NULL) {
        return;
    }

    printf("CSI,");

    for (int i = 0; i < info->len; i++) {
        printf("%d", info->buf[i]);

        if (i < info->len - 1) {
            printf(",");
        }
    }

    printf("\n");
}
static void csi_trigger_task(void *arg)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(5000),
        .sin_addr.s_addr = inet_addr("192.168.4.1")
    };

    const char *msg = "CSI";

    while (1) {
        sendto(
            sock,
            msg,
            strlen(msg),
            0,
            (struct sockaddr *)&dest_addr,
            sizeof(dest_addr)
        );

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void init_STA(){
    Handel_wifi_connection = xEventGroupCreate();
    // Create/Initialize LwIP
    ESP_ERROR_CHECK(esp_netif_init());
    //Create/Initialize Event
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //Create/Initialize Network Interface
    esp_netif_create_default_wifi_sta();
    // Create/Initialize Wi-Fi
    wifi_init_config_t Wifi_Data = WIFI_INIT_CONFIG_DEFAULT();
    Wifi_Data.csi_enable = 1;//enable csi
    ESP_ERROR_CHECK(esp_wifi_init(&Wifi_Data));
    //Event handling wifi sta 
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                                              WIFI_EVENT,
                                              ESP_EVENT_ANY_ID,
                                              &wifi_event_handler,
                                              NULL,
                                              NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                                              IP_EVENT,
                                              IP_EVENT_STA_GOT_IP,
                                              &wifi_event_handler,
                                              NULL,
                                              NULL));

    //configration 
    wifi_config_t wifi_config = {
        .sta ={
            .ssid = ssid_name,
            //.channel = 6,
            .password = "1234Pass",
        }
    };


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    
    wifi_csi_config_t csi_config = {
        .lltf_en = true,
        .htltf_en = true,
        .stbc_htltf2_en = true,
        .ltf_merge_en = true,
        .channel_filter_en = true,
        .manu_scale = false,
        .shift = false,
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_csi_config(&csi_config)
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_csi_rx_cb(wifi_csi_rx_cb, NULL)
    );
    
    ESP_ERROR_CHECK(
        esp_wifi_set_csi(true)
    );   
    xTaskCreate(
    csi_trigger_task,
    "csi_trigger",
    4096,
    NULL,
    5,
    NULL
    );                                      


    EventBits_t bits = xEventGroupWaitBits(Handel_wifi_connection,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ssid_name, "1234Pass");
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ssid_name, "1234Pass");
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

void app_main(void){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    init_STA();
}