#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

const char *TAG = "softAP";
#define ssid_name  "Sender" 

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
}

void init_AP(){
    // Create/Initialize LwIP
    ESP_ERROR_CHECK(esp_netif_init());
    //Create/Initialize Event
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //Create/Initialize Network Interface
    esp_netif_create_default_wifi_ap();
    // Create/Initialize Wi-Fi
    wifi_init_config_t Wifi_Data = WIFI_INIT_CONFIG_DEFAULT();
    Wifi_Data.csi_enable = 1;//enable csi
    ESP_ERROR_CHECK(esp_wifi_init(&Wifi_Data));
    //Event handling wifi ap 
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                              ESP_EVENT_ANY_ID,
                                              &wifi_event_handler,
                                              NULL,
                                              NULL));

    //configration 
    wifi_config_t wifi_config = {
        .ap ={
            .ssid = ssid_name,
            .ssid_len = strlen(ssid_name),
            .channel = 6,
            .password = "1234Pass",
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA2_PSK,
        }
    };
    if (strlen(ssid_name) == 0 && wifi_config.ap.authmode != WIFI_AUTH_OWE) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP,&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    init_AP();
}
