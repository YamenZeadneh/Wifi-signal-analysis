#include "esp_wifi.h"
#include "nvs_flash.h"

const char* TARGET_SSID = "RADAR_TX";
typedef struct {
    uint16_t frame_ctrl;
    uint16_t duration_id;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t sequence_ctrl;
    
}wifi_ieee80211_mac_hdr_t;

void wifi_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t*)buf;

    wifi_ieee80211_mac_hdr_t *hdr =
        (wifi_ieee80211_mac_hdr_t*)pkt->payload;

    uint8_t* mac = hdr->addr2;
    //Serial.println(mac[0]);
    if (mac[0] == 0x8C &&
        mac[1] == 0x94 &&
        mac[2] == 0xDF &&
        mac[3] == 0x47 &&
        mac[4] == 0xF1 &&
        mac[5] == 0xB5) {

        int8_t rssi = pkt->rx_ctrl.rssi;
        Serial.println(rssi);
    }
    // int8_t rssi = pkt->rx_ctrl.rssi;
    //    Serial.println(rssi);
}

void setup() {
    Serial.begin(115200);
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_cb);
    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE); // نفس channel الـ TX
}

void loop() {
    delay(100);
}