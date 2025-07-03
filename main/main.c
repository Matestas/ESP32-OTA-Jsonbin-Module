#include <stdio.h>
#include "wifi_connect.h"
#include "ota_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#define USE_EXTERNAL_CREDENTIALS
    
double version = 0.1; // Global variable to hold the firmware version

#ifdef USE_EXTERNAL_CREDENTIALS
    #include "credentials.h"
#else
    static const char* wifi_ssid = "defaultSSID";
    static const char* wifi_password = "defaultPassword";
    static const char* ota_master_key = "defaultMasterKey";
    static const char* ota_access_key = "defaultAccessKey";
    static const char* ota_url = "https://default.url/ota";
#endif

void app_main(void)
{
    wifi_init();
    wifi_connect_start(wifi_ssid, wifi_password, 10000);
    ESP_LOGI("Main", "Wi-Fi connection process started with SSID: %s", wifi_ssid);
    ota_set_master_key(ota_master_key);
    ota_set_acess_key(ota_access_key);
    ota_set_url(ota_url);
    ota_start();
}