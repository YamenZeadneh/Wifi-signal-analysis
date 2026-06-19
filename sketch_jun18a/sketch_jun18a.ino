#include <WiFi.h>

void setup() {

    Serial.begin(115200);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP("RADAR_TX", "12345678",6);

    Serial.println(WiFi.softAPmacAddress());

}
void loop(){
    delay(100);
}